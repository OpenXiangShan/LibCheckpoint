#ifndef GCPT_ASM
#define GCPT_ASM

void s_bss();
void e_bss();
int payload_start();

int rvh_csr_restore(void *restore_addr);
int rvv_restore(void *rvv_csr_addr, void *rvv_reg_addr);

int restore_csr_vector(void *restore_addr);
int restore_float_vector(void *restore_addr);
int restore_other_csr(void *mtime_cmp_cpt_addr, void *mtime_cpt_addr);
int restore_mtime(void *mtime_cpt_addr);
int restore_mtime_cmp(void *hart_mtime_cmp_addr, void *mtime_cmp_cpt_addr);
int restore_int_vector_jump(void *restore_addr);
int restore_int_vector_mret(void *restore_addr);

int rvh_support_check();
int rvv_support_check();

void gcpt_trap();
void gcpt_start();
void gcpt_end();

#endif
