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
#include <black/sat/solver.hpp>
#include <black/support/range.hpp>

#include <fmt/format.h>

namespace black::internal
{
  /*
   * Private implementation of the solver class.
   */
  struct solver::_solver_t 
  {
  public:
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

    // whether a model has been found 
    // i.e., whether solve() has been called and returned true
    bool model = false;

    // size of the found model (if any)
    size_t model_size = 0;

    // value for solver::last_bound() 
    size_t last_bound = 0;

    // current SAT solver instance
    std::unique_ptr<sat::solver> sat;

    // the name of the currently chosen sat backend
    std::string sat_backend = "z3"; // sensible default

  public:
    // Main algorithm
    tribool solve(size_t k_max);

    /*
     * Functions that implement the SAT encoding. 
     * Refer to the TABLEAUX 2019 and TIME 2021 papers for details.
     */

    // Make the stepped ground version of a formula, f_G^k
    atom ground(formula f, size_t k);

    // Put a formula in negated normal form
    formula to_nnf(formula f);
    formula to_nnf_inner(formula f);

    // Put a formula in Stepped Normal Form
    formula to_ground_snf(formula f, size_t k);

    // collect X/Y/Z-requests
    void add_xyz_requests(formula f);

    // Extract the x-eventuality from an x-request
    static std::optional<formula> get_xev(tomorrow xreq);

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

  solver::solver(alphabet &a)
        : _data{std::make_unique<_solver_t>(a)} { }

  solver::~solver() = default;

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

  tribool solver::solve(size_t k_max) {
    return _data->solve(k_max);
  }

  std::optional<model> solver::model() const {
    if(!_data->model)
      return {};
    
    return {{*this}};
  }

  size_t solver::last_bound() const {
    return _data->last_bound;
  }

  void solver::set_sat_backend(std::string name) {
    _data->sat_backend = std::move(name);
  }

  std::string solver::sat_backend() const {
    return _data->sat_backend;
  }

  size_t model::size() const {
    return _solver._data->model_size;
  }

  size_t model::loop() const {
    black_assert(size() > 0);
    
    size_t k = size() - 1;
    for(size_t l = 0; l < k; ++l) {
       atom loop_var = _solver._data->sigma.var(std::tuple{"_loop_var", l, k});
       if(_solver._data->sat->value(loop_var))
        return l + 1;
    }

    return size();
  }

  tribool model::value(atom a, size_t t) const {
    atom u = _solver._data->ground(a, t);

    return _solver._data->sat->value(u);
  }

  /*
   * Main algorithm. Solve the formula with up to `k_max' iterations
   */
  tribool solver::_solver_t::solve(size_t k_max)
  {
    sat = sat::solver::get_solver(sat_backend);

    last_bound = 0;
    for(size_t k = 0; k <= k_max; last_bound = k++)
    {
      // Generating the k-unraveling.
      // If it is UNSAT, then stop with UNSAT
      sat->assert_formula(k_unraveling(k));
      if(!sat->is_sat())
        return model = false;

      // else, continue to check EMPTY and LOOP.
      // If the k-unrav is SAT assuming EMPTY or LOOP, then stop with SAT
      if(sat->is_sat_with(k_empty(k) || k_loop(k))) {
        model_size = k + 1;
        model = true;
        
        return true;
      }

      // else, generate the PRUNE
      // If the PRUNE is UNSAT, the formula is UNSAT
      sat->assert_formula(!prune(k));
      if(!sat->is_sat())
        return model = false;
    } // end for

    model = false;
    return tribool::undef;
  }

  // Generates the PRUNE encoding
  formula solver::_solver_t::prune(size_t k)
  {
    return big_or(sigma, range(0, k), [&](size_t l) {
      return big_or(sigma, range(l + 1, k), [&](size_t j) {
        return l_to_k_loop(l,j) && l_to_k_loop(j,k) && l_j_k_prune(l,j,k);
      });
    });
  }


  // Generates the _lPRUNE_j^k encoding
  formula solver::_solver_t::l_j_k_prune(size_t l, size_t j, size_t k) {
    return big_and(sigma, xrequests, [&](tomorrow xreq) -> formula {
      auto req = get_xev(xreq); // consider only X-eventualities
      if(!req)
        return sigma.top();

      // Creating the encoding
      formula inner_impl = big_or(sigma, range(j + 1, k + 1), [&](size_t i) {
        return to_ground_snf(*req, i);
      });
      
      formula first_conj = ground(xreq, k) && inner_impl;
      formula second_conj = big_or(sigma, range(l + 1, j + 1), [&](size_t i) {
        return to_ground_snf(*req, i);
      });

      return implies(first_conj, second_conj);
    });
  }


  // Generates the encoding for EMPTY_k
  formula solver::_solver_t::k_empty(size_t k) {
    return big_and(sigma, xrequests, [&](tomorrow req) {
      return !ground(req, k);
    });
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
  // This is modified to allow the extraction of the loop index when printing
  // the model of the formula
  formula solver::_solver_t::k_loop(size_t k) {
    formula axioms = big_and(sigma, range(0,k), [&](size_t l) {
      atom loop_var = sigma.var(std::tuple{"_loop_var", l, k});
      return iff(loop_var, l_to_k_loop(l, k) && l_to_k_period(l, k));
    });
    

    return axioms && big_or(sigma, range(0, k), [&](size_t l) {
      return sigma.var(std::tuple{"_loop_var", l, k});
    });
  }

  // Generates the encoding for _lP_k
  formula solver::_solver_t::l_to_k_period(size_t l, size_t k) {
    return big_and(sigma, xrequests, [&](tomorrow xreq) -> formula {
      auto req = get_xev(xreq); // consider only X-eventualities
      if(!req)
        return sigma.top();
      
      // Creating the encoding
      formula atom_phi_k = ground(xreq, k);
      formula body_impl = big_or(sigma, range(l + 1, k + 1), [&](size_t i) {
        return to_ground_snf(*req, i);
      });

      return implies(atom_phi_k, body_impl);
    });
  }


  // Generates the encoding for _lR_k
  formula solver::_solver_t::l_to_k_loop(size_t l, size_t k) {
    auto make_loop = [&](auto xyz_req) {
      return iff( ground(xyz_req, l), ground(xyz_req, k) );
    };

    auto close_loop = [&](auto req) {
      formula op = req.operand();
      return iff(ground(req, l+1), to_ground_snf(op, k));
    };

    formula x = big_and(sigma, xrequests, make_loop);
    formula y = big_and(sigma, yrequests, make_loop);
    formula z = big_and(sigma, zrequests, make_loop);

    formula yy = big_and(sigma, yrequests, close_loop);
    formula zz = big_and(sigma, zrequests, close_loop);

    return x && y && z && yy && zz;
  }


  // Generates the k-unraveling step for the given k.
  formula solver::_solver_t::k_unraveling(size_t k) {
    if (k == 0) {
      formula y = big_and(sigma, yrequests, [&](formula f) {
        return !to_ground_snf(f, k);
      });

      formula z = big_and(sigma, zrequests, [&](formula f) {
        return to_ground_snf(f, k);
      });

      return to_ground_snf(frm, k) && y && z;
    }

    // STEP
    // X(\alpha)_G^{k} <-> snf(\alpha)_G^{k+1}
    formula step = big_and(sigma, xrequests, [&](tomorrow xreq) {
      return iff( ground(xreq, k - 1), to_ground_snf(xreq.operand(), k) );
    });

    // YESTERDAY and W-YESTERDAY
    // Y/Z(\alpha)_G^{k+1} <-> snf(\alpha)_G^{k}
    auto make_yz = [&](auto yz_req) {
      return iff( ground(yz_req, k), to_ground_snf(yz_req.operand(), k - 1) );
    };

    formula y = big_and(sigma, yrequests, make_yz);
    formula z = big_and(sigma, zrequests, make_yz);
    
    return step && y && z;
  }


  // Turns the current formula into Stepped Normal Form
  // Note: this has to be run *after* the transformation to NNF (to_nnf() below)
  formula solver::_solver_t::to_ground_snf(formula f, size_t k) {
    return f.match(
      [&](boolean)      { return f; },
      [&](atom)         { return ground(f, k); },
      [&](tomorrow)     { return ground(f, k); },
      [&](yesterday)    { return ground(f, k); },
      [&](w_yesterday)  { return ground(f, k); },
      [&](negation n)   { return !to_ground_snf(n.operand(),k); },
      [&](conjunction, formula left, formula right) {
        return to_ground_snf(left,k) && to_ground_snf(right,k);
      },
      [&](disjunction, formula left, formula right) {
        return to_ground_snf(left,k) || to_ground_snf(right,k);
      },
      [&](implication) -> formula {
        black_unreachable(); // removed by to_nnf() LCOV_EXCL_LINE 
      },
      [&](iff) -> formula {
        black_unreachable(); // removed by to_nnf() LCOV_EXCL_LINE
      },
      [&,this](until u, formula left, formula right) {
        return to_ground_snf(right,k) ||
            (to_ground_snf(left,k) && ground(X(u), k));
      },
      [&,this](eventually e, formula op) {
        return to_ground_snf(op,k) || ground(X(e), k);
      },
      [&,this](always a, formula op) {
        return to_ground_snf(op,k) && ground(X(a), k);
      },
      [&,this](release r, formula left, formula right) {
        return (to_ground_snf(left,k) && to_ground_snf(right,k)) ||
            (to_ground_snf(right,k) && ground(X(r), k));
      },
      [&,this](since s, formula left, formula right) {
        return to_ground_snf(right,k) ||
            (to_ground_snf(left,k) && ground(Y(s), k));
      },
      [&,this](triggered t, formula left, formula right) {
        return (to_ground_snf(left,k) && to_ground_snf(right,k) ) ||
            (to_ground_snf(right,k) && ground(Z(t), k));
      },
      [&,this](once o, formula op) {
        return to_ground_snf(op,k) || ground(Y(o), k);
      },
      [&,this](historically h, formula op) {
        return to_ground_snf(op,k) && ground(Z(h), k);
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
    black_unreachable(); // LCOV_EXCL_LINE
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
      case binary::type::iff:  // LCOV_EXCL_LINE
      case binary::type::implication:
        black_unreachable(); // LCOV_EXCL_LINE
    }
    black_unreachable(); // LCOV_EXCL_LINE
  }

  atom solver::_solver_t::ground(formula f, size_t k) {
    return sigma.var(std::pair(f,k));
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

} // end namespace black::size_ternal
