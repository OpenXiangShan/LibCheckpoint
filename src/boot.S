  .option norvc

  .section ".text.entry", "ax", @progbits
  .global _entry
_entry:
  csrr a0, mhartid
  mv t0, a0
  slli t0, t0, 12
  li t1, 4096
  add t0, t0, t1
  la sp, gcpt_stack
  add sp, sp, t0
  j gcpt_c_start

  .section ".bss.stack"
  .align 2
  .global gcpt_stack
gcpt_stack:
  .space 4096 * 128
  .global gcpt_stack_top
gcpt_stack_top:

  .section ".payload", "ax", %progbits
  .align 4
  .global payload_bin
payload_bin:
#ifndef GCPT_PAYLOAD_PATH
#else
  .incbin GCPT_PAYLOAD_PATH
#endif