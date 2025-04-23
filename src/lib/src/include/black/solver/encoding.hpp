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
#include <tsl/hopscotch_set.h>

namespace black_internal::encoder {
  
  using namespace black_internal::logic;

  struct sp_witness_t {
    term standpoint;
    std::optional<formula> argument;

    bool operator==(sp_witness_t const&) const = default;
  };

  struct req_t {
    enum type_t : uint8_t {
      future,
      past
    };

    enum strength_t : uint8_t {
      weak,
      strong
    };

    friend strength_t operator|(strength_t s1, strength_t s2) {
      return strength_t(std::max(uint8_t(s1), uint8_t(s2)));
    }

    friend strength_t operator!(strength_t s) {
      return s == req_t::weak ? req_t::strong : req_t::weak;
    }

    bool operator==(req_t const&) const = default;

    formula target;
    std::vector<var_decl> signature;
    type_t type;
    strength_t strength;
    sp_witness_t sp_witness;
  };

  formula to_formula(req_t req);

  inline std::string to_string(req_t req) {
    return "{" + to_string(to_formula(req)) + "}"; 
  }
  
  inline std::string to_string(sp_witness_t sw) {
    if(sw.argument)
      return
        "<" + to_string(sw.standpoint) + ">" + to_string(*sw.argument) + ""; 
    else
      return to_string(sw.standpoint); 
  }
  
  struct lookahead_t {
    bool operator==(lookahead_t const&) const = default;

    variable target;
    req_t::type_t type;
    req_t::strength_t strength;
  };

  //
  // Functions that implement the SMT encoding.
  //
  struct encoder 
  {
    encoder(formula f, scope &xi, bool finite) 
      : _frm{f}, _sigma{_frm.sigma()}, 
        _global_xi{&xi}, _xi{chain(xi)}, 
        _finite{finite}
    {
      _frm = to_nnf(_frm);
      _collect_requests(_frm, sp_witness_t{_sigma->star(), {}});
    }

    encoder(encoder const&) = delete;
    encoder(encoder &&) = default;

    encoder &operator=(encoder const&) = delete;
    encoder &operator=(encoder &&) = default;

    formula get_formula() const { return _frm; }

    // Return the loop var for the loop from l to k
    static proposition loop_prop(alphabet *sigma, size_t l, size_t k);

    // Make the stepped ground version of a proposition
    static proposition stepped(
      proposition p, size_t k, sp_witness_t sw
    );
    
    // overload with a default star standpoint witness label
    static proposition stepped(proposition p, size_t k);

    // Make the stepped version of a term, t_G^k
    term stepped(term t, size_t k);

    // Make the stepped version of a variable
    variable stepped(variable x, size_t k);

    // Make the stepped version of a relation
    relation stepped(relation r, size_t k);
    
    // Make the stepped version of a function
    function stepped(function r, size_t k);

    // Make the stepped version of an atom
    atom stepped(atom a, size_t k);
    
    // Make the stepped version of an equality
    equality stepped(equality a, size_t k);
    
    // Make the stepped version of a comparison
    comparison stepped(comparison a, size_t k);

    // Make the last-state-wrapped version of a formula
    // (only atoms, equalities and comparisons)
    formula wrapped(formula a, size_t k);

    // Put a formula in negated normal form
    formula to_nnf(formula f);

    // Put a formula in Stepped Normal Form
    formula to_ground_snf(
      formula f, size_t k, sp_witness_t witness, 
      std::vector<var_decl> env
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

    // scope 
    scope *_global_xi;

    scope _xi;

    // encode for finite models
    bool _finite = false;

    // X/Y/Z-requests from the formula's closure
    std::vector<req_t> _requests;

    // state variables for lookaheads
    std::vector<lookahead_t> _lookaheads;

    // cache to memoize to_nnf() calls
    tsl::hopscotch_map<formula, formula> _nnf_cache;

    // all standpoint diamonds appearing in the formula
    tsl::hopscotch_set<formula> _diamonds;

    // the transitive closure of top-level sharpening formulas
    tsl::hopscotch_map<term, tsl::hopscotch_set<term>> _sharpenings;

    tsl::hopscotch_set<term> &enc(term st);
    proposition not_last_prop(size_t);
    proposition not_first_prop(size_t);
    variable ground(lookahead_t lh, size_t k);
    formula ground(req_t, size_t);
    formula forall(std::vector<var_decl> env, formula f);

    void _collect_requests(
      formula f, sp_witness_t sw, std::vector<var_decl> env = {}
    );
    void _collect_lookaheads(term t);
    void _collect_sharpening(term lhs, term rhs);

    req_t mk_req(tomorrow, sp_witness_t, std::vector<var_decl>);
    req_t mk_req(w_tomorrow, sp_witness_t, std::vector<var_decl>);
    req_t mk_req(yesterday, sp_witness_t, std::vector<var_decl>);
    req_t mk_req(w_yesterday, sp_witness_t, std::vector<var_decl>);
    
    struct formula_strength_t {
      std::optional<req_t::strength_t> future;
      std::optional<req_t::strength_t> past;
    };
    
    formula_strength_t strength(formula f);
    
    void error(std::string const&msg);

    // Extract the x-eventuality from an x-request
    static std::optional<formula> _get_ev(formula f);
  };

}

namespace std {
  template<>
  struct hash<black_internal::encoder::req_t> {
    size_t operator()(black_internal::encoder::req_t r) const {
      using black_internal::encoder::req_t;
      using namespace black_internal;
      
      size_t h = std::hash<logic::formula>{}(r.target);
      h = hash_combine(h, std::hash<req_t::type_t>{}(r.type));
      h = hash_combine(h, std::hash<req_t::strength_t>{}(r.strength));
      
      for(auto d : r.signature)
        h = hash_combine(h, d.hash());
      
      return h;
    }
  };
  
  template<>
  struct hash<black_internal::encoder::sp_witness_t> {
    size_t operator()(black_internal::encoder::sp_witness_t sp) const {
      using black_internal::encoder::sp_witness_t;
      using namespace black_internal;
      
      size_t h = std::hash<logic::term>{}(sp.standpoint);
      h = hash_combine(
        h, std::hash<std::optional<logic::formula>>{}(sp.argument)
      );

      return h;
    }
  };
}

#endif
