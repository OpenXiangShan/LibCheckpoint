/* See LICENSE for license details. */

OUTPUT_ARCH( "riscv" )

ENTRY(place_of_gcpt_size)

SECTIONS
{

  /*--------------------------------------------------------------------*/
  /* Code and read-only segment                                         */
  /*--------------------------------------------------------------------*/

  /* Beginning of code and text segment */
  . = ALIGN(4);
  gcpt_begin = .;

  s_text = .;
  .text : {

    *(.restore.size)
    *(.restore.location)
    *(.text.entry)
    *(.text.c_start)
    *(.text.enable_gcpt_trap)
    *(.text.disable_gcpt_trap)
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

  s_restore = .;
  .restore : {

      *(.restore.text)
      *(.restore.coreN_text)

  }
  e_restore = .;

  . = ALIGN(8);
  .flags : {
      *(.gcpt_flag)
  }

  gcpt_end = .;

  payload_start = .;

  . = DEFINED(GCPT_PAYLOAD_PATH) ? gcpt_begin + 0xFFFD0 : .;

  .before_payload :
  {
    PROVIDE(_payload_start = .);
    *(.before_payload)
    . = ALIGN(8);
    PROVIDE(_payload_end = .);
  }

  . = DEFINED(GCPT_PAYLOAD_POSITION) ? GCPT_PAYLOAD_POSITION : .;

  .payload : {
    *(.payload.bin)
  }
}
