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

#include <black/solver/solver.hpp>
#include <black/sat/sat.hpp>

namespace black::internal
{
  struct solver::_solver_t {

    _solver_t(alphabet &a) :
      sigma{a}, frm{a.top()} { }

    // Reference to the original alphabet
    alphabet& sigma;

    // Current LTL formula to solve
    formula frm;

    // X/Y/Z-requests from the formula's closure
    // TODO: specialize to std::unordered_set<tomorrow/yesterday/w_yesterday>
    std::vector<tomorrow> xrequests;
    std::vector<yesterday> yrequests;
    std::vector<w_yesterday> zrequests;

    // cache to memoize to_nnf() calls
    tsl::hopscotch_map<formula, formula> nnf_cache;

    // the name of the currently chosen sat backend
    std::string sat_backend = "z3"; // sensible default

    bool solve(std::optional<ssize_t> k_max_arg);

    // Extract the x-eventuality from an x-request
    static std::optional<formula> get_xev(tomorrow xreq);

    // Generates the PRUNE encoding
    formula prune(ssize_t k);

    // Generates the _lPRUNE_j^k encoding
    formula l_j_k_prune(ssize_t l, ssize_t j, ssize_t k);

    // Generates the EMPTY and LOOP encoding
    formula empty_or_loop(ssize_t k);

    // Generates the encoding for EMPTY_k
    formula k_empty(ssize_t k);

    // Generates the encoding for LOOP_k
    formula k_loop(ssize_t k);

    // Generates the encoding for _lP_k
    formula l_to_k_period(ssize_t l, ssize_t k);

    // Generates the encoding for _lL_k
    formula l_to_k_loop(ssize_t l, ssize_t k);

    // Generates the k-unraveling for the given k
    formula k_unraveling(ssize_t k);

    // Generates the Stepped Normal Form of f
    formula to_ground_snf(formula f, ssize_t k);

    formula to_nnf(formula f);
    formula to_nnf_inner(formula f);

    // collect X/Y/Z-requests
    void add_xyz_requests(formula f);
  };

  solver::solver(alphabet &a)
        : _data{std::make_unique<_solver_t>(a)} { }

  solver::~solver() = default;

  bool solver::solve(std::optional<ssize_t> k_max) {
    return _data->solve(k_max);
  }

  /*
   * Solve the formula with up to `k_max' iterations
   */
  bool solver::_solver_t::solve(std::optional<ssize_t> k_max_arg)
  {
    auto sat = sat::solver::get_solver(sat_backend);

    ssize_t k_max = k_max_arg.value_or(std::numeric_limits<ssize_t>::max());

    for(ssize_t k = 0; k <= k_max; ++k)
    {
      // Generating the k-unraveling
      // if it is unsat, then stop with UNSAT
      sat->assert_formula(k_unraveling(k));
      if(!sat->is_sat())
        return false;

      // else, continue to check EMPTY and LOOP
      // if the k-unrav is SAT assuming EMPTY or LOOP, then stop with SAT
      if(sat->is_sat(empty_or_loop(k)))
        return true;

      // else, generate the PRUNE
      // if the PRUNE is unsat, the formula is unsat
      sat->assert_formula(!prune(k));
      if(!sat->is_sat())
        return false;
    } // end while(true)

    return false;
  }

  // Generates the PRUNE encoding
  formula solver::_solver_t::prune(ssize_t k)
  {
    formula k_prune = sigma.bottom();
    for(ssize_t l = 0; l < k - 1; l++) {
      formula k_prune_inner = sigma.bottom();
      for(ssize_t j = l + 1; j < k; j++) {
        formula llp =
            l_to_k_loop(l,j) && l_to_k_loop(j,k) && l_j_k_prune(l,j,k);
        k_prune_inner = k_prune_inner || llp;
      }
      k_prune = k_prune || k_prune_inner;
    }
    return k_prune;
  }


  // Generates the _lPRUNE_j^k encoding
  formula solver::_solver_t::l_j_k_prune(ssize_t l, ssize_t j, ssize_t k) {
    formula prune = sigma.top();
    for(tomorrow xreq : xrequests) {
      // If the X-requests is an X-eventuality
      if(auto req = get_xev(xreq); req) {
        // Creating the encoding
        formula first_conj = sigma.var(std::pair(formula{xreq},k));
        formula inner_impl = sigma.bottom();
        for(ssize_t i = j + 1; i <= k; i++) {
          formula xnf_req = to_ground_snf(*req, i);
          inner_impl = inner_impl || xnf_req;
        }
        first_conj = first_conj && inner_impl;
        formula second_conj = sigma.bottom();
        for(ssize_t i = l + 1; i <= j; i++) {
          formula xnf_req = to_ground_snf(*req, i);
          second_conj = second_conj || xnf_req;
        }
        prune = prune && implies(first_conj, second_conj);
      }
    }
    return prune;
  }


  // Generates the EMPTY and LOOP encoding
  formula solver::_solver_t::empty_or_loop(ssize_t k) {
    return k_empty(k) || k_loop(k);
  }

  // Generates the encoding for EMPTY_k
  formula solver::_solver_t::k_empty(ssize_t k) {
    formula k_empty = sigma.top();
    for(auto & req : xrequests) {
      k_empty = k_empty && (!( sigma.var(std::pair(formula{req},k)) ));
    }
    return k_empty;
  }

  // extract the requested formula from an X-eventuality
  std::optional<formula> solver::_solver_t::get_xev(tomorrow xreq) {
    return xreq.operand().match(
      [](eventually e) { return std::optional{e.operand()}; },
      [](until u) { return std::optional{u.right()}; },
      [](otherwise) { return std::nullopt; }
    );
  }

  // Generates the encoding for LOOP_k
  formula solver::_solver_t::k_loop(ssize_t k) {
    formula k_loop = sigma.bottom();
    for(ssize_t l = 0; l < k; l++) {
      k_loop = k_loop || (l_to_k_loop(l,k) && l_to_k_period(l,k));
    }
    return k_loop;
  }


  // Generates the encoding for _lP_k
  formula solver::_solver_t::l_to_k_period(ssize_t l, ssize_t k) {
    formula period_lk = sigma.top();
    for(tomorrow xreq : xrequests) {
      // If the X-requests is an X-eventuality
      if(auto req = get_xev(xreq); req) {
        // Creating the encoding
        formula atom_phi_k = sigma.var( std::pair(formula{xreq},k) );
        formula body_impl = sigma.bottom();
        for(ssize_t i = l + 1; i <= k; i++) {
          formula req_atom_i = to_ground_snf(*req, i);
          body_impl = body_impl || req_atom_i;
        }
        period_lk = period_lk && implies(atom_phi_k, body_impl);
      }
    }
    return period_lk;
  }


  // Generates the encoding for _lR_k
  formula solver::_solver_t::l_to_k_loop(ssize_t l, ssize_t k) {
    auto eq_fold = [&](formula acc, auto xyz_req) {
      return acc && iff(
        sigma.var(std::pair(formula{xyz_req},l)),
        sigma.var(std::pair(formula{xyz_req},k))
      );
    };

    return std::accumulate(zrequests.begin(), zrequests.end(),
        std::accumulate(yrequests.begin(), yrequests.end(),
            std::accumulate(xrequests.begin(), xrequests.end(),
                formula{sigma.top()}, eq_fold), eq_fold), eq_fold);
  }


  // Generates the k-unraveling step for the given k.
  formula solver::_solver_t::k_unraveling(ssize_t k) {
    if (k==0) {
      return std::accumulate(zrequests.begin(), zrequests.end(),
          std::accumulate(yrequests.begin(), yrequests.end(),
              to_ground_snf(frm, k),
              [&](formula acc, formula f) {
                return acc && !to_ground_snf(f, k);
              }
          ),
          [&](formula acc, formula f) {
            return acc && to_ground_snf(f, k);
          }
      );
    }

    // STEP
    formula step = std::accumulate(xrequests.begin(), xrequests.end(),
        formula{sigma.top()},
        [&](formula acc, tomorrow xreq) {
          return acc && iff(
              sigma.var(std::pair(formula{xreq},k-1)), // X(\alpha)_G^{k}
              to_ground_snf(xreq.operand(),k)           // snf(\alpha)_G^{k+1}
          );
        }
    );

    // YESTERDAY and W-YESTERDAY
    auto yz_fold = [&](formula acc, auto yz_req) {
      return acc && iff(
          sigma.var(std::pair(formula{yz_req},k)), // Y/Z(\alpha)_G^{k+1}
          to_ground_snf(yz_req.operand(),k-1)       // snf(\alpha)_G^{k}
      );
    };

    return std::accumulate(zrequests.begin(), zrequests.end(),
        std::accumulate(yrequests.begin(), yrequests.end(), step, yz_fold),
        yz_fold
    );
  }


  // Turns the current formula ssize_to Stepped Normal Form
  // Note: this has to be run *after* the transformation to NNF (to_nnf() below)
  formula solver::_solver_t::to_ground_snf(formula f, ssize_t k) {
    return f.match(
      [&](boolean)      { return f; },
      [&](atom)         { return sigma.var(std::pair(f,k)); },
      [&](tomorrow)     { return sigma.var(std::pair(f,k)); },
      [&](yesterday)    { return sigma.var(std::pair(f,k)); },
      [&](w_yesterday)  { return sigma.var(std::pair(f,k)); },
      [&](negation n)   { return !to_ground_snf(n.operand(),k); },
      [&](conjunction, formula left, formula right) {
        return to_ground_snf(left,k) && to_ground_snf(right,k);
      },
      [&](disjunction, formula left, formula right) {
        return to_ground_snf(left,k) || to_ground_snf(right,k);
      },
      [&](implication, formula left, formula right) {
        return implies(to_ground_snf(left,k), to_ground_snf(right,k));
      },
      [&](iff, formula left, formula right) {
        return iff(to_ground_snf(left,k), to_ground_snf(right,k));
      },
      [&,this](until u, formula left, formula right) {
        return to_ground_snf(right,k) ||
            (to_ground_snf(left,k) && sigma.var(std::pair(formula{X(u)},k)));
      },
      [&,this](eventually e, formula op) {
        return to_ground_snf(op,k) || sigma.var(std::pair(formula{X(e)},k));
      },
      [&,this](always a, formula op) {
        return to_ground_snf(op,k) && sigma.var(std::pair(formula{X(a)},k));
      },
      [&,this](release r, formula left, formula right) {
        return to_ground_snf(right,k) &&
            (to_ground_snf(left,k) || sigma.var(std::pair(formula{X(r)},k)));
      },
      [&,this](since s, formula left, formula right) {
        return to_ground_snf(right,k) ||
            (to_ground_snf(left,k) && sigma.var(std::pair(formula{Y(s)},k)));
      },
      [&,this](triggered t, formula left, formula right) {
        return to_ground_snf(right,k) &&
            (to_ground_snf(left,k) || sigma.var(std::pair(formula{Z(t)},k)));
      },
      [&,this](once o, formula op) {
        return to_ground_snf(op,k) || sigma.var(std::pair(formula{Y(o)},k));
      },
      [&,this](historically h, formula op) {
        return to_ground_snf(op,k) && sigma.var(std::pair(formula{Z(h)},k));
      }
    );
  }

  // Duals for temporal operators used in to_nnf()
  static constexpr unary::type dual(unary::type t) {
    switch(t) {
      case unary::type::negation:
      case unary::type::tomorrow:
        return t;
      case unary::type::yesterday:
        return unary::type::w_yesterday;
      case unary::type::w_yesterday:
        return unary::type::yesterday;
      case unary::type::always:
        return unary::type::eventually;
      case unary::type::eventually:
        return unary::type::always;
      case unary::type::once:
        return unary::type::historically;
      case unary::type::historically:
        return unary::type::once;
    }
    black_unreachable();
  }

  static constexpr binary::type dual(binary::type t)
  {
    switch(t) {
      case binary::type::conjunction:
        return binary::type::disjunction;
      case binary::type::disjunction:
        return binary::type::conjunction;
      case binary::type::until:
        return binary::type::release;
      case binary::type::release:
        return binary::type::until;
      case binary::type::since:
        return binary::type::triggered;
      case binary::type::triggered:
        return binary::type::since;
      case binary::type::iff:
      case binary::type::implication:
        black_unreachable(); // these two operators do not have simple duals
    }
    black_unreachable();
  }

  // Transformation in NNF
  formula solver::_solver_t::to_nnf(formula f) {
    if(auto it = nnf_cache.find(f); it != nnf_cache.end())
      return it->second;

    formula n = to_nnf_inner(f);
    nnf_cache.insert({f, n});
    return n;
  }

  formula solver::_solver_t::to_nnf_inner(formula f)
  {
    return f.match(
      [](boolean b) { return b; },
      [](atom a)    { return a; },
      // Push the negation down to literals
      [&](negation n) {
        return n.operand().match(
          [](boolean b) { return !b; },
          [](atom a)    { return !a; },
          [&](negation, formula op) { // special case for double negation
            return to_nnf(op);
          },
          [&](unary u) {
            return unary(dual(u.formula_type()), to_nnf(!u.operand()));
          },
          [&](implication, formula left, formula right) {
            return to_nnf(left) && to_nnf(!right);
          },
          [&](iff, formula left, formula right) {
            return to_nnf(!implies(left,right)) || to_nnf(!implies(right,left));
          },
          [&](binary b, formula left, formula right) {
            return binary(
                dual(b.formula_type()),
                to_nnf(!left), to_nnf(!right)
            );
          }
        );
      },
      // other cases: just recurse down the formula
      [&](unary u) {
        return unary(u.formula_type(), to_nnf(u.operand()));
      },
      [&](implication, formula left, formula right) {
        return to_nnf(!left) || to_nnf(right);
      },
      [&](iff, formula left, formula right) {
	      return to_nnf(implies(left, right)) && to_nnf(implies(right, left));
      },
      [&](binary b) {
        return binary(b.formula_type(), to_nnf(b.left()), to_nnf(b.right()));
      }
    );
  }

  /* Following the definition of "closure":
   * - if f is a future operator, then X(f) is in xrequests
   * - if f is S or O, then Y(f) is in yrequests
   * - if f is T or H, then Z(f) is in zrequests
   */
  void solver::_solver_t::add_xyz_requests(formula f)
  {
    f.match(
      [&](tomorrow t)     { xrequests.push_back(t); },
      [&](yesterday y)    { yrequests.push_back(y); },
      [&](w_yesterday z)  { zrequests.push_back(z); },
      [&](until u)        { xrequests.push_back(X(u)); },
      [&](release r)      { xrequests.push_back(X(r)); },
      [&](always a)       { xrequests.push_back(X(a)); },
      [&](eventually e)   { xrequests.push_back(X(e)); },
      [&](since s)        { yrequests.push_back(Y(s)); },
      [&](once o)         { yrequests.push_back(Y(o)); },
      [&](triggered t)    { zrequests.push_back(Z(t)); },
      [&](historically h) { zrequests.push_back(Z(h)); },
      [](otherwise)       { }
    );

    f.match(
      [&](unary, formula op) {
        add_xyz_requests(op);
      },
      [&](binary, formula left, formula right) {
        add_xyz_requests(left);
        add_xyz_requests(right);
      },
      [](otherwise) { }
    );
  }

  // simple public functions are given an inlineable implementation below
  void solver::assert_formula(formula f) {
    f = _data->to_nnf(f);
    _data->add_xyz_requests(f);
    if( _data->frm == _data->sigma.top() )
      _data->frm = f;
    else
      _data->frm = _data->frm && f;
  }

  void solver::clear() {
    _data = std::make_unique<_solver_t>(_data->sigma);
  }

  void solver::set_sat_backend(std::string name) {
    _data->sat_backend = std::move(name);
  }

  std::string solver::sat_backend() const {
    return _data->sat_backend;
  }

} // end namespace black::ssize_ternal
