#ifndef __GCPT_TRAP__
#define __GCPT_TRAP__

#define DISABLE_TIME_INTR   0x100
#define NOTIFY_PROFILER     0x101
#define NOTIFY_PROFILE_EXIT 0x102

#define GOOD_TRAP                 0x0
#define FLAG_CHECK_ERROR          0x1
#define GCPT_INCOMPLETE           0x2
#define VERSION_NOT_MATCH         0x3
#define GCPT_GET_TRAP             0x4
#define ENCODE_DECODE_CHECK_ERROR 0x5
#define SHOULD_NOT_BE_HERE        0x6
#define GCPT_GET_BAD_IMM          0x7

static inline void nemu_signal(int sig) {
  asm volatile("mv a0, %0\n\t"
               ".insn r 0x6B, 0, 0, x0, x0, x0\n\t"
               :
               : "r"(sig)
               : "a0");
}

void enable_gcpt_trap();
#endif
