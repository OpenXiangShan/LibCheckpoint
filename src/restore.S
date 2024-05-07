/***************************************************************************************
* Copyright (c) 2020-2022 Institute of Computing Technology, Chinese Academy of Sciences
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "encoding.h"
#include "csr.h"

  .option norvc
  .section ".restore.text","ax",@progbits
  .global restore_csr_vector
restore_csr_vector:
  mv t0, a0
  CSRS(CSRS_RESTORE)
  li a0, 1
  ret

  .global rvh_support_check
rvh_support_check:
  csrr t0, misa
  li t1, MISA_H
  and a0, t0, t1
  ret

  .global rvv_support_check
rvv_support_check:
  csrr t0, misa
  li t1, MISA_V
  and a0, t0, t1
  ret

  .global rvh_csr_restore
rvh_csr_restore:
  mv t0, a0
  HCSRS(CSRS_RESTORE)
  li a0, 1
  ret

  .global rvv_restore
rvv_restore:
  # prepare v csrs, might not be necessary
  mv t0, a0
  li t2, VTYPE_ID
  slli t2, t2, 3
  add t2, t0, t2
  ld t1, (t2)
  li t2, VL_ID
  slli t2, t2, 3
  add t2, t0, t2
  ld t2, (t2)
  vsetvl t2, t2, t1
  VCSRS(CSRS_RESTORE)

  addi a1, a1, 0
  vl1re64.v v0, (a1)
  addi a1, a1, 16
  vl1re64.v v1, (a1)
  addi a1, a1, 16
  vl1re64.v v2, (a1)
  addi a1, a1, 16
  vl1re64.v v3, (a1)
  addi a1, a1, 16
  vl1re64.v v4, (a1)
  addi a1, a1, 16
  vl1re64.v v5, (a1)
  addi a1, a1, 16
  vl1re64.v v6, (a1)
  addi a1, a1, 16
  vl1re64.v v7, (a1)
  addi a1, a1, 16
  vl1re64.v v8, (a1)
  addi a1, a1, 16
  vl1re64.v v9, (a1)
  addi a1, a1, 16
  vl1re64.v v10, (a1)
  addi a1, a1, 16
  vl1re64.v v11, (a1)
  addi a1, a1, 16
  vl1re64.v v12, (a1)
  addi a1, a1, 16
  vl1re64.v v13, (a1)
  addi a1, a1, 16
  vl1re64.v v14, (a1)
  addi a1, a1, 16
  vl1re64.v v15, (a1)
  addi a1, a1, 16
  vl1re64.v v16, (a1)
  addi a1, a1, 16
  vl1re64.v v17, (a1)
  addi a1, a1, 16
  vl1re64.v v18, (a1)
  addi a1, a1, 16
  vl1re64.v v19, (a1)
  addi a1, a1, 16
  vl1re64.v v20, (a1)
  addi a1, a1, 16
  vl1re64.v v21, (a1)
  addi a1, a1, 16
  vl1re64.v v22, (a1)
  addi a1, a1, 16
  vl1re64.v v23, (a1)
  addi a1, a1, 16
  vl1re64.v v24, (a1)
  addi a1, a1, 16
  vl1re64.v v25, (a1)
  addi a1, a1, 16
  vl1re64.v v26, (a1)
  addi a1, a1, 16
  vl1re64.v v27, (a1)
  addi a1, a1, 16
  vl1re64.v v28, (a1)
  addi a1, a1, 16
  vl1re64.v v29, (a1)
  addi a1, a1, 16
  vl1re64.v v30, (a1)
  addi a1, a1, 16
  vl1re64.v v31, (a1)
  li a0, 1
  ret


  .global restore_mtime
restore_mtime:
  li t0, CLINT_MMIO + CLINT_MTIME
  mv t1, a0 # source addr
  ld t1, (t1) # load mtime value from checkpoint
  sd t1, (t0) # store in device

  li a0, 1
  ret

  .global restore_mtime_cmp
restore_mtime_cmp:
  # set mtime (inaccurate) and mtimecmp
  mv t0, a0 # dst addr
  mv t1, a1 # source addr
  ld t1, (t1)
  sd t1, (t0)

  li a0, 1
  ret

  .global restore_float_vector
restore_float_vector:
  mv t0, a0
  fld f0, (0*8)(t0)
  fld f1, (1*8)(t0)
  fld f2, (2*8)(t0)
  fld f3, (3*8)(t0)
  fld f4, (4*8)(t0)
  fld f5, (5*8)(t0)
  fld f6, (6*8)(t0)
  fld f7, (7*8)(t0)
  fld f8, (8*8)(t0)
  fld f9, (9*8)(t0)
  fld f10, (10*8)(t0)
  fld f11, (11*8)(t0)
  fld f12, (12*8)(t0)
  fld f13, (13*8)(t0)
  fld f14, (14*8)(t0)
  fld f15, (15*8)(t0)
  fld f16, (16*8)(t0)
  fld f17, (17*8)(t0)
  fld f18, (18*8)(t0)
  fld f19, (19*8)(t0)
  fld f20, (20*8)(t0)
  fld f21, (21*8)(t0)
  fld f22, (22*8)(t0)
  fld f23, (23*8)(t0)
  fld f24, (24*8)(t0)
  fld f25, (25*8)(t0)
  fld f26, (26*8)(t0)
  fld f27, (27*8)(t0)
  fld f28, (28*8)(t0)
  fld f29, (29*8)(t0)
  fld f30, (30*8)(t0)
  fld f31, (31*8)(t0)
  li a0, 1
  ret

  .global restore_int_vector
restore_int_vector:
  mv sp, a0
  ld x1, (1*8)(sp)
  ld x3, (3*8)(sp)
  ld x4, (4*8)(sp)
  ld x5, (5*8)(sp)
  ld x6, (6*8)(sp)
  ld x7, (7*8)(sp)
  ld x8, (8*8)(sp)
  ld x9, (9*8)(sp)
  ld x10, (10*8)(sp)
  ld x11, (11*8)(sp)
  ld x12, (12*8)(sp)
  ld x13, (13*8)(sp)
  ld x14, (14*8)(sp)
  ld x15, (15*8)(sp)
  ld x16, (16*8)(sp)
  ld x17, (17*8)(sp)
  ld x18, (18*8)(sp)
  ld x19, (19*8)(sp)
  ld x20, (20*8)(sp)
  ld x21, (21*8)(sp)
  ld x22, (22*8)(sp)
  ld x23, (23*8)(sp)
  ld x24, (24*8)(sp)
  ld x25, (25*8)(sp)
  ld x26, (26*8)(sp)
  ld x27, (27*8)(sp)
  ld x28, (28*8)(sp)
  ld x29, (29*8)(sp)
  ld x30, (30*8)(sp)
  ld x31, (31*8)(sp)
  ld sp, (2*8)(sp)
  mret
