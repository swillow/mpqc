//
// df_runtime.h
//
// Copyright (C) 2009 Edward Valeev
//
// Author: Edward Valeev <evaleev@vt.edu>
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

#ifndef _mpqc_src_lib_chemistry_qc_df_df_runtime_h
#define _mpqc_src_lib_chemistry_qc_df_df_runtime_h

#include <chemistry/qc/lcao/df.h>
#include <chemistry/qc/lcao/tbint_runtime.h>
#include <Eigen/Dense>
#include <util/misc/sharedptr.h>


namespace sc {

  /** Parsed representation of a string key that represents fitting of a product of space1 and space2 into fspace.
      kernel must be parsable by ParsedTwoBodyOperSetKey.

      The key format is (s1 s2|L(K)|f), where:
      <ul>
        <li> s1 = space1 label </li>
        <li> s2 = space2 label </li>
        <li> L = DF or RI, the latter returns coefficients scaled by inverse square root of the fitting kernel;
        if the fitting kernel is negative definite, will use the inverse square root of the negative. </li>
        <li> K = fitting kernel label </li>
        <li> f = fspace label </li>
       </ul>
      */
  class ParsedDensityFittingKey {
    public:
      ParsedDensityFittingKey(const std::string& key);

      const std::string& key() const { return key_; }
      const std::string& space1() const { return space1_; }
      const std::string& space2() const { return space2_; }
      const std::string& fspace() const { return fspace_; }
      const std::string& kernel() const { return kernel_pkey_.key(); }
      const std::string& kernel_oper() const { return kernel_pkey_.oper(); }
      const std::string& kernel_param() const { return kernel_pkey_.params(); }
      bool ri() const { return ri_; }

      /// computes key from its components
      static std::string key(const std::string& space1,
                             const std::string& space2,
                             const std::string& fspace,
                             const std::string& kernel,
                             bool ri = false);

    private:
      std::string key_;
      std::string space1_, space2_, fspace_;
      ParsedTwoBodyOperSetKey kernel_pkey_;
      bool ri_;
  };

  class DensityFittingParams;

  /**
   *    Smart runtime support for managing DensityFitting objects
   */
  class DensityFittingRuntime : virtual public SavableState {
    public:
      typedef DensityFittingRuntime this_type;
      typedef DistArray4 Result;
      typedef Ref<Result> ResultRef;
      typedef ParsedDensityFittingKey ParsedResultKey;
      typedef DensityFitting::MOIntsRuntime MOIntsRuntime;
#ifdef MPQC_NEW_FEATURES
      typedef Eigen::VectorXd CoefResult;
      typedef std::shared_ptr<Eigen::VectorXd> CoefResultRef;
      typedef std::pair<int, int> IntPair;
      typedef std::pair<std::string, IntPair> CoefKey;
#endif // MPQC_NEW_FEATURES

      // uses MOIntsRuntime to evaluate integrals
      DensityFittingRuntime(const Ref<MOIntsRuntime>& moints_runtime,
                            const DensityFittingParams* dfparams);
      DensityFittingRuntime(StateIn& si);
      void save_data_state(StateOut& so);

      /// obsoletes this object
      void obsolete();

      /// which density fitting solver will be used to compute?
      const DensityFittingParams* dfparams() const { return dfparams_; }

      /** Returns true if the given DensityFitting is available
        */
      bool exists(const std::string& key) const;

#ifdef MPQC_NEW_FEATURES
      /** Returns true if the given coefficient block is available
        */
      bool exists(const CoefKey& key) const;
#endif // MPQC_NEW_FEATURES

      /** Returns the DistArray4 object corresponding to this key.

          key must be in format recognized by ParsedDensityFittingKey.
          If this key is not known, the DistArray4 object will be created
          and (possibly) computed.
        */
      ResultRef get(const std::string& key);   // non-const: can compute something

#ifdef MPQC_NEW_FEATURES
      /** Returns the Eigen::MatrixXd (a.k.a. CoefResultRef) object corresponding
       *  to the CoefKey key given.
       */
      const CoefResultRef get(const CoefKey& key);
      const CoefResultRef get(const std::string& dfkey, int bf1, int bf2){ return get(CoefKey(dfkey, IntPair(bf1, bf2))); }
#endif // MPQC_NEW_FEATURES

      /// returns the runtime used to compute results
      const Ref<MOIntsRuntime>& moints_runtime() const { return moints_runtime_; }

      /// removes all entries that contain this space
      void remove_if(const std::string& space_key);

      /**
       * tries to translate a library basis set label to the corresponding default value for the DF basis
       * @param obs_name orbital basis set name; to be useful must be a canonical library name
       * @param incX optional parameter to raise the cardinal number of the density fitting basis. This may be useful if using density-fitting
       *             slightly outside their intended area of application.
       * @param force_aug optional parameter similar to incX to force inclusion of diffuse components of the density fitting basis
       * @return canonical library name of the default density-fitting basis set; if not able to suggest the default basis, returns an empty string
       */
      static std::string default_dfbs_name(const std::string& obs_name,
                                           int incX = 0,
                                           bool force_aug = false);

    private:
      Ref<MOIntsRuntime> moints_runtime_;
      const DensityFittingParams* dfparams_;

      typedef Registry<std::string, ResultRef, detail::NonsingletonCreationPolicy > ResultRegistry;
      Ref<ResultRegistry> results_;

#ifdef MPQC_NEW_FEATURES
      typedef Registry<CoefKey, CoefResultRef, detail::NonsingletonCreationPolicy > CoefRegistry;
      Ref<CoefRegistry> coef_results_;

      CoefResultRef get_coefficients(const CoefKey& key);

      typedef Eigen::HouseholderQR<Eigen::MatrixXd> Decomposition;
      typedef std::map<IntPair, std::shared_ptr<Decomposition> > DecompositionMap;
      DecompositionMap decomps_;
#endif // MPQC_NEW_FEATURES

      // creates the result for a given key
      const ResultRef& create_result(const std::string& key);

      static ClassDesc class_desc_;

  };

  /** DensityFittingParams defines parameters used by DensityFittingRuntime and other runtime components
      to compute density fitting objects.
    */
  class DensityFittingParams : virtual public SavableState {
    public:
    /**
     * @param basis  The GaussianBasisSet object used to fit product densities. There is no default.
     *               @note DensityFittingRuntime does not use this, but other runtime objects may use it
     *               to set the global density fitting basis.
     * @param kernel_key A string describing the kernel_key. It must be parsable by ParsedTwoBodyOperSetKey, or be empty (the default).
     *               @note DensityFittingRuntime does not use this, but other runtime objects may use it to set the global density fitting method.
     * @param solver A string describing the method of solving the density fitting equations. This is used by DensityFittingRuntime
     *               to produce density fitting objects.
     */
      DensityFittingParams(const Ref<GaussianBasisSet>& basis,
                           const std::string& kernel = std::string(),
                           const std::string& solver = std::string("cholesky_inv"));
      DensityFittingParams(StateIn&);
      ~DensityFittingParams();
      void save_data_state(StateOut&);

      const Ref<GaussianBasisSet>& basis() const { return basis_; }
      const std::string& kernel_key() const { return kernel_; }
      DensityFitting::SolveMethod solver() const { return solver_; }
      bool local_coulomb() const { return local_coulomb_; }
      void local_coulomb(bool val) { local_coulomb_ = val; }
      bool local_exchange() const { return local_exchange_; }
      void local_exchange(bool val) { local_exchange_ = val; }
      bool exact_diag_J() const { return exact_diag_J_; }
      void exact_diag_J(bool val) { exact_diag_J_ = val; }
      bool exact_diag_K() const { return exact_diag_K_; }
      void exact_diag_K(bool val) { exact_diag_K_ = val; }
      /// returns the TwoBodyInt::oper_type object that specifies
      /// the type of the operator kernel_key used for fitting the density
      TwoBodyOper::type kernel_otype() const;
      /// returns the IntParams object corresponding to the fitting kernel_key
      std::string intparams_key() const;

      void print(std::ostream& o) const;
    private:
      static ClassDesc class_desc_;

      bool local_coulomb_;
      bool local_exchange_;
      bool exact_diag_J_;
      bool exact_diag_K_;
      Ref<GaussianBasisSet> basis_;
      std::string kernel_;
      DensityFitting::SolveMethod solver_;
  };

   inline bool operator==(const DensityFittingParams& A, const DensityFittingParams& B) {
    return A.basis()->equiv(B.basis()) && A.kernel_key() == B.kernel_key() && A.solver() == B.solver();
  }

  /// this class encapsulates objects needed to perform density fitting of a 4-center integral
  struct DensityFittingInfo : virtual public SavableState {
    public:
      typedef DensityFittingInfo this_type;

      DensityFittingInfo(const Ref<DensityFittingParams>& p,
                         const Ref<DensityFittingRuntime>& r) :
                           params_(p), runtime_(r) {}
      DensityFittingInfo(StateIn& si) {
        params_ << SavableState::restore_state(si);
        runtime_ << SavableState::restore_state(si);
      }
      void save_data_state(StateOut& so) {
        SavableState::save_state(params_.pointer(),so);
        SavableState::save_state(runtime_.pointer(),so);
      }

      const Ref<DensityFittingParams>& params() const { return params_; }
      const Ref<DensityFittingRuntime>& runtime() const { return runtime_; }

      /// obsoletes all dependent runtimes
      void obsolete();

    private:
      Ref<DensityFittingParams> params_;
      Ref<DensityFittingRuntime> runtime_;

      static ClassDesc class_desc_;
  };

  inline bool operator==(const DensityFittingInfo& A, const DensityFittingInfo& B) {
    // dfinfo objects are equivalent if they use equivalent params and same runtime object
    return (*A.params() == *B.params()) && A.runtime() == B.runtime();
  }


} // end of namespace sc

#endif // end of header guard


// Local Variables:
// mode: c++
// c-file-style: "CLJ-CONDENSED"
// End:
