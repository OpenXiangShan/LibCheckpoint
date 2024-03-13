#include "gcpt_trap.h"
#include "encoding.h"
#include "gcpt_asm.h"
#include "printf.h"
#include <stdint.h>

void __attribute__((section(".text.gcpt_trap_handler"))) gcpt_trap_handler() {
  printf("mepc:    0x%lx\n", read_csr(mepc)); // mepc
  printf("mtval    0x%lx\n", read_csr(mtval)); // mtval
  printf("mcause   0x%lx\n", read_csr(mcause)); // mcause
  printf("mstatus  0x%lx\n", read_csr(mstatus)); // mstatus
  printf("Get gcpt trap\n");
  nemu_signal(GCPT_GET_TRAP);
}

void __attribute__((section(".text.enable_gcpt_trap"))) enable_gcpt_trap() {
  uint64_t mtvec = (uint64_t)gcpt_trap & 0xFFFFFFFFFFFFFFFC;
  write_csr(mtvec, mtvec);
}
