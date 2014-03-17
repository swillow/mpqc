//
// compute_k.cc
//
// Copyright (C) 2014 David Hollman
//
// Author: David Hollman
// Maintainer: DSH
// Created: Feb 13, 2014
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

#include <chemistry/qc/basis/petite.h>
#include <util/misc/xmlwriter.h>

#include "cadfclhf.h"

using namespace sc;
using std::cout;
using std::endl;



double CADFCLHF::get_distance_factor(
    const ShellData& ish, const ShellData& jsh, const ShellData& Xsh
) const
{
  double dist_factor;
  if(linK_use_distance_){
    const double R = get_R(ish, jsh, Xsh);
    const double l_X = double(dfbs_->shell(Xsh).am(0));
    double r_expon = l_X + 1.0;
    if(ish.center == jsh.center) {
      if(ish.center == Xsh.center) {
        r_expon = 0.0;
      }
      else {
        const double l_i = double(gbs_->shell(ish).am(0));
        const double l_j = double(gbs_->shell(jsh).am(0));
        r_expon += abs(l_i-l_j);
      }
    }
    dist_factor = 1.0 / (pow(R, pow(r_expon, distance_damping_factor_)));            //latex `\label{sc:link:dist_damp}`
    // If the distance factor actually makes the bound larger, then ignore it.
    dist_factor = std::min(1.0, dist_factor);
  }
  else {
    dist_factor = 1.0;
  }
  return dist_factor;
};


double CADFCLHF::get_R(
    const ShellData& ish,
    const ShellData& jsh,
    const ShellData& Xsh
) const
{
  double rv = (pair_centers_.at({(int)ish, (int)jsh}) - centers_[Xsh.center]).norm();
  if(use_extents_) {
    const double ext_a = pair_extents_.at({(int)ish, (int)jsh});
    const double ext_b = df_extents_[(int)Xsh];
    if(subtract_extents_) {
      rv -= ext_a + ext_b;
    }
    else if(ext_a + ext_b >= rv){
      rv = 1.0; // Don't do distance screening
    }
  }
  if(rv < 1.0) {
    rv = 1.0;
  }
  return rv;
};

typedef std::pair<int, int> IntPair;

RefSCMatrix
CADFCLHF::compute_K()
{
  /*=======================================================================================*/
  /* Setup                                                 		                        {{{1 */ #if 1 // begin fold
  //----------------------------------------//
  // Convenience variables
  Timer timer("compute K");
  const int me = scf_grp_->me();
  const int n_node = scf_grp_->n();
  const Ref<GaussianBasisSet>& obs = gbs_;
  const int nbf = obs->nbasis();
  const int dfnbf = dfbs_->nbasis();

  //----------------------------------------//
  // Get the density in an Eigen::Map form
  double *D_ptr = allocate<double>(nbf*nbf);
  D_.convert(D_ptr);
  typedef Eigen::Map<Eigen::VectorXd> VectorMap;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMatrix;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor> ColMatrix;
  //typedef Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> MatrixMap;
  typedef Eigen::Map<ColMatrix> MatrixMap;
  // Matrix and vector wrappers, for convenience
  VectorMap d(D_ptr, nbf*nbf);
  MatrixMap D(D_ptr, nbf, nbf);
  // Match density scaling in old code:
  D *= 0.5;
  //----------------------------------------//
  Eigen::MatrixXd Kt(nbf, nbf);
  Kt = Eigen::MatrixXd::Zero(nbf, nbf);
  //----------------------------------------//
  // Compute all of the integrals outside of
  //   the thread loop.  This will need to be
  //   rethought in HUGE cases, but for now
  //   it is not a limiting factor
  timer.enter("compute (X|Y)");
  auto g2_full_ptr = ints_to_eigen_threaded(
      ShellBlockData<>(dfbs_),
      ShellBlockData<>(dfbs_),
      eris_2c_, coulomb_oper_type_
  );
  const auto& g2 = *g2_full_ptr;
  timer.exit();
  //----------------------------------------//
  // reset the iteration over local pairs
  local_pairs_spot_ = 0;
  //----------------------------------------//
  auto get_ish_Xblk_pair = [&](ShellData& ish, ShellBlockData<>& Xblk, PairSet ps) -> bool {
    int spot = local_pairs_spot_++;
    if(spot < local_pairs_k_[ps].size()){
      auto& sig_pair = local_pairs_k_[ps][spot];
      ish = ShellData(sig_pair.first, gbs_, dfbs_);
      Xblk = sig_pair.second;
      return true;
    }
    else{
      return false;
    }
  };
  //----------------------------------------//
  /*****************************************************************************************/ #endif //1}}}
  /*=======================================================================================*/
  /* Make the CADF-LinK lists                                                         {{{1 */ #if 1 //latex `\label{sc:link}`

  std::vector<std::tuple<int, int, int>> L_3_keys;

  // Now make the linK lists if we're doing linK
  if(do_linK_){
    timer.enter("LinK lists");
    // First clear all of the lists
    L_D.clear();
    L_DC.clear();
    L_3.clear();
    //============================================================================//
    Eigen::MatrixXd D_frob(obs->nshell(), obs->nshell());
    do_threaded(nthread_, [&](int ithr){
      for(int lsh_index = ithr; lsh_index < obs->nshell(); lsh_index += nthread_) {           //latex `\label{sc:link:ld}`
        ShellData lsh(lsh_index, obs, dfbs_);
        for(auto&& jsh : shell_range(obs)) {
          double dnorm = D.block(lsh.bfoff, jsh.bfoff, lsh.nbf, jsh.nbf).norm();
          D_frob(lsh, jsh) = dnorm;
          L_D[lsh].insert(jsh, dnorm);
        }
        L_D[lsh].sort();
      }
    });
    // TODO Distribute over MPI processes as well
    //----------------------------------------//                                             //latex `\label{sc:link:setupend}`
    // Form L_DC

    timer.enter("build L_DC");
    for(auto&& jsh : shell_range(obs)) {                                                       //latex `\label{sc:link:ldc}`

      const auto& Drho = D_frob.row(jsh);

      for(auto&& Xsh : shell_range(dfbs_)) {
        auto& Cmaxes_X = Cmaxes_[Xsh];
        // TODO Optimize this to not be N^3

        double max_val = 0.0;

        if(use_norms_sigma_) {
          for(auto&& lsh : shell_range(obs)) {
            //const double this_val = Drho(lsh) * Cmaxes_X[lsh].value;
            //max_val += this_val * this_val;
            max_val += Drho(lsh) * Cmaxes_X[lsh].value;
          } // end loop over lsh
          //max_val = sqrt(max_val);
        }
        else {
          for(auto&& lsh : shell_range(obs)) {
            const double this_val = Drho(lsh) * Cmaxes_X[lsh].value;
            if(this_val > max_val) max_val = this_val;
          } // end loop over lsh
        }
        L_DC[jsh].insert(Xsh,                                                                //latex `\label{sc:link:ldcstore}`
            max_val * schwarz_df_[Xsh]
        );
      } // end loop over Xsh

      L_DC[jsh].sort();

    } // end loop over jsh

    //----------------------------------------//                                             //latex `\label{sc:link:ldc:end}`
    // Form L_3
    timer.change("build L_3");
    double epsilon, epsilon_dist;                                                            //latex `\label{sc:link:l3}`

    if(density_reset_){
      epsilon = full_screening_thresh_;
      epsilon_dist = distance_screening_thresh_;
    }
    else{
      epsilon = pow(full_screening_thresh_, full_screening_expon_);                          //latex `\label{sc:link:expon}`
      epsilon_dist = pow(distance_screening_thresh_, full_screening_expon_);
    }

    do_threaded(nthread_, [&](int ithr){

#define NEW_LINK 1
#if NEW_LINK
      int thr_offset = me*nthread_ + ithr;
      int increment = n_node*nthread_;

      auto jsh_iter = shell_range(gbs_, dfbs_).begin();
      const auto& jsh_end = shell_range(gbs_, dfbs_).end();
      auto Xsh_iter = L_DC[*jsh_iter].begin();
      bool Xsh_done = false;
      const auto& advance_iters = [this, &Xsh_done, &jsh_end](
          int amount,
          decltype(jsh_iter)& jsh_it,
          decltype(Xsh_iter)& Xsh_it
      ) -> bool {
        int incr_remain = amount;
        const auto& curr_end = L_DC[*jsh_it].end();
        int nremain = std::distance(Xsh_it, curr_end);
        // This might be inefficient if the shell iterators are not random access?
        if(Xsh_done) {
          if(nremain > amount) {
            // Skip to the last one we would have done
            nremain = nremain % amount;
          }
          // Otherwise, we're moving on already anyway
          Xsh_done = false;
        }
        bool jsh_changed = false;
        while(nremain <= incr_remain) {
          ++jsh_it;
          jsh_changed = true;
          if(jsh_it == jsh_end) break;
          incr_remain -= nremain;
          nremain = L_DC[*jsh_it].size();
        }
        if(jsh_it != jsh_end) {
          if(jsh_changed) Xsh_it = L_DC[*jsh_it].begin();
          std::advance(Xsh_it, incr_remain);
          return true;
        }
        else {
          return false;
        }
      };

      int advance_size = thr_offset;

      while(advance_iters(advance_size, jsh_iter, Xsh_iter)) {
        advance_size = increment;
        const auto& jsh = *jsh_iter;
        const auto& Xsh = *Xsh_iter;
#else
      for(int jsh_index = ithr; jsh_index < obs->nshell(); jsh_index += nthread_) {
        ShellData jsh(jsh_index, gbs_, dfbs_);
        for(auto Xsh : L_DC[jsh]) {
#endif

        auto& L_sch_jsh = L_schwarz[jsh];

        const double pf = Xsh.value;                                                       //latex `\label{sc:link:pf}`
        const double eps_prime = epsilon / pf;
        const double eps_prime_dist = epsilon_dist / pf;
        const double Xsh_schwarz = schwarz_df_[Xsh];
        bool jsh_added = false;
        for(auto ish : L_sch_jsh) {

          double dist_factor = get_distance_factor(ish, jsh, Xsh);
          assert(ish.value == schwarz_frob_(ish, jsh));

          if(ish.value > eps_prime) {
            jsh_added = true;
            if(!linK_use_distance_ or ish.value * dist_factor > eps_prime_dist) {
              L_3[{ish, Xsh}].insert(jsh,
                  ish.value * dist_factor * Xsh_schwarz
              );

              if(print_screening_stats_) {
                ++iter_stats_->K_3c_needed;
                iter_stats_->K_3c_needed_fxn += ish.nbf * jsh.nbf * Xsh.nbf;
              }

            }
            else if(print_screening_stats_ and linK_use_distance_) {
              ++iter_stats_->K_3c_dist_screened;
              iter_stats_->K_3c_dist_screened_fxn += ish.nbf * jsh.nbf * Xsh.nbf;
            }

          }
          else {
            break;
          }

        } // end loop over ish
        if(not jsh_added){
#if NEW_LINK
          Xsh_done = true;
#else
          break;
#endif
        }

      }
#if !NEW_LINK
      }
#endif

    });
    timer.change("distribute L_3");

#if NEW_LINK
    if(n_node > 1) {
      std::vector<std::tuple<int, int, int, int>> my_L_3_keys;
      for(auto&& L_3_list : L_3) {
        my_L_3_keys.emplace_back(
          me,
          L_3_list.first.first,
          L_3_list.first.second,
          L_3_list.second.size()
        );
      }

      // Get the number of L3 lists per node
      int n_l3_per_node[n_node];
      int ones[n_node];
      std::fill_n(ones, n_node, 1*sizeof(int));
      int L3size = L_3.size();
      scf_grp_->raw_collect(&L3size, (const int*)&ones, &n_l3_per_node);
      int total_n_l3 = 0;
      for(int i = 0; i < n_node; total_n_l3 += n_l3_per_node[i++]);

      // Now get all of the (node, ish, Xsh, size) lists
      int l3_idxs_sizes[total_n_l3*4];
      for(int i = 0; i < n_node; ++i) {
        n_l3_per_node[i] = 4*sizeof(int)*n_l3_per_node[i];
      }
      scf_grp_->raw_collect(my_L_3_keys.data(), (const int*)&n_l3_per_node, &l3_idxs_sizes);

      // Extract the size of each list and sum
      std::map<std::pair<int, int>, int> L3_total_sizes;
      std::map<std::pair<int, int>, std::vector<int>> L3_node_sizes;
      for(int i = 0; i < total_n_l3; ++i) {
        const int node_src = l3_idxs_sizes[4*i];
        const int ish = l3_idxs_sizes[4*i+1];
        const int Xsh = l3_idxs_sizes[4*i+2];
        const int njsh = l3_idxs_sizes[4*i+3];
        DUMP4(node_src, ish, Xsh, njsh);
        L3_total_sizes[{ish, Xsh}] += njsh;
        if(L3_node_sizes.find({ish, Xsh}) == L3_node_sizes.end()) {
          L3_node_sizes.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(ish, Xsh),
            std::forward_as_tuple(n_node)
          );
          assert((L3_node_sizes[{ish, Xsh}].size() == n_node));
        }
        out_assert(node_src, <=, n_node);
        L3_node_sizes[{ish, Xsh}][node_src] = njsh * sizeof(ShellIndexWithValue);
      }

      for(auto&& pair : L3_total_sizes) {
        L_3_keys.emplace_back(
            pair.first.first,
            pair.first.second,
            pair.second
        );
      }

      std::sort(L_3_keys.begin(), L_3_keys.end(),
          [](const std::tuple<int, int, int>& a, const std::tuple<int, int, int>& b) {
            int isha, Xsha, sizea, ishb, Xshb, sizeb;
            std::tie(isha, Xsha, sizea) = a;
            std::tie(ishb, Xshb, sizeb) = b;
            if(sizea > sizeb) return true;
            else if(sizea == sizeb) {
              if(isha < ishb) return true;
              else if(isha == ishb) return Xsha < Xshb;
              else return false;
            }
            else return false;
          }
      );

      // TODO reduce latency penalty here
      for(auto&& L_3_key : L_3_keys) {
        int ish, Xsh, size;
        std::tie(ish, Xsh, size) = L_3_key;
        auto& my_L3_part = L_3[{ish, Xsh}];
        //ShellIndexWithValue full_list[size];
        std::vector<ShellIndexWithValue> full_list(size);
        const auto& unsrt = my_L3_part.unsorted_indices();
        const int mysize = L3_node_sizes[{ish, Xsh}][me];
        DUMP2(mysize/sizeof(ShellIndexWithValue), unsrt.size());
        out_assert(mysize/sizeof(ShellIndexWithValue), ==, unsrt.size());
        ExEnv::out0() << "L3_node_sizes[{" << ish << ", " << Xsh << "}] = ";
        for(auto val : L3_node_sizes[{ish, Xsh}]) {
          ExEnv::out0() << val << " ";
        }
        if(mysize > 0) {
          ExEnv::out0() << ": " << full_list[size-1] << " " << unsrt.data()[mysize/sizeof(ShellIndexWithValue)-1] << endl;
        }
        else{
          ExEnv::out0() << ": " << full_list[size-1] << " (zero size)" << endl;
        }
        scf_grp_->raw_collect(
            unsrt.data(),
            (const int*)L3_node_sizes[{ish, Xsh}].data(),
            full_list.data()
        );
        my_L3_part.clear();
        my_L3_part.acquire_and_sort(full_list.data(), size);
        my_L3_part.set_basis(gbs_, dfbs_);
      }
    }
    else {
#endif
      for(auto&& pair : L_3) {
        L_3_keys.emplace_back(
            pair.first.first,
            pair.first.second,
            pair.second.size()
        );
      }
#if NEW_LINK
    }
#endif

    do_threaded(nthread_, [&](int ithr){
      auto L_3_iter = L_3.begin();
      const auto& L_3_end = L_3.end();
      L_3_iter.advance(ithr);
      while(L_3_iter != L_3_end) {
        if(not linK_block_rho_) L_3_iter->second.set_sort_by_value(false);
        L_3_iter->second.sort();
        L_3_iter.advance(nthread_);
      }
    });                                                                                      //latex `\label{sc:link:l3:end}`

    timer.exit("distribute L_3");

    timer.exit("LinK lists");

  } // end if do_linK_

  /*****************************************************************************************/ #endif //1}}} //latex `\label{sc:link:end}`
  /*=======================================================================================*/
  /* Loop over local shell pairs for three body contributions                         {{{1 */ #if 1 //latex `\label{sc:k3b:begin}`

  {

    Timer timer("three body contributions");
    boost::mutex tmp_mutex;
    std::mutex L_3_mutex;
    auto L_3_key_iter = L_3_keys.begin();
    L_3_key_iter += me;                                                                       //latex `\label{sc:k3b:iterset}`
    boost::thread_group compute_threads;
    // reset the iteration over local pairs
    local_pairs_spot_ = 0;
    // Loop over number of threads
    MultiThreadTimer mt_timer("threaded part", nthread_);
    auto get_ish_Xblk_3 = [&](ShellData& ish, ShellBlockData<>& Xblk) -> bool {                                                //latex `\label{sc:k3b:getiX}`
      if(do_linK_) {
        std::lock_guard<std::mutex> lg(L_3_mutex);
        if(L_3_key_iter == L_3_keys.end()) {
          return false;
        }
        else {

          int ishidx, Xshidx, list_size;
          std::tie(ishidx, Xshidx, list_size) = *L_3_key_iter;
          ish = ShellData(ishidx, gbs_, dfbs_);
          Xblk = ShellBlockData<>(ShellData(Xshidx, dfbs_, gbs_));
          L_3_key_iter += std::min(
              L_3_keys.end() - L_3_key_iter,
              std::iterator_traits<decltype(L_3_key_iter)>::difference_type(n_node)
          );
          return true;
        }
      }
      else {
        int spot = local_pairs_spot_++;
        if(spot < local_pairs_k_[SignificantPairs].size()){
          auto& sig_pair = local_pairs_k_[SignificantPairs][spot];
          ish = ShellData(sig_pair.first, gbs_, dfbs_);
          Xblk = sig_pair.second;
          return true;
        }
        else{
          return false;
        }
      }
    };                                                                                   //latex `\label{sc:k3b:getiXend}`
    for(int ithr = 0; ithr < nthread_; ++ithr) {
      // ...and create each thread that computes pairs
      compute_threads.create_thread([&,ithr](){                                              //latex `\label{sc:k3b:thrpatend}`

        Eigen::MatrixXd Kt_part(nbf, nbf);
        Kt_part = Eigen::MatrixXd::Zero(nbf, nbf);
        //----------------------------------------//
        ShellData ish;
        ShellBlockData<> Xblk;
        //----------------------------------------//                                         //latex `\label{sc:k3b:thrvars}`
        //----------------------------------------//
        while(get_ish_Xblk_3(ish, Xblk)) {                                                            //latex `\label{sc:k3b:while}`
          /*-----------------------------------------------------*/
          /* Compute B intermediate                         {{{2 */ #if 2 // begin fold      //latex `\label{sc:k3b:b}`
          mt_timer.enter("compute B", ithr);
          auto ints_timer = mt_timer.get_subtimer("compute ints", ithr);
          auto k2_part_timer = mt_timer.get_subtimer("k2 part", ithr);
          auto contract_timer = mt_timer.get_subtimer("contract", ithr);

          ColMatrix B_ish(ish.nbf * Xblk.nbf, nbf);
          B_ish = ColMatrix::Zero(ish.nbf * Xblk.nbf, nbf);

          if(do_linK_){

            // TODO figure out how to take advantage of L_3 sorting
            assert(Xblk.nshell == 1);
            for(const auto&& Xsh : shell_range(Xblk)) {                                              //latex `\label{sc:k3b:Xshloop}`

              int block_offset = 0;

              Eigen::MatrixXd D_ordered;                                                   //latex `\label{sc:k3b:reD}`

              if(linK_block_rho_) {

                mt_timer.enter("rearrange D", ithr);
                D_ordered.resize(nbf, nbf);
                for(auto jblk : shell_block_range(L_3[{ish, Xsh}], NoRestrictions)){

                  for(auto jsh : shell_range(jblk)) {
                    D_ordered.middleRows(block_offset, jsh.nbf) = D.middleRows(jsh.bfoff, jsh.nbf);
                    block_offset += jsh.nbf;
                  }

                }                                                                            //latex `\label{sc:k3b:reD:end}`
                mt_timer.exit(ithr);

              }

              int restrictions = linK_block_rho_ ? NoRestrictions : Contiguous;
              block_offset = 0;

              for(const auto&& jblk : shell_block_range(L_3[{ish, Xsh}], restrictions)){             //latex `\label{sc:k3b:noblk:loop}`
                TimerHolder subtimer(ints_timer);

                auto g3_ptr = ints_to_eigen(
                    jblk, ish, Xsh,
                    eris_3c_[ithr], coulomb_oper_type_
                );
                auto& g3_in = *g3_ptr;

                //----------------------------------------//
                // Two-body part

                if(!old_two_body_) {
                  // TODO This breaks integral caching (if I ever use it again)

                  subtimer.change(k2_part_timer);

                  int subblock_offset = 0;
                  for(const auto&& jsblk : shell_block_range(jblk, Contiguous|SameCenter)) {
                    int inner_size = ish.atom_dfnbf;
                    if(ish.center != jsblk.center) {
                      inner_size += jsblk.atom_dfnbf;
                    }

                    const int tot_cols = coefs_blocked_[jsblk.center].cols();
                    const int col_offset = coef_block_offsets_[jsblk.center][ish.center]
                        + ish.bfoff_in_atom*inner_size;
                    double* data_start = coefs_blocked_[jsblk.center].data() +
                        jsblk.bfoff_in_atom * tot_cols + col_offset;

                    StridedRowMap Ctmp(data_start, jsblk.nbf, ish.nbf*inner_size,
                        Eigen::OuterStride<>(tot_cols)
                    );

                    RowMatrix C(Ctmp.nestByValue());
                    C.resize(jsblk.nbf*ish.nbf, inner_size);

                    g3_in.middleRows(subblock_offset*ish.nbf, jsblk.nbf*ish.nbf) -= 0.5
                        * C.rightCols(ish.atom_dfnbf) * g2.block(
                            ish.atom_dfbfoff, Xsh.bfoff,
                            ish.atom_dfnbf, Xsh.nbf
                    );
                    if(ish.center != jsblk.center) {
                      g3_in.middleRows(subblock_offset*ish.nbf, jsblk.nbf*ish.nbf) -= 0.5
                          * C.leftCols(jsblk.atom_dfnbf) * g2.block(
                              jsblk.atom_dfbfoff, Xsh.bfoff,
                              jsblk.atom_dfnbf, Xsh.nbf
                      );
                    }

                    subblock_offset += jsblk.nbf;

                  }
                }

                //----------------------------------------//

                // Now view the integrals as a jblk.nbf x (ish.nbf*Xsh.nbf) matrix, which makes
                //   the contraction more convenient.  Doesn't require any movement of data

                Eigen::Map<ThreeCenterIntContainer> g3(g3_in.data(), jblk.nbf, ish.nbf*Xsh.nbf);

                //----------------------------------------//

                if(print_screening_stats_ > 2) {
                  mt_timer.enter("count underestimated ints", ithr);

                  double epsilon;
                  if(density_reset_){ epsilon = full_screening_thresh_; }
                  else{ epsilon = pow(full_screening_thresh_, full_screening_expon_); }

                  int offset_in_block = 0;
                  for(const auto&& jsh : shell_range(jblk)) {
                    const double g3_norm = g3.middleRows(offset_in_block, jsh.nbf).norm();
                    offset_in_block += jsh.nbf;
                    const int nfxn = jsh.nbf*ish.nbf*Xsh.nbf;
                    if(L_3[{ish, Xsh}].value_for_index(jsh) < g3_norm) {
                      ++iter_stats_->K_3c_underestimated;
                      iter_stats_->K_3c_underestimated_fxn += jsh.nbf*ish.nbf*Xsh.nbf;
                    }
                    if(g3_norm * L_DC[jsh].value_for_index(Xsh) > epsilon) {
                      ++iter_stats_->K_3c_perfect;
                      iter_stats_->K_3c_perfect_fxn += nfxn;
                    }
                    if(xml_screening_data_ and iter_stats_->is_first) {
                      iter_stats_->int_screening_values.mine(ithr).push_back(
                          L_3[{ish, Xsh}].value_for_index(jsh)
                      );
                      iter_stats_->int_actual_values.mine(ithr).push_back(g3_norm);
                      iter_stats_->int_distance_factors.mine(ithr).push_back(
                          get_distance_factor(ish, jsh, Xsh)
                      );
                      iter_stats_->int_distances.mine(ithr).push_back(
                          get_R(ish, jsh, Xsh)
                      );
                      iter_stats_->int_indices.mine(ithr).push_back(
                          std::make_tuple(ish, jsh, Xsh)
                      );
                      iter_stats_->int_ams.mine(ithr).push_back(
                          std::make_tuple(ish.am, jsh.am, Xsh.am)
                      );
                    }
                  }
                  mt_timer.exit(ithr);
                }

                //----------------------------------------//

                subtimer.change(contract_timer);

                #if !CADF_USE_BLAS
                // Eigen version
                if(linK_block_rho_) {
                  B_ish += 2.0 * g3.transpose() * D_ordered.middleRows(block_offset, jblk.nbf);
                }
                else {
                  B_ish += 2.0 * g3.transpose() * D.middleRows(jblk.bfoff, jblk.nbf);
                }
                #else
                // BLAS version
                const char notrans = 'n', trans = 't';
                const blasint M = ish.nbf*Xblk.nbf;
                const blasint K = jblk.nbf;
                const double alpha = 2.0, one = 1.0;
                double* D_data;
                if(linK_block_rho_) {
                  D_data = D_ordered.data() + block_offset;
                }
                else {
                  D_data = D.data() + jblk.bfoff;
                }
                F77_DGEMM(&notrans, &notrans,
                    &M, &nbf, &K,
                    &alpha, g3.data(), &M,
                    D_data, &nbf,
                    &one, B_ish.data(), &M
                );
                #endif

                block_offset += jblk.nbf;

              } // end loop over jsh

            } // end loop over Xsh

          } // end if do_linK_
          else {                                                                             //latex `\label{sc:k3b:nolink}`

            for(auto&& jblk : iter_significant_partners_blocked(ish, Contiguous)){
              TimerHolder subtimer(ints_timer);

              auto g3_ptr = ints_to_eigen(
                  jblk, ish, Xblk,
                  eris_3c_[ithr], coulomb_oper_type_
              );
              auto& g3_in = *g3_ptr;

              //----------------------------------------//
              // Two-body part

              // TODO This breaks integral caching (if I ever use it again)

              if(not old_two_body_) {
                subtimer.change(k2_part_timer);

                int subblock_offset = 0;
                for(const auto&& jsblk : shell_block_range(jblk, SameCenter)) {
                  int inner_size = ish.atom_dfnbf;
                  if(ish.center != jsblk.center) {
                    inner_size += jsblk.atom_dfnbf;
                  }

                  const int tot_cols = coefs_blocked_[jsblk.center].cols();
                  const int col_offset = coef_block_offsets_[jsblk.center][ish.center]
                      + ish.bfoff_in_atom*inner_size;
                  double* data_start = coefs_blocked_[jsblk.center].data() +
                      jsblk.bfoff_in_atom * tot_cols + col_offset;

                  StridedRowMap Ctmp(data_start, jsblk.nbf, ish.nbf*inner_size,
                      Eigen::OuterStride<>(tot_cols)
                  );

                  RowMatrix C(Ctmp.nestByValue());
                  C.resize(jsblk.nbf*ish.nbf, inner_size);

                  g3_in.middleRows(subblock_offset*ish.nbf, jsblk.nbf*ish.nbf) -= 0.5
                      * C.rightCols(ish.atom_dfnbf) * g2.block(
                          ish.atom_dfbfoff, Xblk.bfoff,
                          ish.atom_dfnbf, Xblk.nbf
                  );
                  if(ish.center != jsblk.center) {
                    g3_in.middleRows(subblock_offset*ish.nbf, jsblk.nbf*ish.nbf) -= 0.5
                        * C.leftCols(jsblk.atom_dfnbf) * g2.block(
                            jsblk.atom_dfbfoff, Xblk.bfoff,
                            jsblk.atom_dfnbf, Xblk.nbf
                    );
                  }

                  subblock_offset += jsblk.nbf;

                } // end loop over jsblk
              }

              //----------------------------------------//

              // Now view the integrals as a jblk.nbf x (ish.nbf*Xsh.nbf) matrix, which makes
              //   the contraction more convenient.  Doesn't require any movement of data

              Eigen::Map<ThreeCenterIntContainer> g3(g3_in.data(), jblk.nbf, ish.nbf*Xblk.nbf);

              //----------------------------------------//

              subtimer.change(contract_timer);

              #if !CADF_USE_BLAS
              // Eigen version
              B_ish += 2.0 * g3.transpose() * D.middleRows(jblk.bfoff, jblk.nbf);
              #else
              // BLAS version
              const char notrans = 'n', trans = 't';
              const blasint M = ish.nbf*Xblk.nbf;
              const blasint K = jblk.nbf;
              const double alpha = 2.0, one = 1.0;
              F77_DGEMM(&notrans, &notrans,
                  &M, &nbf, &K,
                  &alpha, g3.data(), &M,
                  D.data() + jblk.bfoff, &nbf,
                  &one, B_ish.data(), &M
              );
              #endif

            } // end loop over jsh

          } // end else (do_linK_ == false)                                                  //latex `\label{sc:k3b:bend}`
          /*******************************************************/ #endif //2}}}
          /*-----------------------------------------------------*/
          /* Compute K contributions                        {{{2 */ #if 2 // begin fold      //latex `\label{sc:k3b:kcontrib}`
          mt_timer.change("K contributions", ithr);
          const int obs_atom_bfoff = obs->shell_to_function(obs->shell_on_center(Xblk.center, 0));
          const int obs_atom_nbf = obs->nbasis_on_center(Xblk.center);
          for(auto&& X : function_range(Xblk)) {
            const auto& C_X = coefs_transpose_[X];
            for(auto&& mu : function_range(ish)) {

              // B_mus[mu.bfoff_in_shell] is (nbf x Ysh.nbf)
              // C_Y is (Y.{obs_}atom_nbf x nbf)
              // result should be (Y.{obs_}atom_nbf x 1)

              Kt_part.row(mu).segment(obs_atom_bfoff, obs_atom_nbf).transpose() +=
                  C_X * B_ish.row(mu.bfoff_in_shell*Xblk.nbf + X.bfoff_in_block).transpose();

              Kt_part.row(mu).transpose() += C_X.transpose()
                  * B_ish.row(mu.bfoff_in_shell*Xblk.nbf + X.bfoff_in_block).segment(obs_atom_bfoff, obs_atom_nbf).transpose();

              Kt_part.row(mu).segment(obs_atom_bfoff, obs_atom_nbf).transpose() -=
                  C_X.middleCols(obs_atom_bfoff, obs_atom_nbf).transpose()
                  * B_ish.row(mu.bfoff_in_shell*Xblk.nbf + X.bfoff_in_block).segment(obs_atom_bfoff, obs_atom_nbf).transpose();

              //----------------------------------------//
            }
          }
          mt_timer.exit(ithr);
          /*******************************************************/ #endif //2}}}            //latex `\label{sc:k3b:kcontrib:end}`
          /*-----------------------------------------------------*/
        } // end while get ish Xblk pair
        //============================================================================//
        //----------------------------------------//
        // Sum Kt parts within node
        boost::lock_guard<boost::mutex> lg(tmp_mutex);
        Kt += Kt_part;
      }); // end create_thread
    } // end enumeration of threads
    compute_threads.join_all();
    mt_timer.exit();
    timer.insert(mt_timer);
    if(print_iteration_timings_) mt_timer.print(ExEnv::out0(), 12, 45);
  } // compute_threads is destroyed here
  /*****************************************************************************************/ #endif //1}}} //latex `\label{sc:k3b:end}`
  /*=======================================================================================*/
  /* Loop over local shell pairs and compute two body part 		                        {{{1 */ #if 1 // begin fold
  if(old_two_body_)
  {
    Timer timer("two body contributions");
    boost::mutex tmp_mutex, L_3_mutex;
    auto L_3_key_iter = L_3.begin();
    L_3_key_iter.advance(scf_grp_->me());
    const int n_node = scf_grp_->n();
    boost::thread_group compute_threads;
    //----------------------------------------//
    // reset the iteration over local pairs
    local_pairs_spot_ = 0;
    // Loop over number of threads
    MultiThreadTimer mt_timer("threaded part", nthread_);
    for(int ithr = 0; ithr < nthread_; ++ithr) {
      // ...and create each thread that computes pairs
      compute_threads.create_thread([&,ithr](){

        Eigen::MatrixXd Kt_part(nbf, nbf);
        Kt_part = Eigen::MatrixXd::Zero(nbf, nbf);

        ShellData ish;
        ShellBlockData<> Yblk;
        //----------------------------------------//
        while(get_ish_Xblk_pair(ish, Yblk, SignificantPairs)) {

          mt_timer.enter("compute Ct", ithr);
          std::vector<RowMatrix> Ct_mus(ish.nbf);
          for(auto mu : function_range(ish)) {
            Ct_mus[mu.bfoff_in_shell].resize(nbf, Yblk.nbf);
            Ct_mus[mu.bfoff_in_shell] = Eigen::MatrixXd::Zero(nbf, Yblk.nbf);
          }
          for(auto&& Xblk : shell_block_range(dfbs_, 0, 0, NoLastIndex, SameCenter)){
            const auto& g2 = g2_full_ptr->block(Yblk.bfoff, Xblk.bfoff, Yblk.nbf, Xblk.nbf);

            if(ish.center == Xblk.center) {
              mt_timer.enter("X and ish same center", ithr);
              for(auto&& jsh : iter_significant_partners(ish)) {

                for(auto&& mu : function_range(ish)) {
                  for(auto&& rho : function_range(jsh)) {

                    BasisFunctionData first = mu, second = rho;
                    if(jsh > ish) first = rho, second = mu;
                    IntPair mu_rho(first, second);
                    assert(coefs_.find(mu_rho) != coefs_.end());
                    auto& cpair = coefs_[mu_rho];
                    auto& Cmu = (jsh <= ish || ish.center == jsh.center) ? *cpair.first : *cpair.second;
                    auto& Crho = jsh <= ish ? *cpair.second : *cpair.first;

                    Ct_mus[mu.bfoff_in_shell].row(rho) += 2.0 *
                        g2 * Cmu.segment(Xblk.bfoff_in_atom, Xblk.nbf);

                  } // end loop over rho in jsh
                } // end loop over mu in ish

              } // end loop over jsh
              mt_timer.exit(ithr);

            }
            else {
              mt_timer.enter("X and ish diff center", ithr);
              for(auto&& jsh : iter_shells_on_center(obs, Xblk.center)) {

                for(auto&& mu : function_range(ish)) {
                  for(auto&& rho : function_range(jsh)) {

                    BasisFunctionData first = mu, second = rho;

                    if(jsh > ish) first = rho, second = mu;
                    IntPair mu_rho(first, second);
                    assert(coefs_.find(mu_rho) != coefs_.end());

                    auto& cpair = coefs_[mu_rho];
                    auto& Cmu = (jsh <= ish || ish.center == jsh.center) ? *cpair.first : *cpair.second;
                    auto& Crho = jsh <= ish ? *cpair.second : *cpair.first;

                    Ct_mus[mu.bfoff_in_shell].row(rho) += 2.0 *
                        g2 * Crho.segment(Xblk.bfoff_in_atom, Xblk.nbf);

                  } // end loop over rho in jsh
                } // end loop over mu in ish

              } // end loop over jsh
              mt_timer.exit(ithr);
            }

          } // end loop over Xblk
          //----------------------------------------//
          mt_timer.change("compute dt", ithr);
          // TODO make this one matrix?
          std::vector<RowMatrix> dt_mus(ish.nbf);
          for(auto&& mu : function_range(ish)) {
            dt_mus[mu.bfoff_in_shell].resize(nbf, Yblk.nbf);
            dt_mus[mu.bfoff_in_shell] = D * Ct_mus[mu.bfoff_in_shell];
          }
          //if(xml_debug_) {
          //  for(auto&& mu : function_range(ish)){
          //    write_as_xml("new_dt_part", dt_mus[mu.bfoff_in_shell], std::map<std::string, int>{
          //      {"mu", mu}, {"Xbfoff", Yblk.bfoff},
          //      {"Xnbf", Yblk.nbf}, {"dfnbf", dfbs_->nbasis()}
          //    });
          //  }
          //}
          //----------------------------------------//
          mt_timer.change("K contribution", ithr);
          for(auto&& Y : function_range(Yblk)) {
            const auto& C_Y = coefs_transpose_[Y];
            const int obs_atom_bfoff = obs->shell_to_function(obs->shell_on_center(Y.center, 0));
            const int obs_atom_nbf = obs->nbasis_on_center(Y.center);
            for(auto&& mu : function_range(ish)) {
              // dt_mus[mu.bfoff_in_shell] is (nbf x Ysh.nbf)
              // C_Y is (Y.{obs_}atom_nbf x nbf)
              // result should be (Y.{obs_}atom_nbf x 1)
              Kt_part.row(mu).segment(obs_atom_bfoff, obs_atom_nbf).transpose() -=
                  0.5 * C_Y * dt_mus[mu.bfoff_in_shell].col(Y.bfoff_in_block);
              // The sigma <-> nu term
              Kt_part.row(mu).transpose() -= 0.5
                  * C_Y.transpose()
                  * dt_mus[mu.bfoff_in_shell].col(Y.bfoff_in_block).segment(obs_atom_bfoff, obs_atom_nbf);
              // Add back in the nu.center == Y.center part
              Kt_part.row(mu).segment(obs_atom_bfoff, obs_atom_nbf).transpose() += 0.5
                  * C_Y.middleCols(obs_atom_bfoff, obs_atom_nbf).transpose()
                  * dt_mus[mu.bfoff_in_shell].col(Y.bfoff_in_block).segment(obs_atom_bfoff, obs_atom_nbf);
              //----------------------------------------//
            }
          }
          //----------------------------------------//
          mt_timer.exit(ithr);
        } // end while get pair
        //----------------------------------------//
        // Sum Kt parts within node
        boost::lock_guard<boost::mutex> lg(tmp_mutex);
        Kt += Kt_part;
      }); // end create_thread
    } // end enumeration of threads
    compute_threads.join_all();
    mt_timer.exit();
    if(print_iteration_timings_) mt_timer.print(ExEnv::out0(), 12, 45);
    timer.insert(mt_timer);
  } // compute_threads is destroyed here
  /*****************************************************************************************/ #endif //1}}}
  /*=======================================================================================*/
  /* Global sum K                                         		                        {{{1 */ #if 1 //latex `\label{sc:kglobalsum}`
  //----------------------------------------//
  scf_grp_->sum(Kt.data(), nbf*nbf);
  //----------------------------------------//
  // Symmetrize K
  Eigen::MatrixXd K(nbf, nbf);
  K = Kt + Kt.transpose();
  //----------------------------------------//
  /*****************************************************************************************/ #endif //1}}} //latex `\label{sc:kglobalsumend}`
  /*=======================================================================================*/
  /* Transfer K to a RefSCMatrix                           		                        {{{1 */ #if 1 // begin fold
  Ref<Integral> localints = integral()->clone();
  localints->set_basis(obs);
  Ref<PetiteList> pl = localints->petite_list();
  RefSCDimension obsdim = pl->AO_basisdim();
  RefSCMatrix result(
      obsdim,
      obsdim,
      obs->so_matrixkit()
  );
  result.assign(K.data());
  /*****************************************************************************************/ #endif //1}}}
  /*=======================================================================================*/
  /* Clean up                                             		                        {{{1 */ #if 1 // begin fold
  //----------------------------------------//
  deallocate(D_ptr);
  /*****************************************************************************************/ #endif //1}}}
  /*=======================================================================================*/
  return result;
}

//////////////////////////////////////////////////////////////////////////////////

