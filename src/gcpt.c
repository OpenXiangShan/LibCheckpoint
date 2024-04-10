#include "checkpoint.pb.h"
#include "decode_restore.h"
#include "gcpt_asm.h"
#include "gcpt_trap.h"
#include "pb.h"
#include "printf.h"
#include "utils.h"
#include <pb_decode.h>
#include <pb_encode.h>
#include <string.h>

#define MAGIC_NUMBER         0xdeadbeef
#define RESET_VECTOR         0x80000000
#define GCPT_DEVICE_ADDR     0x60000000
#define PROTOBUF_BUFFER_SIZE 8192

#ifdef ENCODE_DECODE_CHECK
__attribute__((unused)) static char memory_buffer[0x100000];
#endif

void *get_memory_buffer() {
#ifdef ENCODE_DECODE_CHECK
  return memory_buffer;
#else
  return (void *)GCPT_DEVICE_ADDR;
#endif
}

void (*_start_payload)();

#define glue(a, b) a##b
#define PROTOBUF_DECODE(FIELDS, FIELD_NAME)                              \
  static bool glue(FIELD_NAME, _decode)(pb_istream_t * stream,           \
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

PROTOBUF_DECODE(checkpoint_header_fields, header)
PROTOBUF_DECODE(single_core_rvgc_rvv_rvh_memlayout_fields, memlayout)

#ifdef ENCODE_DECODE_CHECK
static bool encode_check() {
  bool status;
  pb_ostream_t stream = pb_ostream_from_buffer(get_memory_buffer(), 4096);

  status = pb_encode_ex(&stream, checkpoint_header_fields,
                        &multicore_default_header, PB_ENCODE_NULLTERMINATED);
  if (!status) {
    printf("LOG: header encode error %s\n", stream.errmsg);
    return false;
  }

  status = pb_encode_ex(&stream, single_core_rvgc_rvv_rvh_memlayout_fields,
                        &multicore_default_layout, PB_ENCODE_NULLTERMINATED);
  if (!status) {
    printf("LOG: body encode error %s\n", stream.errmsg);
    return false;
  }

  return true;
}
#endif

void __attribute__((section(".text.c_start"))) gcpt_c_start(int cpu_id) {
  __attribute__((unused)) int signal = GOOD_TRAP;
  // must clear bss
  clear_bss();
  // will be replaced when csr restore
  enable_gcpt_trap();

#ifdef ENCODE_DECODE_CHECK
  if (!test_encode()) {
    signal = FLAG_CHECK_ERROR;
    goto failed;
  }
  printf("write success\n");
#endif

  checkpoint_header header;
  single_core_rvgc_rvv_rvh_memlayout memlayout;

  pb_istream_t stream =
    pb_istream_from_buffer((void *)get_memory_buffer(), PROTOBUF_BUFFER_SIZE);

  bool header_decode_result    = false;
  bool memlayout_decode_result = false;

  header_decode_result = header_decode(&stream, &header);
  if (header_decode_result == true && header.magic_number == MAGIC_NUMBER) {
    printf("Header magic number %lx\n", header.magic_number);
  } else {
    goto boot_payload;
  }

  memlayout_decode_result = memlayout_decode(&stream, &memlayout);
  if (memlayout_decode_result == false) {
    goto boot_payload;
  }

  multicore_decode_restore((uint64_t)get_memory_buffer() + header.cpt_offset,
                           header.single_core_size, cpu_id, &memlayout);
  // should not be here
  nemu_signal(SHOULD_NOT_BE_HERE);

boot_payload:
#ifdef ENCODE_DECODE_CHECK
  printf("Check error from encode or decode\n");
  signal = ENCODE_DECODE_CHECK_ERROR;
  nemu_signal(signal)
#endif

    printf("Will boot from payload\n");
  _start_payload();

#ifdef ENCODE_DECODE_CHECK
failed:
  printf("Get failed\n");
  nemu_signal(signal);
#endif
}
