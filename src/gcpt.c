#include "checkpoint.pb.h"
#include "csr.h"
#include "decode_restore.h"
#include "gcpt_asm.h"
#include "gcpt_trap.h"
#include "pb.h"
#include "printf.h"
#include "spinlock.h"
#include "utils.h"
#include <pb_decode.h>
#include <pb_encode.h>
#include <stdint.h>
#include <string.h>

#define MAGIC_NUMBER         0xdeadbeef
#define RESET_VECTOR         0x80000000
#define BOOT_LOADER          0x80100000
#define GCPT_DEVICE_ADDR     0x60000000
#define PROTOBUF_BUFFER_SIZE 4096

// #define USING_QEMU_DUAL_CORE_SYSTEM
// #define ENCODE_DECODE_CHECK

void *get_memory_buffer() {
#ifdef USING_BARE_METAL_WORKLOAD
  return (void *)0x80000000;
#else
#ifdef USING_QEMU_DUAL_CORE_SYSTEM
  return (void *)0x80000000;
#else
  return (void *)GCPT_DEVICE_ADDR;
#endif
#endif
}


#define GLUE(a, b) a##b
#define PROTOBUF_DECODE(FIELDS, FIELD_NAME)                              \
  static bool GLUE(FIELD_NAME, _decode)(pb_istream_t * stream,           \
                                        void *memlayout) {               \
    bool status =                                                        \
      pb_decode_ex(stream, FIELDS, memlayout, PB_ENCODE_NULLTERMINATED); \
    if (!status) {                                                       \
      printf("%s decode filed: %s\n", #FIELD_NAME, stream->errmsg);      \
      return false;                                                      \
    }                                                                    \
    printf("%s decode success\n", #FIELD_NAME);                          \
    return true;                                                         \
  }

__attribute__((unused))
PROTOBUF_DECODE(checkpoint_header_fields, header)

__attribute__((unused))
PROTOBUF_DECODE(single_core_rvgc_rvv_rvh_memlayout_fields, memlayout)

// Depending on the link script, the hardware state may be overwritten and
// therefore suspended
#ifdef SUPPORT_ORIGINAL_CHECKPOINT
int try_restore_from_rvgcvh_single_core_memlayout() {
#define CPT_MAGIC_BUMBER 0xbeef
#define BOOT_FLAG_ADDR   0x800ECDB0
  if ((uint64_t)gcpt_end >= BOOT_FLAG_ADDR) {
    printf("GCTP has cross over hardware status in memory, could not check rvgcvh memlayout checkpoint\n", "lalal");
    return -1;
  }

  if ((*((uint32_t *)BOOT_FLAG_ADDR)) == CPT_MAGIC_BUMBER) {
    single_core_rvv_rvh_rvgc_restore(&default_rvgc_v_h_memlayout);
    return 0;
  }else {
    printf("GCPT Not found single core rvgcvh checkpoint boot flag in memory\n");
    return -1;
  }
#undef CPT_MAGIC_BUMBER
#undef BOOT_FLAG_ADDR
}

int try_restore_from_rvgc_original_single_core_memlayout() {
#define CPT_MAGIC_BUMBER 0xbeef
#define BOOT_FLAG_ADDR   0x80000f00
  if ((uint64_t)gcpt_end >= BOOT_FLAG_ADDR) {
    printf("GCTP has cross over hardware status in memory, could not check original checkpoint\n");
    return -1;
  }

  if ((*((uint32_t *)BOOT_FLAG_ADDR)) == CPT_MAGIC_BUMBER) {
    single_core_rvv_rvh_rvgc_restore(&default_original_rvgc_memlayout);
    return 0;
  }else {
    printf("GCPT Not found original single core checkpoint boot flag in memory\n");
    return -1;
  }
#undef CPT_MAGIC_BUMBER
#undef BOOT_FLAG_ADDR
}
#endif


spinlock_t bss_lock = {.lock = 0};
void __attribute__((section(".text.c_start"))) gcpt_c_start(int cpu_id) {
  __attribute__((unused)) int signal = GOOD_TRAP;
  // must clear bss
  if (cpu_id == 0) {
    clear_bss();
    mb();
    atomic_set(&bss_lock.lock, 1);
  } else{
    while (atomic_read(&bss_lock.lock) == 0); 
    mb();
  }

  mt_printf("Hello, gcpt at cpu %d\n", cpu_id);

  enable_gcpt_trap();

#ifdef USING_BARE_METAL_WORKLOAD
  #define CPT_MAGIC_BUMBER 0xbeef
  uint64_t *cpt_magic_number_addr = (void*)((uint64_t)0x80000000 + (uint64_t)0xECDB0);
  if (*cpt_magic_number_addr != CPT_MAGIC_BUMBER) {
    goto boot_payload;
  }
  multicore_decode_restore((uint64_t)get_memory_buffer(),
                           1024 * 1024, cpu_id, NULL);
  // should not be here
  nemu_signal(SHOULD_NOT_BE_HERE);
#endif

#ifdef USING_QEMU_DUAL_CORE_SYSTEM
  #define CPT_MAGIC_BUMBER 0xbeef
  uint64_t *cpt_magic_number_addr = (void*)((uint64_t)0x80000000 + (uint64_t)0x300000 + (uint64_t)0xECDB0);
  if (*cpt_magic_number_addr != CPT_MAGIC_BUMBER) {
    goto boot_payload;
  }
  multicore_decode_restore((uint64_t)get_memory_buffer() + 0x300000,
                           1024 * 1024, cpu_id, NULL);
  // should not be here
  nemu_signal(SHOULD_NOT_BE_HERE);
#else

  checkpoint_header header;
  single_core_rvgc_rvv_rvh_memlayout memlayout;

  pb_istream_t stream =
    pb_istream_from_buffer((void *)get_memory_buffer(), PROTOBUF_BUFFER_SIZE);

  bool header_decode_result    = false;
  bool memlayout_decode_result = false;

  header_decode_result = header_decode(&stream, &header);
  if (header_decode_result == true && header.magic_number == MAGIC_NUMBER) {
    printf("Get header magic number %lx\n", header.magic_number);
  } else {
    printf("Decode header from device failed or get magic number failed\n");
    goto boot_payload;
  }

  memlayout_decode_result = memlayout_decode(&stream, &memlayout);
  if (memlayout_decode_result == false) {
    printf("Decode memlayout failed\n");
    goto boot_payload;
  }

  multicore_decode_restore((uint64_t)get_memory_buffer() + header.cpt_offset,
                           header.single_core_size, cpu_id, &memlayout);
#endif

  // should not be here
  nemu_signal(SHOULD_NOT_BE_HERE);

  extern void payload_bin();
  extern void before_boot_payload();
boot_payload:
  mt_printf("Will boot payload from %p\n", payload_bin);
  disable_gcpt_trap();
  before_boot_payload();
  mt_printf("Not found payload!\n");
}
