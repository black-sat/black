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

#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>

#include <vector>

#include <tsl/hopscotch_map.h>

namespace black_internal::encoder {
  
  using namespace black_internal::logic;

  struct req_t {
    enum type_t : uint8_t {
      future,
      past,
      atom
    };

    enum strength_t : uint8_t {
      none,
      weak,
      strong
    };

    bool operator==(req_t const&) const = default;

    formula<LTLPFO> target;
    std::vector<var_decl> signature;
    type_t type;
    strength_t future_strength;
    strength_t past_strength;
  };

  formula<LTLPFO> to_formula(req_t req);

  inline std::string to_string(req_t req) {
    return "{" + to_string(to_formula(req)) + "}"; 
  }

  //
  // Functions that implement the SAT encoding. 
  // Refer to the TABLEAUX 2019 and TIME 2021 papers for details.
  //
  struct encoder 
  {
    encoder(formula<LTLPFO> f, scope &xi, bool finite) 
      : _frm{f}, _sigma{_frm.sigma()}, 
        _global_xi{&xi}, _xi{chain(xi)}, 
        _finite{finite}
    {
      _frm = to_nnf(_frm);
      _collect_requests(_frm);
    }

    encoder(encoder const&) = delete;
    encoder(encoder &&) = default;

    encoder &operator=(encoder const&) = delete;
    encoder &operator=(encoder &&) = default;

    formula<LTLPFO> get_formula() const { return _frm; }

    // Return the loop var for the loop from l to k
    static proposition loop_prop(alphabet *sigma, size_t l, size_t k);

    // Make the stepped ground version of a proposition
    static proposition stepped(proposition p, size_t k);

    // Make the stepped version of a term, t_G^k
    term<FO> stepped(term<LTLPFO> t, size_t k);

    // Make the stepped version of a variable
    variable stepped(variable x, size_t k);

    // Make the stepped version of a relation
    relation stepped(relation r, size_t k);
    
    // Make the stepped version of a function
    function stepped(function r, size_t k);

    // Make the stepped version of an atom
    atom<FO> stepped(atom<LTLPFO> a, size_t k);
    
    // Make the stepped version of an equality
    equality<FO> stepped(equality<LTLPFO> a, size_t k);
    
    // Make the stepped version of a comparison
    comparison<FO> stepped(comparison<LTLPFO> a, size_t k);

    // Put a formula in negated normal form
    formula<LTLPFO> to_nnf(formula<LTLPFO> f);

    // Put a formula in Stepped Normal Form
    formula<FO> to_ground_snf(
      formula<LTLPFO> f, size_t k, std::vector<var_decl> env
    );

    // Generates the PRUNE encoding
    formula<FO> prune(size_t k);

    // Generates the _lPRUNE_j^k encoding
    formula<FO> l_j_k_prune(size_t l, size_t j, size_t k);

    // Generates the encoding for EMPTY_k
    formula<FO> k_empty(size_t k);

    // Generates the encoding for LOOP_k
    formula<FO> k_loop(size_t k);

    // Generates the encoding for _lP_k
    formula<FO> l_to_k_period(size_t l, size_t k);

    // Generates the encoding for _lL_k
    formula<FO> l_to_k_loop(size_t l, size_t k, bool close_yesterdays);

    // Generates the k-unraveling for the given k
    formula<FO> k_unraveling(size_t k);

  private:
    // the formula to encode
    formula<LTLPFO> _frm;

    // the alphabet of frm
    alphabet *_sigma = nullptr;

    // scope 
    scope *_global_xi;

    scope _xi;

    // encode for finite models
    bool _finite = false;

    // X/Y/Z-requests from the formula's closure
    std::vector<req_t> _requests;

    // cache to memoize to_nnf() calls
    tsl::hopscotch_map<formula<LTLPFO>, formula<LTLPFO>> _nnf_cache;

    formula<FO> ground(req_t, size_t);
    formula<FO> forall(std::vector<var_decl> env, formula<FO> f);

    void _collect_requests(formula<LTLPFO> f, std::vector<var_decl> env = {});
    req_t mk_req(tomorrow<LTLPFO>, std::vector<var_decl>);
    req_t mk_req(w_tomorrow<LTLPFO>, std::vector<var_decl>);
    req_t mk_req(yesterday<LTLPFO>, std::vector<var_decl>);
    req_t mk_req(w_yesterday<LTLPFO>, std::vector<var_decl>);
    std::optional<req_t> mk_req(atom<LTLPFO>, std::vector<var_decl>);
    std::optional<req_t> mk_req(equality<LTLPFO>, std::vector<var_decl>);
    std::optional<req_t> mk_req(comparison<LTLPFO>, std::vector<var_decl>);
    req_t::strength_t future_strength(formula<LTLPFO> a);
    req_t::strength_t past_strength(formula<LTLPFO> a);
    
    void error(std::string const&msg);

    // Extract the x-eventuality from an x-request
    static std::optional<formula<LTLPFO>> _get_ev(formula<LTLPFO> f);
  };

}

namespace std {
  template<>
  struct hash<black_internal::encoder::req_t> {
    size_t operator()(black_internal::encoder::req_t r) const {
      using black_internal::encoder::req_t;
      using namespace black_internal;
      
      size_t h = std::hash<logic::formula<logic::LTLPFO>>{}(r.target);
      h = hash_combine(h, std::hash<req_t::type_t>{}(r.type));
      h = hash_combine(h, std::hash<req_t::strength_t>{}(r.future_strength));
      h = hash_combine(h, std::hash<req_t::strength_t>{}(r.past_strength));
      
      for(auto d : r.signature)
        h = hash_combine(h, d.hash());
      
      return h;
    }
  };
}

#endif
