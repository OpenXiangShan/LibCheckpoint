/* See LICENSE for license details. */

OUTPUT_ARCH( "riscv" )

ENTRY(_entry)

SECTIONS
{

  /*--------------------------------------------------------------------*/
  /* Code and read-only segment                                         */
  /*--------------------------------------------------------------------*/

  /* Beginning of code and text segment */
  . = 0x80000000;
  gcpt_begin = .;

  s_text = .;
  .text : {

    *(.text.entry)
    *(.text.c_start)
    *(.text.enable_gcpt_trap)
    *(.text.gcpt_trap_handler)
    *(.text.gcpt_trap_entry)

    *(.text .text.*)

  }

  . = ALIGN(4K);
  e_text = .;

  s_rodata = .;
  .rodata : {

    *(.rodata .rodata.*)

  }
  . = ALIGN(4K);
  e_rodata = .;

  s_data = .;
  .data : {

    *(.data)

    *(.data.*)

    *(.sdata .sdata.*)

  }
  . = ALIGN(4K);
  e_data = .;

  . = ALIGN(4K);
  .bss : {

    *(.bss.stack)

    s_bss = .;

    *(.bss .bss.*)

    *(.sbss .sbss.*)

  }
  . = ALIGN(4K);
  e_bss = .;

  .restore : {

      *(.restore.text)

  }

  gcpt_end = .;

  . = 0x80100000;
  payload_start = .;
  .payload :
  {
    PROVIDE(_payload_start = .);
    *(.payload)
    . = ALIGN(8);
    PROVIDE(_payload_end = .);
  }
}
