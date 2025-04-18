#ifndef DECODE_RESTORE_H
#define DECODE_RESTORE_H

#include "checkpoint.pb.h"
#include <pb_decode.h>
#include <pb_encode.h>

typedef struct csr_id2name {
  int id;
  char *name;
} csr_id2name_t;

__attribute__((unused)) static csr_id2name_t regs[4096] = {
  [0x100] = {0x100, "sstatus"},       [0x105] = {0x105, "stvec"},
  [0x106] = {0x106, "scounteren"},    [0x140] = {0x140, "sscratch"},
  [0x141] = {0x141, "sepc"},          [0x142] = {0x142, "scause"},
  [0x14d] = {0x14d, "stimecmp"},      [0x180] = {0x180, "satp"},
  [0x1c0] = {0x1c0, "smte"},          [0x200] = {0x200, "vsstatus"},
  [0x300] = {0x300, "mstatus"},       [0x301] = {0x301, "misa"},
  [0x302] = {0x302, "medeleg"},       [0x303] = {0x303, "mideleg"},
  [0x304] = {0x304, "mie"},           [0x305] = {0x305, "mtvec"},
  [0x306] = {0x306, "mcounteren"},    [0x30a] = {0x30a, "menvcfg"},
  [0x310] = {0x310, "mstatush"},      [0x31a] = {0x31a, "menvcfgh"},
  [0x320] = {0x320, "mcountinhibit"}, [0x323] = {0x323, "mhpmevent3"},
  [0x324] = {0x324, "mhpmevent4"},    [0x325] = {0x325, "mhpmevent5"},
  [0x326] = {0x326, "mhpmevent6"},    [0x327] = {0x327, "mhpmevent7"},
  [0x328] = {0x328, "mhpmevent8"},    [0x329] = {0x329, "mhpmevent9"},
  [0x32a] = {0x32a, "mhpmevent10"},   [0x32b] = {0x32b, "mhpmevent11"},
  [0x32c] = {0x32c, "mhpmevent12"},   [0x32d] = {0x32d, "mhpmevent13"},
  [0x32e] = {0x32e, "mhpmevent14"},   [0x32f] = {0x32f, "mhpmevent15"},
  [0x330] = {0x330, "mhpmevent16"},   [0x331] = {0x331, "mhpmevent17"},
  [0x332] = {0x332, "mhpmevent18"},   [0x340] = {0x340, "mscratch"},
  [0x341] = {0x341, "mepc"},          [0x342] = {0x342, "mcause"},
  [0x344] = {0x344, "mip"},           [0x3a0] = {0x3a0, "pmpcfg0"},
  [0x3b0] = {0x3b0, "pmpaddr0"},      [0x3b1] = {0x3b1, "pmpaddr1"},
  [0x3b2] = {0x3b2, "pmpaddr2"},      [0x3b3] = {0x3b3, "pmpaddr3"},
  [0x3c0] = {0x3c0, "mmte"},          [0x600] = {0x600, "hstatus"},
  [0x60a] = {0x60a, "henvcfg"},       [0x61a] = {0x61a, "henvcfgh"},
  [0x646] = {0x646, "hviprio1"},      [0x647] = {0x647, "hviprio2"},
  [0x7a1] = {0x7a1, "tdata1"},        [0x7a4] = {0x7a4, "tinfo"},
  [0xb00] = {0xb00, "mcycle"},        [0xb02] = {0xb02, "minstret"},
  [0xb03] = {0xb03, "mhpmcounter3"},  [0xb80] = {0xb80, "mcycleh"},
  [0xb82] = {0xb82, "minstreth"},     [0xc00] = {0xc00, "cycle"},
  [0xc01] = {0xc01, "time"},          [0xc02] = {0xc02, "instret"},
  [0xc03] = {0xc03, "hpmcounter3"},   [0xc22] = {0xc22, "vlenb"},
  [0xc80] = {0xc80, "cycleh"},        [0xc82] = {0xc82, "instreth"},
  [0xda0] = {0xda0, "scountovf"},     [0xdb0] = {0xdb0, "stopi"},
};

void single_core_rvv_rvh_rvgc_restore(
  single_core_rvgc_rvv_rvh_memlayout *memlayout, int cpu_id);

void multicore_decode_restore(uint64_t cpt_base_address,
                              uint64_t single_core_size, int cpu_id,
                              single_core_rvgc_rvv_rvh_memlayout *memlayout);

#endif
