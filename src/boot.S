#define COMPLETE_FLAG 0xcaff
#define GCPT_INCOMPLETE         0x2

  .option norvc

  .section ".restore.size", "ax", @progbits
  .global place_of_gcpt_size
place_of_gcpt_size:
  j boot_check
  .word gcpt_end-gcpt_begin

  .section ".restore.location", "ax", @progbits
get_restorer_entry_pc:
  auipc t0, 0
  li t1, 0x8
  sub t0, t0, t1
  jalr x0, 0(ra)

  .section ".text.entry", "ax", @progbits
boot_check:
  la s0, read_complete_flag #load read complete flag address
  ld t1, (s0)
  li t2, COMPLETE_FLAG
  bne t1, t2, read_incomplete # read incomplete goto bad_trap
  j _entry

read_incomplete:
  li a0, GCPT_INCOMPLETE
  j read_incomplete_trap

read_incomplete_trap:
  .insn r 0x6B, 0, 0, x0, x0, x0

  .global _entry
_entry:
  csrr a0, mhartid
  mv t0, a0
  slli t0, t0, 12
  li t1, 4096
  add t0, t0, t1
  la sp, gcpt_stack
  add sp, sp, t0
  jal ra, get_restorer_entry_pc
  mv a1, t0
  j gcpt_c_start

  .section ".bss.stack", "aw"
  .align 2
  .global gcpt_stack
gcpt_stack:
  .space 4096 * 128
  .global gcpt_stack_top
gcpt_stack_top:

  .section ".gcpt_flag", "a", @progbits
read_complete_flag:
  .word COMPLETE_FLAG
  .word 0x0

  .section ".before_payload", "ax", %progbits
  .align 2
  .global before_boot_payload
before_boot_payload:
  mv t0, a0
  li a1, 0
  csrr a0, mhartid
  jalr zero, 0(t0)

  .section ".payload.bin", "ax", %progbits
  .align 2
  .global payload_bin
payload_bin:
#ifndef GCPT_PAYLOAD_PATH
  li a0, 1
  ret
#else
  .incbin GCPT_PAYLOAD_PATH
#endif
