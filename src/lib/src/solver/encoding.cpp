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

#include <black/solver/encoding.hpp>
#include <black/support/range.hpp>

#include <string_view>

using namespace std::literals;

namespace black::internal 
{
  // Generates the PRUNE encoding
  formula encoder::prune(size_t k)
  {
    return big_or(*_sigma, range(0, k), [&](size_t l) {
      return big_or(*_sigma, range(l + 1, k), [&](size_t j) {
        return l_to_k_loop(l,j,false) && l_to_k_loop(j,k, false) && 
               l_j_k_prune(l,j,k);
      });
    });
  }


  // Generates the _lPRUNE_j^k encoding
  formula encoder::l_j_k_prune(size_t l, size_t j, size_t k) {
    return big_and(*_sigma, _xrequests, [&](unary xreq) -> formula {
      auto req = _get_xev(xreq); // consider only X-eventualities
      if(!req)
        return _sigma->top();

      // Creating the encoding
      formula inner_impl = 
        big_or(*_sigma, range(j + 1, k + 1), [&](size_t i) {
          return to_ground_snf(*req, i);
        });

      formula first_conj = ground(xreq, k) && inner_impl;
      formula second_conj = 
        big_or(*_sigma, range(l + 1, j + 1), [&](size_t i) {
          return to_ground_snf(*req, i);
        });

      return implies(first_conj, second_conj);
    });
  }


  // Generates the encoding for EMPTY_k
  formula encoder::k_empty(size_t k) {
    return big_and(*_sigma, _xrequests, [&,this](unary req) -> formula {
      if(!_finite || req.formula_type() == unary::type::tomorrow)
        return !ground(req, k);
      return _sigma->top();
    }) && !end_of_trace_prop(k);
  }

  // extract the requested formula from an X-eventuality
  std::optional<formula> encoder::_get_xev(unary xreq) {
    black_assert( // LCOV_EXCL_LINE
      xreq.formula_type() == unary::type::tomorrow ||
      xreq.formula_type() == unary::type::w_tomorrow
    );

    return xreq.operand().match(
      [](eventually e) { return std::optional{e.operand()}; },
      [](until u) { return std::optional{u.right()}; },
      [](otherwise) { return std::nullopt; }
    );
  }

  proposition encoder::loop_prop(size_t l, size_t k) {
    return _sigma->prop(std::tuple{"_loop_prop"sv, l, k});
  }

  // Generates the encoding for LOOP_k
  // This is modified to allow the extraction of the loop index when printing
  // the model of the formula
  formula encoder::k_loop(size_t k) {
    if(_finite)
      return _sigma->bottom();

    formula axioms = big_and(*_sigma, range(0,k), [&](size_t l) {
      proposition loop_prop = this->loop_prop(l, k);
      return iff(loop_prop, l_to_k_loop(l, k, true) && l_to_k_period(l, k));
    });
    

    return axioms && big_or(*_sigma, range(0, k), [&](size_t l) {
      return loop_prop(l, k);
    });
  }

  // Generates the encoding for _lP_k
  formula encoder::l_to_k_period(size_t l, size_t k) {
    return big_and(*_sigma, _xrequests, [&](unary xreq) -> formula {
      auto req = _get_xev(xreq); // consider only X-eventualities
      if(!req)
        return _sigma->top();
      
      // Creating the encoding
      formula proposition_phi_k = ground(xreq, k);
      formula body_impl = 
        big_or(*_sigma, range(l + 1, k + 1), [&](size_t i) {
          return to_ground_snf(*req, i);
        });

      return implies(proposition_phi_k, body_impl);
    });
  }


  // Generates the encoding for _lR_k
  formula encoder::l_to_k_loop(size_t l, size_t k, bool close_yesterdays) {
    auto make_loop = [&](auto xyz_req) {
      return iff( ground(xyz_req, l), ground(xyz_req, k) );
    };

    auto close_loop = [&](auto req) {
      formula op = req.operand();
      return iff(ground(req, l+1), to_ground_snf(op, k));
    };

    formula x = big_and(*_sigma, _xrequests, make_loop);
    formula y = big_and(*_sigma, _yrequests, make_loop);
    formula z = big_and(*_sigma, _zrequests, make_loop);

    if(close_yesterdays) {
      formula yy = big_and(*_sigma, _yrequests, close_loop) ;
      formula zz = big_and(*_sigma, _zrequests, close_loop);

      return x && y && z && yy && zz;
    }
    
    return x && y && z;
  }


  // Generates the k-unraveling step for the given k.
  formula encoder::k_unraveling(size_t k) {
    if (k == 0) {
      formula y = big_and(*_sigma, _yrequests, [&](formula f) {
        return !to_ground_snf(f, k);
      });

      formula z = big_and(*_sigma, _zrequests, [&](formula f) {
        return to_ground_snf(f, k);
      });

      return to_ground_snf(_frm, k) && y && z;
    }

    // \ell_k
    formula ell = end_of_trace_prop(k - 1);

    // STEP
    // X(\alpha)_G^{k} <-> snf(\alpha)_G^{k+1}
    formula step = big_and(*_sigma, _xrequests, [&](unary xreq) {
      return iff( ground(xreq, k - 1), to_ground_snf(xreq.operand(), k) );
    });

    // YESTERDAY and W-YESTERDAY
    // Y/Z(\alpha)_G^{k+1} <-> snf(\alpha)_G^{k}
    auto make_yz = [&](auto yz_req) {
      return iff( ground(yz_req, k), to_ground_snf(yz_req.operand(), k - 1) );
    };

    formula y = big_and(*_sigma, _yrequests, make_yz);
    formula z = big_and(*_sigma, _zrequests, make_yz);
    
    return ell && step && y && z;
  }

  bool encoder::atom_is_strong(atom a) {
    for(term t : a.terms())
      if(term_is_strong(t))
        return true;
    return false;
  }

  bool encoder::atom_is_weak(atom a) {
    bool has_strong_terms = false;
    bool has_weak_terms = false;

    for(term t : a.terms()) {
      if(term_is_strong(t))
        has_strong_terms = true;
      if(term_is_weak(t))
        has_weak_terms = true;
    }

    return !has_strong_terms && has_weak_terms;
  }

  bool encoder::term_is_strong(term t) {
    return t.match(
      [](constant) { return false; },
      [](variable) { return false; },
      [&](application a) {
        for(term t2 : a.arguments())
          if(term_is_strong(t2))
            return true;
        return false;
      },
      [](next) { return true; },
      [](wnext) { return false; }
    );
  }

  bool encoder::term_is_weak(term t) {
    bool has_next = false;
    bool has_wnext = false;

    std::function<void(term)> check = [&](term t2) {
      t2.match( // LCOV_EXCL_LINE
        [](constant) { },
        [](variable) { },
        [&](application a) {
          for(term t3 : a.arguments())
            check(t3);
        },
        [&](next) {
          has_next = true;
        },
        [&](wnext) {
          has_wnext = true;
        }
      );
    };

    check(t);

    return !has_next && has_wnext;
  }

  formula encoder::end_of_trace_prop(size_t i) {
    return _sigma->prop(std::tuple{"_end_of_trace_prop"sv, i});
  }

  // Turns the current formula into Stepped Normal Form
  // Note: this has to be run *after* the transformation to NNF (to_nnf() below)
  formula encoder::to_ground_snf(formula f, size_t k) {
    return to_ground_snf(f, k, {});
  }

  formula encoder::to_ground_snf(
    formula f, size_t k, std::vector<variable> const&scope
  ) {
    return f.match( // LCOV_EXCL_LINE
      [&](boolean)      { return f; },
      [&](atom a) -> formula { 
        std::vector<term> terms;
        for(term t : a.terms())
          terms.push_back(stepped(t, k, scope));

        black_assert(!(atom_is_strong(a) && atom_is_weak(a))); // LCOV_EXCL_LINE
        if(atom_is_weak(a))
          return !end_of_trace_prop(k) || atom(a.rel(), terms);
        if(atom_is_strong(a))
          return end_of_trace_prop(k) && atom(a.rel(), terms);

        return atom(a.rel(), terms);
      }, // LCOV_EXCL_LINE
      [&](quantifier q) {
        std::vector<variable> new_scope = scope;
        new_scope.push_back(q.var());
        return quantifier(
          q.quantifier_type(), q.var(), to_ground_snf(q.matrix(), k, new_scope)
        );
      }, // LCOV_EXCL_LINE
      [&](proposition)  { return ground(f, k); },
      [&](negation n)   { return !to_ground_snf(n.operand(),k, scope); },
      [&](big_conjunction c) {
        return big_and(*f.sigma(), c.operands(), [&](formula op) {
          return to_ground_snf(op, k, scope);
        });
      },
      [&](big_disjunction c) {
        return big_or(*f.sigma(), c.operands(), [&](formula op) {
          return to_ground_snf(op, k, scope);
        });
      },
      [&](implication) -> formula { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE 
      },
      [&](iff) -> formula { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE
      },
      [&](tomorrow)     { return ground(f, k); },
      [&](w_tomorrow)   { return ground(f, k); },
      [&](yesterday)    { return ground(f, k); },
      [&](w_yesterday)  { return ground(f, k); },
      [&](until u, formula left, formula right) {
        return to_ground_snf(right,k, scope) ||
            (to_ground_snf(left,k, scope) && ground(X(u), k));
      },
      [&](w_until w, formula left, formula right) {
        return to_ground_snf(right, k) ||
            (to_ground_snf(left,k, scope) && ground(wX(w), k));
      },
      [&](eventually e, formula op) {
        return to_ground_snf(op,k, scope) || ground(X(e), k);
      },
      [&](always a, formula op) {
        return to_ground_snf(op,k, scope) && ground(wX(a), k);
      },
      [&](release r, formula left, formula right) {
        return 
          (to_ground_snf(left,k, scope) && to_ground_snf(right,k,scope)) ||
            (to_ground_snf(right,k, scope) && ground(wX(r), k));
      },
      [&](s_release r, formula left, formula right) {
        return 
          (to_ground_snf(left,k, scope) && to_ground_snf(right,k,scope)) ||
            (to_ground_snf(right,k, scope) && ground(X(r), k));
      },
      [&](since s, formula left, formula right) {
        return to_ground_snf(right,k, scope) ||
            (to_ground_snf(left,k, scope) && ground(Y(s), k));
      },
      [&](triggered t, formula left, formula right) {
        return 
          (to_ground_snf(left,k, scope) && to_ground_snf(right,k, scope))
            || (to_ground_snf(right,k, scope) && ground(Z(t), k));
      },
      [&](once o, formula op) {
        return to_ground_snf(op,k, scope) || ground(Y(o), k);
      },
      [&](historically h, formula op) {
        return to_ground_snf(op,k, scope) && ground(Z(h), k);
      }
    );
  }

  // Duals for temporal operators used in to_nnf()
  static constexpr unary::type dual(unary::type t) {
    switch(t) {
      case unary::type::negation:
      case unary::type::tomorrow:
        return unary::type::w_tomorrow;
      case unary::type::w_tomorrow:
        return unary::type::tomorrow;
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
      case binary::type::until:
        return binary::type::release;
      case binary::type::release:
        return binary::type::until;
      case binary::type::w_until:
        return binary::type::s_release;
      case binary::type::s_release:
        return binary::type::w_until;
      case binary::type::since:
        return binary::type::triggered;
      case binary::type::triggered:
        return binary::type::since;
      case binary::type::conjunction: // LCOV_EXCL_LINE
      case binary::type::disjunction: // LCOV_EXCL_LINE
      case binary::type::iff:         // LCOV_EXCL_LINE
      case binary::type::implication: // LCOV_EXCL_LINE
        black_unreachable();          // LCOV_EXCL_LINE
    }
    black_unreachable(); // LCOV_EXCL_LINE
  }

  term encoder::stepped(term t, size_t k, std::vector<variable> const&scope) {
    return t.match( // LCOV_EXCL_LINE
      [](constant c) { return c; },
      [&](variable x) { 
        for(variable v : scope)
          if(x == v)
            return x;
        return _sigma->var(std::pair(t, k)); 
      },
      [&](application a) {
        std::vector<term> terms;
        for(term ti : a.arguments())
          terms.push_back(stepped(ti, k, scope));
        
        return application(a.func(), terms);
      }, // LCOV_EXCL_LINE
      [&](next n) -> term {
        return stepped(n.argument(), k + 1, scope);
      },
      [&](wnext n) {
        return stepped(n.argument(), k + 1, scope);
      }
    );
  }

  proposition encoder::ground(formula f, size_t k) {
    return _sigma->prop(std::pair(f,k));
  }

  // Transformation in NNF
  formula encoder::to_nnf(formula f) {
    if(auto it = _nnf_cache.find(f); it != _nnf_cache.end())
      return it->second;     

    formula nnf = f.match( // LCOV_EXCL_LINE
      [](boolean b) { return b; },
      [](proposition p) { return p; },
      [](atom a) { return a; },
      [&](quantifier q) {
        return quantifier(q.quantifier_type(), q.var(), to_nnf(q.matrix()));
      },
      // Push the negation down to literals
      [&](negation n) {
        return n.operand().match(
          [](boolean b)     { return !b; },
          [](proposition p) { return !p; },
          [](atom a)        { return !a; },
          [&](quantifier q) {
            quantifier::type dual = 
              q.quantifier_type() == quantifier::type::exists ? // LCOV_EXCL_LINE
              quantifier::type::forall : quantifier::type::exists;

            return quantifier(dual, q.var(), to_nnf(!q.matrix()));
          },
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
          [&](big_conjunction c) {
            return big_or(*f.sigma(), c.operands(), [&](formula op) {
              return to_nnf(!op);
            });
          },
          [&](big_disjunction c) {
            return big_and(*f.sigma(), c.operands(), [&](formula op) {
              return to_nnf(!op);
            });
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
      [&](big_conjunction c) {
        return big_and(*f.sigma(), c.operands(), [&](formula op) {
          return to_nnf(op);
        });
      },
      [&](big_disjunction c) {
        return big_or(*f.sigma(), c.operands(), [&](formula op) {
          return to_nnf(op);
        });
      },
      [&](binary b) {
        return binary(b.formula_type(), to_nnf(b.left()), to_nnf(b.right()));
      }
    );

    _nnf_cache.insert({f, nnf});
    return nnf;
  }

  /* Following the definition of "closure":
   * - if f is a future operator, then X(f) is in _xrequests
   * - if f is S or O, then Y(f) is in _yrequests
   * - if f is T or H, then Z(f) is in _zrequests
   */
  void encoder::_add_xyz_requests(formula f)
  {    
    f.match(
      [&](tomorrow t)     { _xrequests.push_back(t);     },
      [&](w_tomorrow w)   { _xrequests.push_back(w);     },
      [&](yesterday y)    { _yrequests.push_back(y);     },
      [&](w_yesterday z)  { _zrequests.push_back(z);     },
      [&](until u)        { _xrequests.push_back(X(u));  },
      [&](release r)      { _xrequests.push_back(wX(r)); },
      [&](w_until r)      { _xrequests.push_back(wX(r)); },
      [&](s_release r)    { _xrequests.push_back(X(r));  },
      [&](always a)       { _xrequests.push_back(wX(a)); },
      [&](eventually e)   { _xrequests.push_back(X(e));  },
      [&](since s)        { _yrequests.push_back(Y(s));  },
      [&](once o)         { _yrequests.push_back(Y(o));  },
      [&](triggered t)    { _zrequests.push_back(Z(t));  },
      [&](historically h) { _zrequests.push_back(Z(h));  },
      [](otherwise)       { }
    );

    f.match(
      [](boolean) { },
      [](proposition) { },
      [](quantifier) { },
      [](atom) { },
      [&](unary, formula op) {
        _add_xyz_requests(op);
      },
      [&](big_conjunction c) {
        for(formula op : c.operands())
          _add_xyz_requests(op);
      },
      [&](big_disjunction c) {
        for(formula op : c.operands())
          _add_xyz_requests(op);
      },
      [&](binary, formula left, formula right) {
        _add_xyz_requests(left);
        _add_xyz_requests(right);
      }
    );
  }
}
