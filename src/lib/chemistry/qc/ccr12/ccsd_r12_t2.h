//
// ccsd_r12_t2.h --- T2 residual evaluator of the full CCSD-R12
//
// Copyright (C) 2009 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki.toru@gmail.com>
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

#ifndef _chemistry_qc_ccr12_ccsd_r12_t2_h
#define _chemistry_qc_ccr12_ccsd_r12_t2_h

#include <chemistry/qc/ccr12/tensor.h>
#include <chemistry/qc/ccr12/ccr12_info.h>


namespace sc {

class CCSD_R12_T2 {

  protected:
   CCR12_Info* z;
   std::vector<Tensor*> in;

   void offset_smith_0_1();
   void smith_0_1_0(); //z->f1()=>in.at(1)
   void offset_smith_1_6();
   void smith_1_6_0(); //z->f1()=>in.at(2)
   void smith_2_37(); //z->t1(),z->v2()=>in.at(2)
   void smith_1_6(); //z->t1(),in.at(2)=>in.at(1)
   void smith_1_28(); //z->t1(),z->v2()=>in.at(1)
   void smith_1_42(); //z->t2(),z->v2()=>in.at(1)
   void smith_1_47(); //z->c2(),z->vr2()=>in.at(1)
   void smith_0_1(Ref<Tensor>&); //z->t2(),in.at(1)=>Ref<Tensor>&
   void offset_smith_0_2();
   void smith_0_2_0(); //z->f1()=>in.at(1)
   void smith_1_29(); //z->t1(),z->v2()=>in.at(1)
   void smith_1_40(); //z->t2(),z->v2()=>in.at(1)
   void smith_1_45(); //z->qy(),z->v2()=>in.at(1)
   void smith_0_2(Ref<Tensor>&); //z->t2(),in.at(1)=>Ref<Tensor>&
   void offset_smith_0_3();
   void smith_0_3_0(); //z->f1()=>in.at(1)
   void smith_1_20(); //z->t1(),z->v2()=>in.at(1)
   void smith_1_43(); //z->t2(),z->v2()=>in.at(1)
   void smith_1_49(); //z->qy(),z->v2()=>in.at(1)
   void smith_0_3(Ref<Tensor>&); //z->qy(),in.at(1)=>Ref<Tensor>&
   void offset_smith_0_4();
   void smith_0_4_0(); //z->v2()=>in.at(1)
   void offset_smith_1_4();
   void smith_1_4_0(); //z->f1()=>in.at(2)
   void smith_2_31(); //z->t1(),z->v2()=>in.at(2)
   void smith_1_4(); //z->qy(),in.at(2)=>in.at(1)
   void offset_smith_1_5();
   void smith_1_5_0(); //z->f1()=>in.at(2)
   void smith_2_38(); //z->t1(),z->v2()=>in.at(2)
   void smith_1_5(); //z->t2(),in.at(2)=>in.at(1)
   void offset_smith_1_10();
   void smith_1_10_0(); //z->v2()=>in.at(2)
   void offset_smith_2_18();
   void smith_2_18_0(); //z->v2()=>in.at(3)
   void smith_3_30(); //z->t1(),z->v2()=>in.at(3)
   void smith_2_18(); //z->t1(),in.at(3)=>in.at(2)
   void smith_2_33(); //z->c2(),z->vr2()=>in.at(2)
   void smith_2_34(); //z->t2(),z->v2()=>in.at(2)
   void smith_1_10(); //z->t1(),in.at(2)=>in.at(1)
   void offset_smith_1_11();
   void smith_1_11_0(); //z->v2()=>in.at(2)
   void smith_2_19(); //z->t1(),z->v2()=>in.at(2)
   void smith_1_11(); //z->t1(),in.at(2)=>in.at(1)
   void offset_smith_1_21();
   void smith_1_21_0(); //z->v2()=>in.at(2)
   void smith_2_32(); //z->t1(),z->v2()=>in.at(2)
   void smith_1_21(); //z->qy(),in.at(2)=>in.at(1)
   void smith_1_23(); //z->c2(),z->vr2()=>in.at(1)
   void offset_smith_1_24();
   void smith_1_24_0(); //z->v2()=>in.at(2)
   void smith_2_36(); //z->t1(),z->v2()=>in.at(2)
   void smith_1_24(); //z->t2(),in.at(2)=>in.at(1)
   void smith_1_25(); //z->t2(),z->v2()=>in.at(1)
   void smith_0_4(Ref<Tensor>&); //z->t1(),in.at(1)=>Ref<Tensor>&
   void smith_0_7(Ref<Tensor>&); //z->v2()=>Ref<Tensor>&
   void offset_smith_0_9();
   void smith_0_9_0(); //z->v2()=>in.at(1)
   void smith_1_12(); //z->t1(),z->v2()=>in.at(1)
   void smith_0_9(Ref<Tensor>&); //z->t1(),in.at(1)=>Ref<Tensor>&
   void offset_smith_0_13();
   void smith_0_13_0(); //z->v2()=>in.at(1)
   void offset_smith_1_26();
   void smith_1_26_0(); //z->v2()=>in.at(2)
   void smith_2_35(); //z->t1(),z->v2()=>in.at(2)
   void smith_1_26(); //z->t1(),in.at(2)=>in.at(1)
   void smith_1_41(); //z->t2(),z->v2()=>in.at(1)
   void smith_1_46(); //z->c2(),z->vr2()=>in.at(1)
   void smith_0_13(Ref<Tensor>&); //z->t2(),in.at(1)=>Ref<Tensor>&
   void offset_smith_0_14();
   void smith_0_14_0(); //z->v2()=>in.at(1)
   void smith_1_27(); //z->t1(),z->v2()=>in.at(1)
   void smith_1_39(); //z->t2(),z->v2()=>in.at(1)
   void smith_1_44(); //z->qy(),z->v2()=>in.at(1)
   void smith_0_14(Ref<Tensor>&); //z->t2(),in.at(1)=>Ref<Tensor>&
   void smith_0_15(Ref<Tensor>&); //z->t2(),z->v2()=>Ref<Tensor>&
   void offset_smith_0_16();
   void smith_0_16_0(); //z->v2()=>in.at(1)
   void smith_1_22(); //z->t1(),z->v2()=>in.at(1)
   void smith_1_48(); //z->qy(),z->v2()=>in.at(1)
   void smith_0_16(Ref<Tensor>&); //z->qy(),in.at(1)=>Ref<Tensor>&
   void smith_0_17(Ref<Tensor>&); //z->c2(),z->vr2()=>Ref<Tensor>&

  public:
   CCSD_R12_T2(CCR12_Info* info);
   ~CCSD_R12_T2();
    
   void compute_amp(Ref<Tensor>& out);

};



}

#endif


