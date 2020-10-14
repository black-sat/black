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
  /*
   * Solve the formula with up to `k_max' iterations
   */
  bool solver::solve(std::optional<int> k_max_arg)
  {
    auto sat = sat::solver::get_solver(_sat_backend);

    int k_max = k_max_arg.value_or(std::numeric_limits<int>::max());

    for(int k=0; k <= k_max; ++k)
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
  formula solver::prune(int k)
  {
    formula k_prune = _alpha.bottom();
    for(int l=0; l<k-1; l++) {
      formula k_prune_inner = _alpha.bottom();
      for(int j=l+1; j<k; j++) {
        formula llp =
            l_to_k_loop(l,j) && l_to_k_loop(j,k) && l_j_k_prune(l,j,k);
        k_prune_inner = k_prune_inner || llp;
      }
      k_prune = k_prune || k_prune_inner;
    }
    return k_prune;
  }


  // Generates the _lPRUNE_j^k encoding
  formula solver::l_j_k_prune(int l, int j, int k) {
    formula prune = _alpha.top();
    for(tomorrow xreq : _xrequests) {
      // If the X-requests is an X-eventuality
      if(auto req = get_xev(xreq); req) {
        // Creating the encoding
        formula first_conj = _alpha.var(std::pair(formula{xreq},k));
        formula inner_impl = _alpha.bottom();
        for(int i=j+1; i<=k; i++) {
          formula xnf_req = to_ground_snf(*req, i, false);
          inner_impl = inner_impl || xnf_req;
        }
        first_conj = first_conj && inner_impl;
        formula second_conj = _alpha.bottom();
        for(int i=l+1; i<=j; i++) {
          formula xnf_req = to_ground_snf(*req, i, false);
          second_conj = second_conj || xnf_req;
        }
        prune = prune && implies(first_conj, second_conj);
      }
    }
    return prune;
  }


  // Generates the EMPTY and LOOP encoding
  formula solver::empty_or_loop(int k) {
    return k_empty(k) || k_loop(k);
  }



  // Generates the encoding for EMPTY_k
  formula solver::k_empty(int k) {
    formula k_empty = _alpha.top();
    for(auto & req : _xrequests) {
      k_empty = k_empty && (!( _alpha.var(std::pair(formula{req},k)) ));
    }
    return k_empty;
  }

  // extract the requested formula from an X-eventuality
  std::optional<formula> solver::get_xev(tomorrow xreq) {
    return xreq.operand().match(
      [](eventually e) { return std::optional{e.operand()}; },
      [](until u) { return std::optional{u.right()}; },
      [](otherwise) { return std::nullopt; }
    );
  }

  // Generates the encoding for LOOP_k
  formula solver::k_loop(int k) {
    formula k_loop = _alpha.bottom();
    for(int l=0; l<k; l++) {
      k_loop = k_loop || (l_to_k_loop(l,k) && l_to_k_period(l,k));
    }
    return k_loop;
  }


  // Generates the encoding for _lP_k
  formula solver::l_to_k_period(int l, int k) {
    formula period_lk = _alpha.top();
    for(tomorrow xreq : _xrequests) {
      // If the X-requests is an X-eventuality
      if(auto req = get_xev(xreq); req) {
        // Creating the encoding
        formula atom_phi_k = _alpha.var( std::pair(formula{xreq},k) );
        formula body_impl = _alpha.bottom();
        for(int i=l+1; i<=k; i++) {
          formula req_atom_i = to_ground_snf(*req, i, false);
          body_impl = body_impl || req_atom_i;
        }
        period_lk = period_lk && implies(atom_phi_k, body_impl);
      }
    }
    return period_lk;
  }


  // Generates the encoding for _lL_k
  formula solver::l_to_k_loop(int l, int k) {
    formula loop_lk = _alpha.top();
    for(tomorrow xreq : _xrequests) {
      formula first_atom = _alpha.var( std::pair(formula{xreq},l) );
      formula second_atom = _alpha.var( std::pair(formula{xreq},k) );
      // big and formula
      loop_lk = loop_lk && iff(first_atom,second_atom);
    }
    return loop_lk;
  }


  // Generates the k-unraveling step for the given k.
  formula solver::k_unraveling(int k) {
    // Keep the X-requests generated in phase k-1.
    // Clear all the X-requests from the vector
    _xrequests.clear();

    if (k==0) {
      return std::accumulate(_zclosure.begin(), _zclosure.end(),
          std::accumulate(_yclosure.begin(), _yclosure.end(),
              to_ground_snf(_frm, k, true),
              [&](formula acc, formula f) {
                return acc && !to_ground_snf(f, k, true);
              }
          ),
          [&](formula acc, formula f) {
            return acc && to_ground_snf(f, k, true);
          }
      );
    }

    // STEP
    formula step = std::accumulate(_xclosure.begin(), _xclosure.end(),
        formula{_alpha.top()},
        [&](formula acc, tomorrow xreq) {
          return acc && iff(
              _alpha.var(std::pair(formula{xreq},k-1)), // X(\alpha)_G^{k}
              to_ground_snf(xreq.operand(),k,true)      // snf(\alpha)_G^{k+1}
          );
        }
    );

    // YESTERDAY and W-YESTERDAY
    auto yz_fold = [&](formula acc, auto yz_req) {
      return acc && iff(
          _alpha.var(std::pair(formula{yz_req},k)), // Y/Z(\alpha)_G^{k+1}
          to_ground_snf(yz_req.operand(),k-1,true)  // snf(\alpha)_G^{k}
      );
    };

    return std::accumulate(_zclosure.begin(), _zclosure.end(),
        std::accumulate(_yclosure.begin(), _yclosure.end(), step, yz_fold),
        yz_fold
    );
  }


  // Turns the current formula into Stepped Normal Form
  // Note: this has to be run *after* the transformation to NNF (to_nnf() below)
  formula solver::to_ground_snf(formula f, int k, bool update) {
    return f.match(
      [&](boolean) { return f; },
      [&](atom) { return _alpha.var(std::pair(f,k)); },
      [&,this](tomorrow t) {
        if(update)
          _xrequests.push_back(t);
        return _alpha.var(std::pair(f,k));
      },
      [&](yesterday) { return _alpha.var(std::pair(f,k)); },
      [&](w_yesterday) { return _alpha.var(std::pair(f,k)); },
      [&](negation n) { return !to_ground_snf(n.operand(),k, update); },
      [&](conjunction, formula left, formula right) {
        return to_ground_snf(left,k,update) && to_ground_snf(right,k,update);
      },
      [&](disjunction, formula left, formula right) {
        return to_ground_snf(left,k,update) || to_ground_snf(right,k,update);
      },
      [&](implication, formula left, formula right) {
        return implies(
            to_ground_snf(left,k,update),
            to_ground_snf(right,k,update)
            );
      },
      [&](iff, formula left, formula right) {
        return iff(
            to_ground_snf(left,k,update),
            to_ground_snf(right,k,update)
            );
      },
      [&,this](until u, formula left, formula right) {
        if(update)
          _xrequests.push_back(X(u));

        return to_ground_snf(right,k,update) ||
            (to_ground_snf(left,k,update) &&
            _alpha.var(std::pair(formula{X(u)},k)));
      },
      [&,this](eventually e, formula op) {
        if(update)
          _xrequests.push_back(X(e));
        return to_ground_snf(op,k,update) ||
            _alpha.var(std::pair(formula{X(e)},k));
      },
      [&,this](always a, formula op) {
        if(update)
          _xrequests.push_back(X(a));
        return to_ground_snf(op,k,update) &&
            _alpha.var(std::pair(formula{X(a)},k));
      },
      [&,this](release r, formula left, formula right) {
        if(update)
          _xrequests.push_back(X(r));
        return to_ground_snf(right,k,update) &&
            (to_ground_snf(left,k,update) ||
            _alpha.var(std::pair(formula{X(r)},k)));
      },
      [&,this](since s, formula left, formula right) {
        return to_ground_snf(right,k,update) ||
            (to_ground_snf(left,k,update) &&
            _alpha.var(std::pair(formula{Y(s)},k)));
      },
      [&,this](triggered t, formula left, formula right) {
        return to_ground_snf(left,k,update) &&
            (to_ground_snf(right,k,update) ||
            _alpha.var(std::pair(formula{Z(t)},k)));
      },
      [&,this](once o, formula op) {
        return to_ground_snf(op,k,update) ||
            _alpha.var(std::pair(formula{Y(o)},k));
      },
      [&,this](historically h, formula op) {
        return to_ground_snf(op,k,update) &&
            _alpha.var(std::pair(formula{Z(h)},k));
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
  formula solver::to_nnf(formula f) {
    if(auto it = _nnf_cache.find(f); it != _nnf_cache.end())
      return it->second;

    formula n = to_nnf_inner(f);
    _nnf_cache.insert({f, n});
    return n;
  }

  formula solver::to_nnf_inner(formula f)
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
            // return iff(to_nnf(!left), to_nnf(right));
	          return to_nnf(!implies(left,right)) ||
	    	           to_nnf(!implies(right,left));
          },
          [&](binary b, formula left, formula right) {
            return binary(dual(b.formula_type()),
                          to_nnf(!left), to_nnf(!right));
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
	      return to_nnf(implies(left, right)) &&
	             to_nnf(implies(right, left));
      },
      [&](binary b) {
        return binary(b.formula_type(), to_nnf(b.left()), to_nnf(b.right()));
      }
    );
  }

  /* Following the definition of "closure":
   * - if f is a future operator, then X(f) is in _xclosure
   * - if f is S or O, then Y(f) is in _yclosure
   * - if f is T or H, then Z(f) is in _zclosure
   */
  void solver::add_xyz_closure(formula f)
  {
    f.match(
      [&](tomorrow t)     { _xclosure.push_back(t); },
      [&](yesterday y)    { _yclosure.push_back(y); },
      [&](w_yesterday z)  { _zclosure.push_back(z); },
      [&](until u)        { _xclosure.push_back(X(u)); },
      [&](release r)      { _xclosure.push_back(X(r)); },
      [&](always a)       { _xclosure.push_back(X(a)); },
      [&](eventually e)   { _xclosure.push_back(X(e)); },
      [&](since s)        { _yclosure.push_back(Y(s)); },
      [&](once o)         { _yclosure.push_back(Y(o)); },
      [&](triggered t)    { _zclosure.push_back(Z(t)); },
      [&](historically h) { _zclosure.push_back(Z(h)); },
      [](otherwise)       { }
    );

    f.match(
      [&](unary, formula op) {
        add_xyz_closure(op);
      },
      [&](binary, formula left, formula right) {
        add_xyz_closure(left);
        add_xyz_closure(right);
      },
      [](otherwise) { }
    );
  }

  void solver::set_sat_backend(std::string name) {
    _sat_backend = std::move(name);
  }

  std::string solver::sat_backend() const {
    return _sat_backend;
  }

} // end namespace black::internal
