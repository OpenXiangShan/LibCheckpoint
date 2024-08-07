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

  .section ".data"
target_address:
#ifndef GCPT_PAYLOAD_POSITION
  .word 0x80100000
#else
  .word GCPT_PAYLOAD_POSITION
#endif

  .section ".before_payload", "ax", %progbits
  .align 4
  .global before_boot_payload
before_boot_payload:
  li a1, 0
  csrr a0, mhartid
  la t0, target_address
  ld t0, (t0)
  jalr zero, 0(t0)

  .section ".payload.bin", "ax", %progbits
  .align 4
  .global payload_bin
payload_bin:
#ifndef GCPT_PAYLOAD_PATH
  li a0, 1
  ret
#else
  .incbin GCPT_PAYLOAD_PATH
#endif
