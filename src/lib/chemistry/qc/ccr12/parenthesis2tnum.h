//
// parenthesis2tnum.h --- base class for the numerators in various (2)T/(2)Q correction
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

#pragma once
#ifndef _chemistry_qc_ccr12_parenthesis2tnum_h
#define _chemistry_qc_ccr12_parenthesis2tnum_h

#include <util/ref/ref.h>
#include <chemistry/qc/ccr12/ccr12_info.h>

namespace sc {
 
/** PTNum is the base class for the numerator in various (2)T/(2)Q models.  */
class Parenthesis2tNum : public RefCount {

  protected:
   CCR12_Info* z;

   std::vector<Tensor*> in; 
   std::vector<Tensor*> i1xn; 

  public:
   Parenthesis2tNum(CCR12_Info* info) : z(info) {};
    
   ~Parenthesis2tNum() {};

   virtual void compute_amp(double*,const long,const long,const long,const long,const long,const long,const long) {};
   virtual void compute_amp(double*,const long,const long,const long,const long,const long,const long,const long,const long,const long) {};

};

}

#endif


