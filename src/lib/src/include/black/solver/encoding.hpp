//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
// (C) 2019 Nicola Gigante
// (C) 2020 Gabriele Venturato
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef BLACK_SOLVER_ENCODING_HPP
#define BLACK_SOLVER_ENCODING_HPP

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>

#include <vector>

#include <tsl/hopscotch_map.h>

namespace black::internal {
  
  //
  // Functions that implement the SAT encoding. 
  // Refer to the TABLEAUX 2019 and TIME 2021 papers for details.
  //
  struct encoder 
  {
    encoder(formula f) : frm{f}, sigma{frm.sigma()}
    {
      frm = to_nnf(frm);
      add_xyz_requests(frm);
    }

    // the formula to encode
    formula frm;

    // the alphabet of frm
    alphabet *sigma = nullptr;

    // X/Y/Z-requests from the formula's closure
    std::vector<tomorrow> xrequests;
    std::vector<yesterday> yrequests;
    std::vector<w_yesterday> zrequests;

    // cache to memoize to_nnf() calls
    tsl::hopscotch_map<formula, formula> nnf_cache;

    // collect X/Y/Z-requests
    void add_xyz_requests(formula f);

    // Extract the x-eventuality from an x-request
    static std::optional<formula> get_xev(tomorrow xreq);

    // Return the loop var for the loop from l to k
    atom loop_var(size_t l, size_t k);

    // Make the stepped ground version of a formula, f_G^k
    atom ground(formula f, size_t k);

    // Put a formula in negated normal form
    formula to_nnf(formula f);
    formula to_nnf_inner(formula f);

    // Put a formula in Stepped Normal Form
    formula to_ground_snf(formula f, size_t k);

    // Generates the PRUNE encoding
    formula prune(size_t k);

    // Generates the _lPRUNE_j^k encoding
    formula l_j_k_prune(size_t l, size_t j, size_t k);

    // Generates the encoding for EMPTY_k
    formula k_empty(size_t k);

    // Generates the encoding for LOOP_k
    formula k_loop(size_t k);

    // Generates the encoding for _lP_k
    formula l_to_k_period(size_t l, size_t k);

    // Generates the encoding for _lL_k
    formula l_to_k_loop(size_t l, size_t k);

    // Generates the k-unraveling for the given k
    formula k_unraveling(size_t k);
  };

}

#endif
