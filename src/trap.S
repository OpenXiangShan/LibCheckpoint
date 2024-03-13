  .section ".text.gcpt_trap_entry","ax",@progbits
  .global gcpt_trap
  .align 2
gcpt_trap:
  j gcpt_trap_handler
