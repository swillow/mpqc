//
// ri_basis.cc
//
// Copyright (C) 2004 Edward Valeev
//
// Author: Edward Valeev <edward.valeev@chemistry.gatech.edu>
// Maintainer: EV
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

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdexcept>
#include <sstream>

#include <util/misc/formio.h>
#include <util/misc/exenv.h>
#include <chemistry/qc/basis/basis.h>
#include <chemistry/qc/basis/symmint.h>
#include <chemistry/qc/mbptr12/linearr12.h>
#include <chemistry/qc/mbptr12/vxb_eval_info.h>

using namespace sc;
using namespace std;

void
R12IntEvalInfo::construct_ri_basis_(bool safe)
{
  if (bs_aux_->equiv(bs_)) {
    bs_ri_ = bs_;
    if (abs_method_ == LinearR12::ABS_EV ||
	abs_method_ == LinearR12::ABS_EVPlus)
      throw std::runtime_error("R12IntEvalInfo::construct_ri_basis_ -- ABS methods EV and EV+ can only be used when ABS != OBS");
  }
  else {
    switch(abs_method_) {
      case LinearR12::ABS_KS:
	construct_ri_basis_ks_(safe);
	break;
      case LinearR12::ABS_KSPlus:
	construct_ri_basis_ksplus_(safe);
	break;
      case LinearR12::ABS_EV:
	construct_ri_basis_ev_(safe);
	break;
      case LinearR12::ABS_EVPlus:
	construct_ri_basis_evplus_(safe);
	break;
      default:
	throw std::runtime_error("R12IntEvalInfo::construct_ri_basis_ -- invalid ABS method");
    }
  }
}

void
R12IntEvalInfo::construct_ri_basis_ks_(bool safe)
{
  bs_ri_ = bs_aux_;
  if (!abs_spans_obs_()) {
    ExEnv::out0() << endl << indent << "WARNING: the auxiliary basis is not safe to use with the given orbital basis" << endl << endl;
    if (safe)
      throw std::runtime_error("R12IntEvalInfo::construct_ri_basis_ks_ -- auxiliary basis is not safe to use with the given orbital basis");
  }
}

void
R12IntEvalInfo::construct_ri_basis_ksplus_(bool safe)
{
  GaussianBasisSet& abs = *(bs_aux_.pointer());
  bs_ri_ = abs + bs_;
}

void
R12IntEvalInfo::construct_ri_basis_ev_(bool safe)
{
  bs_ri_ = bs_aux_;
  if (!abs_spans_obs_()) {
    ExEnv::out0() << endl << indent << "WARNING: the auxiliary basis is not safe to use with the given orbital basis" << endl << endl;
    if (safe)
      throw std::runtime_error("R12IntEvalInfo::construct_ri_basis_ev_ -- auxiliary basis is not safe to use with the given orbital basis");
  }
  construct_ortho_comp_();
}

void
R12IntEvalInfo::construct_ri_basis_evplus_(bool safe)
{
  GaussianBasisSet& abs = *(bs_aux_.pointer());
  bs_ri_ = abs + bs_;
  construct_ortho_comp_();
}

void
R12IntEvalInfo::construct_orthog_aux_()
{
  if (orthog_aux_.nonnull())
    return;

  // Make an Integral and initialize with bs_aux
  Ref<Integral> integral_aux = integral_->clone();

  //
  // Compute ABS-ABS overlap matrix
  //
  integral_aux->set_basis(bs_aux_);
  Ref<PetiteList> plist2 = integral_aux->petite_list();
  Ref<OneBodyInt> ov_22_engine = integral_aux->overlap();

  // form skeleton s matrix
  Ref<SCMatrixKit> matrixkit = bs_aux_->matrixkit();
  RefSymmSCMatrix s(bs_aux_->basisdim(), matrixkit);
  Ref<SCElementOp> ov =
    new OneBodyIntOp(new SymmOneBodyIntIter(ov_22_engine, plist2));

  s.assign(0.0);
  s.element_op(ov);
  ov=0;
  if (debug_ > 1) s.print("AO skeleton overlap (ABS/ABS)");

  // then symmetrize it
  RefSCDimension sodim_aux = plist2->SO_basisdim();
  Ref<SCMatrixKit> so_matrixkit = bs_aux_->so_matrixkit();
  RefSymmSCMatrix overlap_aux(sodim_aux, so_matrixkit);
  plist2->symmetrize(s,overlap_aux);

  // and clean up a bit
  ov_22_engine = 0;
  s = 0;

  //
  // Compute orthogonalizer for the auxiliary basis
  //
  ExEnv::out0() << indent << "Orthogonalizing ABS:" << endl << incindent;
  OverlapOrthog orthog(ref_->orthog_method(),
                       overlap_aux,
                       so_matrixkit,
                       ref_->lindep_tol(),
                       0);
  nlindep_aux_ = orthog.nlindep();
  RefSCMatrix orthog_aux_so = orthog.basis_to_orthog_basis();
  orthog_aux_so = orthog_aux_so.t();
  orthog_aux_ = plist2->evecs_to_AO_basis(orthog_aux_so);
  orthog_aux_so = 0;
  overlap_aux_ = overlap_aux;
  ExEnv::out0() << decindent;

  if (bs_aux_ == bs_ri_) {
    orthog_ri_ = orthog_aux_;
    nlindep_ri_ = nlindep_aux_;
    overlap_ri_ = overlap_aux_;
  }
}

void
R12IntEvalInfo::construct_orthog_ri_()
{
  if (bs_ri_.null())
    throw std::runtime_error("R12IntEvalInfo::construct_orthog_ri_ -- RI basis has not been set yet");
  if (bs_aux_ == bs_ri_)
    construct_orthog_aux_();
  if (orthog_ri_.nonnull())
    return;

  // Make an Integral and initialize with bs_aux
  Ref<Integral> integral_aux = integral_->clone();

  //
  // Compute RIBS-RIBS overlap matrix
  //
  integral_aux->set_basis(bs_ri_);
  Ref<PetiteList> plist3 = integral_aux->petite_list();
  Ref<OneBodyInt> ov_33_engine = integral_aux->overlap();

  // form skeleton s matrix
  Ref<SCMatrixKit> matrixkit = bs_ri_->matrixkit();
  Ref<SCMatrixKit> so_matrixkit = bs_ri_->so_matrixkit();
  RefSymmSCMatrix s3(bs_ri_->basisdim(), matrixkit);
  Ref<SCElementOp> ov3 =
    new OneBodyIntOp(new SymmOneBodyIntIter(ov_33_engine, plist3));

  s3.assign(0.0);
  s3.element_op(ov3);
  ov3=0;
  if (debug_ > 1) s3.print("AO skeleton overlap (ABS+OBS/ABS+OBS)");

  // then symmetrize it
  RefSCDimension sodim_ri = plist3->SO_basisdim();
  RefSymmSCMatrix overlap_ri(sodim_ri, so_matrixkit);
  plist3->symmetrize(s3,overlap_ri);

  // and clean up a bit
  ov_33_engine = 0;
  s3 = 0;

  //
  // Compute orthogonalizer for the auxiliary basis
  //
  ExEnv::out0() << indent << "Orthogonalizing RI-BS:" << endl << incindent;
  OverlapOrthog orthog(ref_->orthog_method(),
		       overlap_ri,
		       so_matrixkit,
		       ref_->lindep_tol(),
		       0);
  nlindep_ri_ = orthog.nlindep();
  RefSCMatrix orthog_ri_so = orthog.basis_to_orthog_basis();
  orthog_ri_so = orthog_ri_so.t();
  orthog_ri_ = plist3->evecs_to_AO_basis(orthog_ri_so);
  orthog_ri_so = 0;
  overlap_ri_ = overlap_ri;
  ExEnv::out0() << decindent;
}

bool
R12IntEvalInfo::abs_spans_obs_()
{
  construct_orthog_aux_();

  // Make an Integral and initialize with bs_aux
  Ref<Integral> integral_aux = integral_->clone();

  //
  // Compute RIBS-RIBS overlap matrix
  //
  GaussianBasisSet& abs = *(bs_aux_.pointer());
  Ref<GaussianBasisSet> ri_basis = abs + bs_;
  int nlindep_ri = 0;
  if (bs_ri_.nonnull() && ri_basis->equiv(bs_ri_)) {
    construct_orthog_ri_();
    nlindep_ri = nlindep_ri_;
  }
  else {
    integral_aux->set_basis(ri_basis);
    Ref<PetiteList> plist3 = integral_aux->petite_list();
    Ref<OneBodyInt> ov_33_engine = integral_aux->overlap();
    
    // form skeleton s matrix
    Ref<SCMatrixKit> matrixkit = ri_basis->matrixkit();
    Ref<SCMatrixKit> so_matrixkit = ri_basis->so_matrixkit();
    RefSymmSCMatrix s3(ri_basis->basisdim(), matrixkit);
    Ref<SCElementOp> ov3 =
      new OneBodyIntOp(new SymmOneBodyIntIter(ov_33_engine, plist3));
    
    s3.assign(0.0);
    s3.element_op(ov3);
    ov3=0;
    if (debug_ > 1) s3.print("AO skeleton overlap (ABS+OBS/ABS+OBS)");
    
    // then symmetrize it
    RefSCDimension sodim_ri = plist3->SO_basisdim();
    RefSymmSCMatrix overlap_ri(sodim_ri, so_matrixkit);
    plist3->symmetrize(s3,overlap_ri);
    
    // and clean up a bit
    ov_33_engine = 0;
    s3 = 0;
    
    //
    // Compute orthogonalizer for the auxiliary basis
    //
    ExEnv::out0() << indent << "Orthogonalizing ABS+OBS:" << endl << incindent;
    OverlapOrthog orthog(ref_->orthog_method(),
			 overlap_ri,
			 so_matrixkit,
			 ref_->lindep_tol(),
			 0);
    nlindep_ri = orthog.nlindep();
    ExEnv::out0() << decindent;

    ri_basis = 0;
  }

  if (nlindep_ri - nlindep_aux_ - noso_ == 0)
    return true;
  else
    return false;

}

/////////////////////////////////////////////

void
R12IntEvalInfo::construct_ortho_comp_()
{
   construct_orthog_ri_();

   ExEnv::out0() << indent << "Projecting out OBS and final RI-BS orthonormalization:" << endl << incindent;

   // Make an Integral and initialize with bs_aux
   Ref<Integral> integral_aux = integral()->clone();

   //
   // Compute OBS/RI-BS overlap matrix
   //
   Ref<GaussianBasisSet> obs = bs_;
   Ref<GaussianBasisSet> ri_bs = bs_ri_;
   Ref<SCMatrixKit> ao_matrixkit = bs_ri_->matrixkit();
   Ref<SCMatrixKit> so_matrixkit = bs_ri_->so_matrixkit();
   integral_aux->set_basis(obs,ri_bs);
   Ref<OneBodyInt> ov_12_engine = integral_aux->overlap();
   
   // form AO s matrix
   RefSCMatrix s12(obs->basisdim(), ri_bs->basisdim(), ao_matrixkit);
   Ref<SCElementOp> ov_op12 =
     new OneBodyIntOp(new OneBodyIntIter(ov_12_engine));
    
   s12.assign(0.0);
   s12.element_op(ov_op12);
   ov_op12=0;
   if (debug_ > 1) s12.print("AO overlap (OBS/RI-BS)");

   // get MOs
   RefSCMatrix scf_vec = ref_->eigenvectors();
   RefSCMatrix so_ao = integral_aux->petite_list()->sotoao();
   scf_vec = scf_vec.t() * so_ao;
    
   //
   // Compute orthogonalizer for the RI basis
   //
    
   // Convert blocked basisdim into a nonblocked basisdim
   integral_aux->set_basis(ri_bs);
   Ref<PetiteList> plist_ri = integral_aux->petite_list();
   RefSCMatrix ao2so_ri = plist_ri->aotoso();
   RefSCMatrix s12_nb(scf_vec.coldim(),ao2so_ri->rowdim(), so_matrixkit);
   s12_nb->convert(s12);
   s12 = 0;

   // MOs (in terms of AOs) "transformed" into the RI AO basis
   RefSCMatrix scf_vec_ri_bm = scf_vec * s12_nb * orthog_ri_ * orthog_ri_.t();  

   // Transform MOs into the auxiliary SO basis
   RefSCMatrix scf_vec_ri = scf_vec_ri_bm * ao2so_ri;

   // Orthogonalize MOs expressed in the auxiliary SO basis
   RefSymmSCMatrix CtSC(scf_vec_ri.rowdim(), so_matrixkit);
   CtSC.assign(0.0);
   CtSC.accumulate_transform(scf_vec_ri, overlap_ri_);
   OverlapOrthog Corthog(ref_->orthog_method(),
                         CtSC,
                         so_matrixkit,
                         ref_->lindep_tol(),
                         0);
   scf_vec_ri = Corthog.basis_to_orthog_basis() * scf_vec_ri;
   ExEnv::out0() << endl;

   // Prepare the initial set of vectors in RI-BS
   int ng = orthog_ri_.coldim()->blocks()->nblock();
   RefSCDimension sodim_ri = ao2so_ri.coldim();
   RefSCMatrix oso_ri(orthog_ri_.coldim(), sodim_ri, so_matrixkit);
   oso_ri.assign(orthog_ri_.t()*ao2so_ri);
   ao2so_ri = 0;

   // Project out MOs from the RI-BS vectors (a la Gram-Schmidt)
   for(int g=0; g<ng; g++) {
      
     RefSCMatrix scfvec_block = scf_vec_ri.block(g);
     RefSCMatrix oso_ri_block = oso_ri.block(g);
     RefSymmSCMatrix overlap_ri_block = overlap_ri_.block(g);

     int nso_ri = oso_ri_block.rowdim()->n();
     for(int so=0; so<nso_ri; so++) {
	
       // Get the vector -- don't need to normalize here because oso_ri are already orthonormal
       RefSCVector tmpvec_block = oso_ri_block.get_row(so);
       
       // Project against MOs
       int noso = scfvec_block.rowdim().n();
       for(int mo=0; mo<noso; mo++) {
	 RefSCVector movec = scfvec_block.get_row(mo);
	 double overlap = tmpvec_block.scalar_product(overlap_ri_block * movec);
	 tmpvec_block.accumulate(-overlap*movec);
       }
       
       oso_ri_block.assign_row(tmpvec_block,so);
       
     }
   }
    
   //
   // SVD-orthonormalize RI-BS vectors from which OBS has already been projected out
   //
   RefSymmSCMatrix UtSU(oso_ri.rowdim(), so_matrixkit);
   UtSU.assign(0.0);
   UtSU.accumulate_transform(oso_ri, overlap_ri_);
   OverlapOrthog svdorthog(ref_->orthog_method(),
			   UtSU,
			   so_matrixkit,
			   ref_->lindep_tol(),
			   0);
   
   oso_ri = svdorthog.basis_to_orthog_basis() * oso_ri;
   oso_ri = oso_ri.t();
   RefSCDimension osodim_ri = oso_ri.coldim();
   orthog_ri_ = plist_ri->evecs_to_AO_basis(oso_ri);
   oso_ri = 0;
   if (osodim_ri.n() == 0)
     throw std::runtime_error("Number of linearly independent functions in RI-BS orthogonal to OBS is zero. Modify/increase your auxiliary basis.");

   if (debug_ > 1) (orthog_ri_.t() * plist_ri->to_AO_basis(overlap_ri_) * orthog_ri_).print("Ut * S(RI-BS/RI-BS) * U");

}

/////////////////////////////////////////////////////////////////////////////

// Local Variables:
// mode: c++
// c-file-style: "CLJ-CONDENSED"
// End:
