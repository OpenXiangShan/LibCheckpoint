#ifndef __CPT_DEFAULT_VALUES_H__
#define __CPT_DEFAULT_VALUES_H__

#include <checkpoint.pb.h>

__attribute__((unused)) static single_core_rvgc_rvv_rvh_memlayout
  default_rvgc_v_h_memlayout = {
    .magic_number_cpt_addr = 0x800ECDB0,
    .pc_cpt_addr         = 0x800ECDB8,
    .mode_cpt_addr       = 0x800ECDC0,
    .mtime_cpt_addr      = 0x800ECDC8,
    .mtime_cmp_cpt_addr  = 0x800ECDD0,
    .misc_done_cpt_addr  = 0x800ECDD8,
    .misc_reserve        = 0x800ECDE0,
    .int_reg_cpt_addr    = 0x800EDDE0,
    .int_reg_done        = 0x800EDEE0,
    .float_reg_cpt_addr  = 0x800EDEE8,
    .float_reg_done      = 0x800EDFE8,
    .csr_reg_cpt_addr    = 0x800EDFF0,
    .csr_reg_done        = 0x800F5FF0,
    .csr_reserve         = 0x800F5FF8,
    .vector_reg_cpt_addr = 0x800FDFF8,
    .vector_reg_done     = 0x800FFFF8,
};

__attribute__((unused)) static single_core_rvgc_rvv_rvh_memlayout
  default_qemu_memlayout = {
    .magic_number_cpt_addr = 0xECDB0,
    .pc_cpt_addr         = 0xECDB8,
    .mode_cpt_addr       = 0xECDC0,
    .mtime_cpt_addr      = 0xECDC8,
    .mtime_cmp_cpt_addr  = 0xECDD0,
    .misc_done_cpt_addr  = 0xECDD8,
    .misc_reserve        = 0xECDE0,
    .int_reg_cpt_addr    = 0xEDDE0,
    .int_reg_done        = 0xEDEE0,
    .float_reg_cpt_addr  = 0xEDEE8,
    .float_reg_done      = 0xEDFE8,
    .csr_reg_cpt_addr    = 0xEDFF0,
    .csr_reg_done        = 0xF5FF0,
    .csr_reserve         = 0xF5FF8,
    .vector_reg_cpt_addr = 0xFDFF8,
    .vector_reg_done     = 0xFFFF8,
};

__attribute__((unused)) static checkpoint_header multicore_default_header = {
  .magic_number = 0xdeadbeef,
  .cpt_offset =
    sizeof(checkpoint_header) + sizeof(single_core_rvgc_rvv_rvh_memlayout),
  .cpu_num      = 1,
  .single_core_size = 1 * 1024 * 1024, // 1M
  .version      = 0x20240205
};

__attribute__((unused)) static single_core_rvgc_rvv_rvh_memlayout
  default_multicore_layout = {
    .magic_number_cpt_addr = 0x0,
    .pc_cpt_addr         = 0x8,
    .mode_cpt_addr       = 0x10,
    .mtime_cpt_addr      = 0x18,
    .mtime_cmp_cpt_addr  = 0x20,
    .misc_done_cpt_addr  = 0x28,
    .misc_reserve        = 0x30,
    .int_reg_cpt_addr    = 0x1000,
    .int_reg_done        = 0x1128,
    .float_reg_cpt_addr  = 0x1130,
    .float_reg_done      = 0x1230,
    .csr_reg_cpt_addr    = 0x1238,
    .csr_reg_done        = 0x9238,
    .csr_reserve         = 0x9240,
    .vector_reg_cpt_addr = 0x11240,
    .vector_reg_done     = 0x13240,
};

#endif

