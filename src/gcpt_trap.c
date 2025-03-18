#include "gcpt_trap.h"
#include "encoding.h"
#include "gcpt_asm.h"
#include "utils.h"
#include <stdint.h>

void __attribute__((section(".text.gcpt_trap_handler"))) gcpt_trap_handler() {
  mt_printf("mepc:    0x%lx\n", read_csr(mepc)); // mepc
  mt_printf("mtval    0x%lx\n", read_csr(mtval)); // mtval
  mt_printf("mcause   0x%lx\n", read_csr(mcause)); // mcause
  mt_printf("mstatus  0x%lx\n", read_csr(mstatus)); // mstatus
  mt_printf("Get gcpt trap\n");
  nemu_signal(GCPT_GET_TRAP);
}

void __attribute__((section(".text.enable_gcpt_trap"))) enable_gcpt_trap() {
  uint64_t mtvec = (uint64_t)gcpt_trap & 0xFFFFFFFFFFFFFFFC;
  uint64_t mstatus = read_csr(mstatus) | MSTATUS_MIE;
  write_csr(mstatus, mstatus);
  write_csr(mtvec, mtvec);
}


void __attribute__((section(".text.disable_gcpt_trap"))) disable_gcpt_trap() {
  uint64_t mtvec = (uint64_t)0;
  // enable mie will cause secondary hart boot failed
  uint64_t mstatus = read_csr(mstatus) & ~MSTATUS_MIE;
  write_csr(mstatus, mstatus);
  write_csr(mtvec, mtvec);
}