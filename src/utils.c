#include "utils.h"
#include "gcpt_asm.h"
#include "stdint.h"

void clear_bss() {
  char *bss_start = (void *)s_bss;
  for (uint64_t i = 0; i < (uint64_t)e_bss - (uint64_t)s_bss; i++) {
    *(bss_start + i) = 0;
  }
}

void *memcpy(void *dst, const void *src, unsigned long size) {
  for (unsigned long i = 0; i < size; i++) {
    *((char *)dst + i) = *((char *)src + i);
  }
  return dst;
}

void *memset(void *dst, int value, unsigned long size) {
  for (unsigned long i = 0; i < size; i++) {
    *((char *)dst + i) = value;
  }
  return dst;
}
