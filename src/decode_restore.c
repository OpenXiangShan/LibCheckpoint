#include "decode_restore.h"
#include "checkpoint.pb.h"
#include "csr.h"
#include "encoding.h"
#include "gcpt_asm.h"
#include "spinlock.h"
#include "gcpt_trap.h"
#include "printf.h"

spinlock_t globash_restore_lock;
static uint64_t release_restore = 0;
static uint64_t global_atomic_restore_counter = 0;

#define RESTORE_CHECK(func, name, success_status, ...) \
  {                                                    \
    if (func(__VA_ARGS__) == success_status) {         \
      printf(#name " restore success\n");              \
    } else {                                           \
      printf(#name " restore failed\n");               \
    }                                                  \
  }

bool checkpoint_possibility_restore_check(uint64_t *csr_address,
                                          int *rvv_could_restore,
                                          int *rvh_could_restore,
                                          int *rvf_could_restore,
                                          uint64_t *mode_address) {
  uint64_t host_mstatus = read_csr(mstatus);
  uint64_t host_misa    = read_csr(misa);

  uint64_t checkpoint_mstatus =
    *(uint64_t *)((uint64_t)csr_address + (0x300 << 3));
  uint64_t checkpoint_misa =
    *(uint64_t *)((uint64_t)csr_address + (0x301 << 3));

  uint64_t checkpoint_mode = *(mode_address);

  if ((checkpoint_mode << 11) == (uint64_t)MSTATUS_MPP) {
    printf("checkpoint mode is M %lx\n", checkpoint_mode);
    return false;
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
      // prepare mstatus
      write_csr(mstatus, host_mstatus | MSTATUS_VS_INITIAL);
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
      // prepare mstatus
      write_csr(mstatus, host_mstatus | MSTATUS_FS_INITIAL);
      // can restore F ext
      *rvf_could_restore = 1;
    } else {
      *rvf_could_restore = 0;
    }
  }

  return true;
}

single_core_rvgc_rvv_rvh_memlayout
  get_core_memlayout(uint64_t base_addr, uint64_t single_core_size, int cpu_id,
                     single_core_rvgc_rvv_rvh_memlayout *memlayout) {

  // percpu hardware status offset
  uint64_t offset = cpu_id * single_core_size + base_addr;

  single_core_rvgc_rvv_rvh_memlayout *single_core_memlayout;
  if (memlayout == NULL) {
#ifdef USING_QEMU_DUAL_CORE_SYSTEM
    single_core_memlayout = &default_qemu_memlayout;
#else
    single_core_memlayout = &default_multicore_layout;
#endif
  } else {
    single_core_memlayout = memlayout;
  }

  single_core_rvgc_rvv_rvh_memlayout cpux_memlayout = {
    .pc_cpt_addr         = single_core_memlayout->pc_cpt_addr + offset,
    .mode_cpt_addr       = single_core_memlayout->mode_cpt_addr + offset,
    .mtime_cpt_addr      = single_core_memlayout->mtime_cpt_addr + offset,
    .mtime_cmp_cpt_addr  = single_core_memlayout->mtime_cmp_cpt_addr + offset,
    .misc_done_cpt_addr  = single_core_memlayout->misc_done_cpt_addr + offset,
    .misc_reserve        = single_core_memlayout->misc_reserve + offset,
    .int_reg_cpt_addr    = single_core_memlayout->int_reg_cpt_addr + offset,
    .int_reg_done        = single_core_memlayout->int_reg_done + offset,
    .float_reg_cpt_addr  = single_core_memlayout->float_reg_cpt_addr + offset,
    .float_reg_done      = single_core_memlayout->float_reg_done + offset,
    .csr_reg_cpt_addr    = single_core_memlayout->csr_reg_cpt_addr + offset,
    .csr_reg_done        = single_core_memlayout->csr_reg_done + offset,
    .csr_reserve         = single_core_memlayout->csr_reserve + offset,
    .vector_reg_cpt_addr = single_core_memlayout->vector_reg_cpt_addr + offset,
    .vector_reg_done     = single_core_memlayout->vector_reg_done + offset,
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
  int rvv_could_restore = 0;
  int rvh_could_restore = 0;
  int rvf_could_restore = 0;
  int all_cpu_num = *((uint64_t*)memlayout->csr_reserve);

  if (!checkpoint_possibility_restore_check(
        (void *)memlayout->csr_reg_cpt_addr, &rvv_could_restore,
        &rvh_could_restore, &rvf_could_restore,
        (void *)memlayout->mode_cpt_addr)) {
    nemu_signal(GOOD_TRAP);
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
  printf("all cpu num %d atomic flag %d\n", all_cpu_num, atomic_read(&global_atomic_restore_counter));
  if (atomic_read(&global_atomic_restore_counter) != all_cpu_num && all_cpu_num != 0) {
    spinlock_unlock(&globash_restore_lock);
    while (atomic_read(&release_restore) != 1) {}
  }else{
    spinlock_unlock(&globash_restore_lock);
    atomic_set(&release_restore, 1);
    RESTORE_CHECK(restore_mtime, MTIME, 1,
                (void *)memlayout->mtime_cpt_addr);
  }

  restore_int_vector((void *)memlayout->int_reg_cpt_addr);
}
