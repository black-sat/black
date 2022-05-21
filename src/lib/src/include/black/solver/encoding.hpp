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
    encoder(formula f, bool finite) 
      : _frm{f}, _sigma{_frm.sigma()}, _finite{finite}
    {
      _frm = to_nnf(_frm);
      _add_xyz_requests(_frm);
    }

    formula get_formula() const { return _frm; }

    // Return the loop var for the loop from l to k
    proposition loop_prop(size_t l, size_t k);

    // Make the stepped ground version of a formula, f_G^k
    proposition ground(formula f, size_t k);

    // Make the stepped version of a term, t_G^k
    term stepped(term t, size_t k, std::vector<variable> const& scope);

    // Make the stepped version of a relation
    relation stepped(relation r, size_t k);
    
    // Make the stepped version of a function
    function stepped(function r, size_t k);

    // Put a formula in negated normal form
    formula to_nnf(formula f);

    // Put a formula in Stepped Normal Form
    formula to_ground_snf(formula f, size_t k);
    formula to_ground_snf(
      formula f, size_t k, std::vector<variable> const&scope
    );

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
    formula l_to_k_loop(size_t l, size_t k, bool close_yesterdays);

    // Generates the k-unraveling for the given k
    formula k_unraveling(size_t k);

  private:
    // the formula to encode
    formula _frm;

    // the alphabet of frm
    alphabet *_sigma = nullptr;

    // encode for finite models
    bool _finite = false;

    // X/Y/Z-requests from the formula's closure
    std::vector<unary> _xrequests;
    std::vector<yesterday> _yrequests;
    std::vector<w_yesterday> _zrequests;

    // cache to memoize to_nnf() calls
    tsl::hopscotch_map<formula, formula> _nnf_cache;

    // collect X/Y/Z-requests
    void _add_xyz_requests(formula f);
    bool atom_has_strong_prev(atom a);
    bool atom_has_weak_prev(atom a);
    bool term_has_strong_prev(term t);
    bool term_has_weak_prev(term t);
    bool atom_is_strong(atom a);
    bool atom_is_weak(atom a);
    bool term_is_strong(term t);
    bool term_is_weak(term t);
    formula end_of_trace_prop(size_t i);
    void error(std::string const&msg);

    // Extract the x-eventuality from an x-request
    static std::optional<formula> _get_xev(unary xreq);
  };

}

#endif
