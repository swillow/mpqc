//
// int1e.cc
//
// Copyright (C) 2004 Sandia National Laboratories.
//
// Author: Joseph Kenny <jpkenny@sandia.gov>
// Maintainer: Joseph Kenny
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

#ifdef __GNUG__
#pragma implementation
#endif

#include <chemistry/qc/intcca/int1e.h>

using namespace std;
using namespace sc;
using namespace Chemistry::QC::GaussianBasis;

Int1eCCA::Int1eCCA(Integral *integral,
		   const Ref<GaussianBasisSet>&b1,
		   const Ref<GaussianBasisSet>&b2,
		   int order, IntegralEvaluatorFactory eval_factory, 
                   std::string int_type, bool use_opaque):
  bs1_(b1), bs2_(b2),
  overlap_ptr_(0), kinetic_ptr_(0),
  nuclear_ptr_(0), hcore_ptr_(0),
  integral_(integral), eval_factory_(eval_factory), use_opaque_(use_opaque)
{

  int scratchsize=0,nshell2;
  
  /* The efield routines look like derivatives so bump up order if
   * it is zero to allow efield integrals to be computed.
   */
  if (order == 0) order = 1;

  nshell2 = bs1_->max_ncartesian_in_shell()*bs2_->max_ncartesian_in_shell();

  if (order == 0) 
    scratchsize = nshell2;
  else if (order == 1) 
    scratchsize = nshell2*3;
  else {
    ExEnv::errn() << 
      scprintf("Int1eCCA constructor: invalid order: %d\n",order);
    exit(1);
  }

  if( !use_opaque_ ) {
    buff_ = new double[scratchsize];
  }

  cca_bs1_ = GaussianBasis_Molecular::_create();
  cca_bs1_.initialize( bs1_.pointer(), bs1_->name() );
  if( bs1_.pointer() != bs2_.pointer() ) {
    cca_bs2_ = GaussianBasis_Molecular::_create();
    cca_bs2_.initialize( bs2_.pointer(), bs2_->name() );
  }
  else
    cca_bs2_ = cca_bs1_;

  if( int_type == "overlap" ) {
    overlap_ = eval_factory_.get_integral_evaluator2( "overlap", 0, 
                                                      cca_bs1_, cca_bs2_ );
    overlap_ptr_ = &overlap_;
    if( use_opaque_ ) {
      try{ buff_ = static_cast<double*>( overlap_ptr_->get_buffer() ); }
      catch(exception &e) { e.what(); abort(); }
    }
  }

  else if( int_type == "kinetic" ) {
    kinetic_ = eval_factory_.get_integral_evaluator2( "kinetic", 0,
                                                      cca_bs1_, cca_bs2_ );
    kinetic_ptr_ = &kinetic_;
    if( use_opaque_ ) {
      try{ buff_ = static_cast<double*>( kinetic_ptr_->get_buffer() ); }
      catch(exception &e) { e.what(); abort(); }
    }
  }

  if( int_type == "nuclear" ) {
    nuclear_ = eval_factory_.get_integral_evaluator2( "potential", 0,
                                                      cca_bs1_, cca_bs2_ );
    nuclear_ptr_ = &nuclear_;
    if( use_opaque_ ) {
      try{ buff_ = static_cast<double*>( nuclear_ptr_->get_buffer() ); }
      catch(exception &e) { e.what(); abort(); }
    }
  }

  if( int_type == "hcore" ) {
    hcore_ = eval_factory_.get_integral_evaluator2( "1eham", 0,
                                                    cca_bs1_, cca_bs2_ );
    hcore_ptr_ = &hcore_;
    if( use_opaque_ ) {
      try{ buff_ = static_cast<double*>( hcore_ptr_->get_buffer() ); }
      catch(exception &e) { e.what(); abort(); }
    }
  }

}

Int1eCCA::~Int1eCCA()
{
  // transform_done();
  // int_done_1e();
  // int_done_offsets1();
}

void
Int1eCCA::overlap( int ish, int jsh )
{
  if( use_opaque_ ) {
    overlap_ptr_->compute( ish, jsh, 0 );
  }
  else {
    sidl_buffer_ = overlap_ptr_->compute_array( ish, jsh, 0 ); 
    copy_buffer();
  }
}  

void
Int1eCCA::kinetic( int ish, int jsh )
{
  if( use_opaque_ )
    kinetic_ptr_->compute( ish, jsh, 0 );
  else {
    sidl_buffer_ = kinetic_ptr_->compute_array( ish, jsh, 0 ); 
    copy_buffer();
  }
}

void
Int1eCCA::nuclear( int ish, int jsh )
{
  if( use_opaque_ )
    nuclear_ptr_->compute( ish, jsh, 0 );
  else {
    sidl_buffer_ = nuclear_ptr_->compute_array( ish, jsh, 0 ); 
    copy_buffer();
  }
}

void
Int1eCCA::hcore( int ish, int jsh )
{
  if( use_opaque_ )
    hcore_ptr_->compute( ish, jsh, 0 );
  else {
    sidl_buffer_ = hcore_ptr_->compute_array( ish, jsh, 0 ); 
    copy_buffer();
  }
}

void 
Int1eCCA::copy_buffer() 
{
  int sidl_size = 1 + sidl_buffer_.upper(0) - sidl_buffer_.lower(0);
  for(int i=0; i<sidl_size; ++i) 
    buff_[i] = sidl_buffer_.get(i);
}


/////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: c++
// End:
