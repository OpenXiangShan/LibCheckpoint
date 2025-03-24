#include "decode_restore.h"
#include "checkpoint.pb.h"
#include "csr.h"
#include "encoding.h"
#include "gcpt_asm.h"
#include "gcpt_trap.h"
#include "printf.h"
#include "spinlock.h"
#include "utils.h"
#include <stdint.h>

#define NOP 0x00000013

spinlock_t globash_restore_lock;
static uint64_t release_restore               = 0;
static uint64_t global_atomic_restore_counter = 0;

typedef union {
  struct {
    uint32_t opcode   : 7;
    uint32_t rd       : 5;
    uint32_t imm_19_12: 8;
    uint32_t imm_11   : 1;
    uint32_t imm_10_1 : 10;
    uint32_t imm_20   : 1;
  };
  uint32_t inst;
} JalInstr;

#define RESTORE_CHECK(func, name, success_status, ...) \
  {                                                    \
    if (func(__VA_ARGS__) == success_status) {         \
      printf(#name " restore success\n");              \
    } else {                                           \
      printf(#name " restore failed\n");               \
    }                                                  \
  }

static uint32_t generate_jal(int32_t imm, uint8_t rd) {
  JalInstr jal  = {0};
  jal.opcode    = 0b1101111;
  jal.rd        = rd;
  jal.imm_20    = (imm >> 19) & 1;
  jal.imm_19_12 = (imm >> 11) & 0xff;
  jal.imm_11    = (imm >> 10) & 1;
  jal.imm_10_1  = (imm >> 0) & 0x3ff;

  uint32_t instr = jal.inst;
  return instr;
}

static int is_valid_imm(int32_t imm) {
  return (imm >= -(1 << 20)) && (imm < (1 << 20));
}

static void checkpoint_ext_check(uint64_t *csr_address, int *rvv_could_restore,
                                 int *rvh_could_restore, int *rvf_could_restore,
                                 int *should_jump_to_mepc,
                                 uint64_t *mode_address) {

  uint64_t host_misa = read_csr(misa);

  uint64_t checkpoint_mstatus =
    *(uint64_t *)((uint64_t)csr_address + (CSR_MSTATUS << 3));
  uint64_t checkpoint_misa =
    *(uint64_t *)((uint64_t)csr_address + (CSR_MISA << 3));

  uint64_t checkpoint_mode = *(mode_address);

  if ((checkpoint_mode << 11) == (uint64_t)MSTATUS_MPP) {
    printf("checkpoint mode is M %lx\n", checkpoint_mode);
    *should_jump_to_mepc = 1;
  }

  if ((checkpoint_misa & MISA_H) && (host_misa & MISA_H) &&
      (checkpoint_mstatus & MSTATUS_MPV)) {
    // can restore H ext
    *rvh_could_restore = 1;
  } else {
    *rvh_could_restore = 0;
  }

  if ((checkpoint_misa & MISA_V) && (host_misa & MISA_V)) {
    if (((checkpoint_mstatus & MSTATUS_VS_INITIAL) == MSTATUS_VS_INITIAL) ||
        ((checkpoint_mstatus & MSTATUS_VS_CLEAN) == MSTATUS_VS_CLEAN) ||
        ((checkpoint_mstatus & MSTATUS_VS_DIRTY) == MSTATUS_VS_DIRTY)) {
      // can restore V ext
      *rvv_could_restore = 1;
    } else {
      *rvv_could_restore = 0;
    }
  }

  if ((checkpoint_misa & MISA_DF) && (host_misa & MISA_DF)) {
    if (((checkpoint_mstatus & MSTATUS_FS_INITIAL) == MSTATUS_FS_INITIAL) ||
        ((checkpoint_mstatus & MSTATUS_FS_CLEAN) == MSTATUS_FS_CLEAN) ||
        ((checkpoint_mstatus & MSTATUS_FS_DIRTY) == MSTATUS_FS_DIRTY)) {
      // can restore F ext
      *rvf_could_restore = 1;
    } else {
      *rvf_could_restore = 0;
    }
  }
}

single_core_rvgc_rvv_rvh_memlayout
  get_core_memlayout(uint64_t base_addr, uint64_t single_core_size, int cpu_id,
                     single_core_rvgc_rvv_rvh_memlayout *memlayout) {

  // percpu hardware status offset
  uint64_t offset = cpu_id * single_core_size + base_addr;

  single_core_rvgc_rvv_rvh_memlayout cpux_memlayout = {
    .magic_number_cpt_addr = memlayout->magic_number_cpt_addr + offset,
    .pc_cpt_addr         = memlayout->pc_cpt_addr + offset,
    .mode_cpt_addr       = memlayout->mode_cpt_addr + offset,
    .mtime_cpt_addr      = memlayout->mtime_cpt_addr + offset,
    .mtime_cmp_cpt_addr  = memlayout->mtime_cmp_cpt_addr + offset,
    .misc_done_cpt_addr  = memlayout->misc_done_cpt_addr + offset,
    .misc_reserve        = memlayout->misc_reserve + offset,
    .int_reg_cpt_addr    = memlayout->int_reg_cpt_addr + offset,
    .int_reg_done        = memlayout->int_reg_done + offset,
    .float_reg_cpt_addr  = memlayout->float_reg_cpt_addr + offset,
    .float_reg_done      = memlayout->float_reg_done + offset,
    .csr_reg_cpt_addr    = memlayout->csr_reg_cpt_addr + offset,
    .csr_reg_done        = memlayout->csr_reg_done + offset,
    .csr_reserve         = memlayout->csr_reserve + offset,
    .vector_reg_cpt_addr = memlayout->vector_reg_cpt_addr + offset,
    .vector_reg_done     = memlayout->vector_reg_done + offset,
  };

  return cpux_memlayout;
}

#define DISPLAY_REGS(BUFFER_BASE, START_IDX, IDX_MAX, WRAP_WIDTH, FORMAT, ...) \
  for (int i = START_IDX; i < IDX_MAX; i++) {                                  \
    printf(FORMAT, __VA_ARGS__);                                               \
    if ((i + 1) % WRAP_WIDTH == 0) {                                           \
      printf("\n");                                                            \
    }                                                                          \
  }

__attribute__((unused)) static void
  display_checkpoint_registers(single_core_rvgc_rvv_rvh_memlayout *memlayout) {

  uint8_t *buffer_offset = 0;

  printf("Int Register display: \n");
  buffer_offset = (uint8_t *)memlayout->int_reg_cpt_addr;
  DISPLAY_REGS(buffer_offset, 0, 32, 4, "gpr %04d value %016lx ", i,
               *(uint64_t *)(buffer_offset + i * 8))

  printf("Float Register display: \n");
  buffer_offset = (uint8_t *)memlayout->float_reg_cpt_addr;
  DISPLAY_REGS(buffer_offset, 0, 32, 4, "fpr %04d value %016lx ", i,
               *(uint64_t *)(buffer_offset + i * 8))

  printf("Vector Register display: \n");
  buffer_offset = (uint8_t *)memlayout->vector_reg_cpt_addr;
  for (int i = 0; i < 32 * 2; i++) {
    if ((i + 1) % (2) == 0) {
      printf("[%lx]: 0x%016lx_%016lx\n", (uint64_t)buffer_offset + (i - 1) * 8,
             *(uint64_t *)(buffer_offset + (i - 1) * 8),
             *(uint64_t *)(buffer_offset + (i) * 8));
    }
  }

  printf("Csr registers display\n");
  buffer_offset = (uint8_t *)memlayout->csr_reg_cpt_addr;
  for (int i = 0; i < 4096; i++) {
    uint64_t val;
    val = *(uint64_t *)(buffer_offset + i * 8);
    if (val != 0) {
      if (regs[i].id == i) {
        printf("csr id %x name: %s value %lx\n", i, regs[i].name, val);
      } else {
        printf("csr id %x value %lx\n", i, val);
      }
    }
  }

  buffer_offset = (uint8_t *)memlayout->mtime_cmp_cpt_addr;
  printf("mtime_cmp registers memory: %lx %lx\n", *(uint64_t *)buffer_offset,
         buffer_offset);

  buffer_offset = (uint8_t *)memlayout->mtime_cpt_addr;
  printf("mtime registers memory: %lx %lx\n", *(uint64_t *)buffer_offset,
         buffer_offset);
}

void multicore_decode_restore(uint64_t cpt_base_address,
                              uint64_t single_core_size, int cpu_id,
                              single_core_rvgc_rvv_rvh_memlayout *memlayout) {

  single_core_rvgc_rvv_rvh_memlayout multi_core_mem_layout =
    get_core_memlayout(cpt_base_address, single_core_size, cpu_id, memlayout);

#ifdef STOP_CPU
  if (cpu_id == STOP_CPU) {
    while (1) {}
  }
#endif /* ifdef STOP_CPU */

#ifdef DISPLAY
  if (DISPLAY == cpu_id) {
    display_checkpoint_registers(&multi_core_mem_layout);
  }
#endif /* ifdef DISPLAY */

  single_core_rvv_rvh_rvgc_restore(&multi_core_mem_layout, cpu_id);
};

void single_core_rvv_rvh_rvgc_restore(
  single_core_rvgc_rvv_rvh_memlayout *memlayout, int cpu_id) {
  if (memlayout == NULL) {
    memlayout = &default_rvgc_v_h_memlayout;
  }
  int rvv_could_restore   = 0;
  int rvh_could_restore   = 0;
  int rvf_could_restore   = 0;
  int should_jump_to_mepc = 0;
  int all_cpu_num         = *((uint64_t *)memlayout->csr_reserve);

  checkpoint_ext_check((void *)memlayout->csr_reg_cpt_addr, &rvv_could_restore,
                       &rvh_could_restore, &rvf_could_restore,
                       &should_jump_to_mepc, (void *)memlayout->mode_cpt_addr);

  if (should_jump_to_mepc) {
    extern void test_jump_target();
    uint64_t target_pc = *((uint64_t *)memlayout->pc_cpt_addr);
    extern int is_valid_imm(int32_t imm);
    extern uint32_t generate_jal(int32_t imm, uint8_t rd);
    extern uint32_t core0;

    uint32_t *pc = &core0;
    uint32_t offset;
    uint32_t jal_instr;

    pc    += 3 * (cpu_id + 1) + cpu_id;
    offset = (uint32_t)((uint64_t)target_pc - (uint64_t)pc);

    if (!is_valid_imm(offset)) {
      mt_printf("Offset is out of range for JAL instruction pc is %p jump target "
             "addr is %p\n",
             pc, target_pc);
      nemu_signal(GCPT_GET_BAD_IMM);
    }

    offset    = (offset >> 1) & 0xFFFFF;
    jal_instr = generate_jal(offset, 0); // x0 is rd
    *pc       = jal_instr;

    __builtin___clear_cache((char *)pc, (char *)pc + sizeof(uint32_t));
  }

  if (rvh_could_restore) {
    RESTORE_CHECK(rvh_csr_restore, H, 1, (void *)memlayout->csr_reg_cpt_addr);
  }

  if (rvv_could_restore) {
    RESTORE_CHECK(rvv_restore, V, 1, (void *)memlayout->csr_reg_cpt_addr,
                  (void *)memlayout->vector_reg_cpt_addr);
  }

  if (rvf_could_restore) {
    RESTORE_CHECK(restore_float_vector, Float, 1,
                  (void *)memlayout->float_reg_cpt_addr);
  }

  RESTORE_CHECK(restore_csr_vector, CSR, 1,
                (void *)memlayout->csr_reg_cpt_addr);

  RESTORE_CHECK(restore_mtime_cmp, MTIME_CMP, 1,
                (void *)(uint64_t)(CLINT_MMIO + CLINT_MTIMECMP + (cpu_id * 8)),
                (void *)(memlayout->mtime_cmp_cpt_addr + (cpu_id * 8)));

  spinlock_lock(&globash_restore_lock);
  atomic_add(&global_atomic_restore_counter, 1);
  mt_printf("all cpu num %d atomic flag %d\n", all_cpu_num,
         atomic_read(&global_atomic_restore_counter));
  if (atomic_read(&global_atomic_restore_counter) != all_cpu_num &&
      all_cpu_num != 0) {
    spinlock_unlock(&globash_restore_lock);
    while (atomic_read(&release_restore) != 1) {}
  } else {
    spinlock_unlock(&globash_restore_lock);
    atomic_set(&release_restore, 1);
    RESTORE_CHECK(restore_mtime, MTIME, 1, (void *)memlayout->mtime_cpt_addr);
  }

  if (should_jump_to_mepc) {
    restore_int_vector_jump((void *)memlayout->int_reg_cpt_addr);
  } else {
    restore_int_vector_mret((void *)memlayout->int_reg_cpt_addr);
  }
}
