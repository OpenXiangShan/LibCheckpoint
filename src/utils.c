#include "utils.h"
#include "gcpt_asm.h"
#include "spinlock.h"
#include "stdint.h"
#include <stdarg.h>
#include <printf.h>

spinlock_t mt_printf_lock = { .lock = 0 };

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

int mt_printf(const char* format, ...) {
  va_list va;
  va_start(va, format);
  spinlock_lock(&mt_printf_lock);
  const int ret = vprintf(format, va);
  spinlock_unlock(&mt_printf_lock);
  va_end(va);
  return ret;
}

#include <stddef.h>

void __riscv_flush_icache(void* addr, size_t len) {
    asm volatile ("fence.i" ::: "memory");
}
