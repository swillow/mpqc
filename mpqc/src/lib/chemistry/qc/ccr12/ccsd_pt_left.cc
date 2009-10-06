//
// ccsd_pt_left.cc
//
// Copyright (C) 2009 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@qtp.ufl.edu>
// Maintainer: TS
//
// This file is part of the SC Toolkit.
//
// The SC Toolkit is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The SC Toolkit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the SC Toolkit; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
// The U.S. Government is granted a limited license as per AL 91-7.
//

// This is a C++ code generated by SMITH
  
#include <algorithm>
#include <chemistry/qc/ccr12/ccsd_pt_left.h>
#include <chemistry/qc/ccr12/parenthesis2tnum.h>
using namespace sc;
  
  
CCSD_PT_LEFT::CCSD_PT_LEFT(CCR12_Info* info):Parenthesis2tNum(info){};
  
  
  
void CCSD_PT_LEFT::compute_amp(double** heap,const long t_p4b,const long t_p5b,const long t_p6b,const long t_h1b,const long t_h2b,const long t_h3b,const long toggle){
  
if (toggle==2L) {
  smith_0_1(heap,t_p4b,t_p5b,t_p6b,t_h1b,t_h2b,t_h3b);
}
  
}
  
void CCSD_PT_LEFT::smith_0_1(double** heap,const long t_p4b,const long t_p5b,const long t_p6b,const long t_h1b,const long t_h2b,const long t_h3b){ 
      
double* a_i0 = heap[0];
double* k_a0 = heap[1];
double* k_a0_sort = heap[2];
double* k_a1 = heap[3];
double* k_a1_sort = heap[4];
double* k_c_sort = heap[5];

const long perm[9][6]={0,1,2,3,4,5, 0,1,2,4,3,5, 0,1,2,5,3,4, 1,0,2,3,4,5, 1,0,2,4,3,5, 1,0,2,5,3,4, 2,0,1,3,4,5, 2,0,1,4,3,5, 2,0,1,5,3,4}; 
const long t_b[6]={t_p4b,t_p5b,t_p6b,t_h1b,t_h2b,t_h3b}; 
for (long permutation=0L;permutation<9L;++permutation) { 
 const long p4b=t_b[perm[permutation][0]]; 
 const long p5b=t_b[perm[permutation][1]]; 
 const long p6b=t_b[perm[permutation][2]]; 
 const long h1b=t_b[perm[permutation][3]]; 
 const long h2b=t_b[perm[permutation][4]]; 
 const long h3b=t_b[perm[permutation][5]]; 
 bool skip=false; 
 for (long p_p=0L;p_p<permutation;++p_p) { 
  if(p4b==t_b[perm[p_p][0]] 
  && p5b==t_b[perm[p_p][1]] 
  && p6b==t_b[perm[p_p][2]] 
  && h1b==t_b[perm[p_p][3]] 
  && h2b==t_b[perm[p_p][4]] 
  && h3b==t_b[perm[p_p][5]]) skip=true; 
 } 
 if (skip) continue; 
 if (!z->restricted() || z->get_spin(p4b)+z->get_spin(p5b)+z->get_spin(p6b)+z->get_spin(h1b)+z->get_spin(h2b)+z->get_spin(h3b)!=12L) { 
  if (z->get_spin(p4b)+z->get_spin(p5b)+z->get_spin(p6b)==z->get_spin(h1b)+z->get_spin(h2b)+z->get_spin(h3b)) { 
   if ((z->get_sym(p4b)^(z->get_sym(p5b)^(z->get_sym(p6b)^(z->get_sym(h1b)^(z->get_sym(h2b)^z->get_sym(h3b))))))==(z->irrep_t()^z->irrep_v())) { 
    long dimc=z->get_range(p4b)*z->get_range(p5b)*z->get_range(p6b)*z->get_range(h1b)*z->get_range(h2b)*z->get_range(h3b); 
    std::fill(k_c_sort,k_c_sort+(size_t)dimc,0.0); 
    if (z->get_spin(p4b)==z->get_spin(h1b)) { 
     if ((z->get_sym(p4b)^z->get_sym(h1b))==z->irrep_t()) { 
      long p4b_0,h1b_0; 
      z->restricted_2(p4b,h1b,p4b_0,h1b_0); 
      long p5b_1,p6b_1,h2b_1,h3b_1; 
      z->restricted_4(p5b,p6b,h2b,h3b,p5b_1,p6b_1,h2b_1,h3b_1); 
      long dim_common=1L; 
      long dima0_sort=z->get_range(p4b)*z->get_range(h1b); 
      long dima0=dim_common*dima0_sort; 
      long dima1_sort=z->get_range(p5b)*z->get_range(p6b)*z->get_range(h2b)*z->get_range(h3b); 
      long dima1=dim_common*dima1_sort; 
      if (dima0>0L && dima1>0L) { 
       z->t1()->get_block(h1b_0+z->noab()*(p4b_0-z->noab()),k_a0); 
       z->sort_indices2(k_a0,k_a0_sort,z->get_range(p4b),z->get_range(h1b),1,0,+1.0); 
       z->v2()->get_block(h3b_1+(z->nab())*(h2b_1+(z->nab())*(p6b_1+(z->nab())*(p5b_1))),k_a1); 
       z->sort_indices4(k_a1,k_a1_sort,z->get_range(p5b),z->get_range(p6b),z->get_range(h2b),z->get_range(h3b),3,2,1,0,+1.0); 
       double factor=1.0; 
       z->smith_dgemm(dima0_sort,dima1_sort,dim_common,factor,k_a0_sort,dim_common,k_a1_sort,dim_common,1.0,k_c_sort,dima0_sort); 
      } 
     } 
    } 
    if (p5b>=p4b && h2b>=h1b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),5,3,2,4,1,0,+1.0); 
    } 
    if (p5b>=p4b && h3b>=h1b && h1b>=h2b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),5,3,2,1,4,0,-1.0); 
    } 
    if (p5b>=p4b && h1b>=h3b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),5,3,2,1,0,4,+1.0); 
    } 
    if (p6b>=p4b && p4b>=p5b && h2b>=h1b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),3,5,2,4,1,0,-1.0); 
    } 
    if (p6b>=p4b && p4b>=p5b && h3b>=h1b && h1b>=h2b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),3,5,2,1,4,0,+1.0); 
    } 
    if (p6b>=p4b && p4b>=p5b && h1b>=h3b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),3,5,2,1,0,4,-1.0); 
    } 
    if (p4b>=p6b && h2b>=h1b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),3,2,5,4,1,0,+1.0); 
    } 
    if (p4b>=p6b && h3b>=h1b && h1b>=h2b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),3,2,5,1,4,0,-1.0); 
    } 
    if (p4b>=p6b && h1b>=h3b) { 
     z->sort_indices_acc6(k_c_sort,a_i0,z->get_range(h3b),z->get_range(h2b),z->get_range(p6b),z->get_range(p5b),z->get_range(h1b),z->get_range(p4b),3,2,5,1,0,4,+1.0); 
    } 
   } 
  } 
 } 
} 
} 

