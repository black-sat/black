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
#include <black/logic/prettyprint.hpp>

#include <string_view>

using namespace std::literals;

namespace black_internal::encoder
{
  // Generates the PRUNE encoding
  formula<FO> encoder::prune(size_t k)
  {
    return big_or(*_sigma, range(0, k), [&](size_t l) {
      return big_or(*_sigma, range(l + 1, k), [&](size_t j) {
        return l_to_k_loop(l,j,false) && l_to_k_loop(j,k, false) && 
               l_j_k_prune(l,j,k);
      });
    });
  }


  // Generates the _lPRUNE_j^k encoding
  formula<FO> encoder::l_j_k_prune(size_t l, size_t j, size_t k) {
    return 
      big_and(*_sigma, _xrequests, [&](unary<LTLPFO> xreq) -> formula<FO> 
    {
      std::optional<formula<LTLPFO>> req = _get_xev(xreq); 
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
  formula<FO> encoder::k_empty(size_t k) {
    return big_and(*_sigma, _xrequests, 
    [&,this](unary<LTLPFO> req) -> formula<FO> {
      if(!_finite || req.node_type() == unary<LTLPFO>::type::tomorrow{})
        return !ground(req, k);
      return _sigma->top();
    }) && !end_of_trace_prop(k);
  }

  // extract the requested formula from an X-eventuality
  std::optional<formula<LTLPFO>> encoder::_get_xev(unary<LTLPFO> xreq) {
    black_assert( // LCOV_EXCL_LINE
      xreq.node_type() == unary<LTLPFO>::type::tomorrow{} ||
      xreq.node_type() == unary<LTLPFO>::type::w_tomorrow{}
    );

    return xreq.argument().match(
      [](eventually<LTLPFO> e) { return std::optional{e.argument()}; },
      [](until<LTLPFO> u) { return std::optional{u.right()}; },
      [](otherwise) { return std::nullopt; }
    );
  }

  proposition encoder::loop_prop(alphabet *sigma, size_t l, size_t k) {
    return sigma->proposition(std::tuple{"_loop_prop"sv, l, k});
  }

  // Generates the encoding for LOOP_k
  // This is modified to allow the extraction of the loop index when printing
  // the model of the formula
  formula<FO> encoder::k_loop(size_t k) {
    if(_finite)
      return _sigma->bottom();

    formula axioms = big_and(*_sigma, range(0,k), [&](size_t l) {
      proposition lp = loop_prop(_sigma, l, k);
      return iff(lp, l_to_k_loop(l, k, true) && l_to_k_period(l, k));
    });
    

    return // LCOV_EXCL_LINE
      axioms && big_or(*_sigma, range(0, k), [&](size_t l) { // LCOV_EXCL_LINE
      return loop_prop(_sigma, l, k);
    });
  }

  // Generates the encoding for _lP_k
  formula<FO> encoder::l_to_k_period(size_t l, size_t k) {
    return big_and(*_sigma, _xrequests, [&](unary<LTLPFO> xreq) -> formula<FO> {
      std::optional<formula<LTLPFO>> req = _get_xev(xreq);
      if(!req)
        return _sigma->top();
      
      // Creating the encoding
      formula<FO> proposition_phi_k = ground(xreq, k);
      formula<FO> body_impl = 
        big_or(*_sigma, range(l + 1, k + 1), [&](size_t i) {
          return to_ground_snf(*req, i);
        });

      return implies(proposition_phi_k, body_impl);
    });
  }


  // Generates the encoding for _lR_k
  formula<FO> encoder::l_to_k_loop(size_t l, size_t k, bool close_yesterdays) {
    auto make_loop = [&](auto xyz_req) {
      return iff( ground(xyz_req, l), ground(xyz_req, k) );
    };

    auto close_loop = [&](auto req) {
      formula<LTLPFO> op = req.argument();
      return iff(ground(req, l+1), to_ground_snf(op, k));
    };

    formula<FO> x = big_and(*_sigma, _xrequests, make_loop);
    formula<FO> y = big_and(*_sigma, _yrequests, make_loop);
    formula<FO> z = big_and(*_sigma, _zrequests, make_loop);

    if(close_yesterdays) {
      formula<FO> yy = big_and(*_sigma, _yrequests, close_loop) ;
      formula<FO> zz = big_and(*_sigma, _zrequests, close_loop);

      return x && y && z && yy && zz;
    }
    
    return x && y && z;
  }


  // Generates the k-unraveling step for the given k.
  formula<FO> encoder::k_unraveling(size_t k) {
    if (k == 0) {
      formula<FO> y = big_and(*_sigma, _yrequests, [&](formula<LTLPFO> f) {
        return !to_ground_snf(f, k);
      });

      formula<FO> z = big_and(*_sigma, _zrequests, [&](formula<LTLPFO> f) {
        return to_ground_snf(f, k);
      });

      return to_ground_snf(_frm, k) && y && z;
    }

    // \ell_k
    formula<FO> ell = end_of_trace_prop(k - 1);

    // STEP
    // X(\alpha)_G^{k} <-> snf(\alpha)_G^{k+1}
    formula<FO> step = big_and(*_sigma, _xrequests, [&](unary<LTLPFO> xreq) {
      return iff( ground(xreq, k - 1), to_ground_snf(xreq.argument(), k) );
    });

    // YESTERDAY and W-YESTERDAY
    // Y/Z(\alpha)_G^{k+1} <-> snf(\alpha)_G^{k}
    auto make_yz = [&](auto yz_req) {
      return iff( ground(yz_req, k), to_ground_snf(yz_req.argument(), k - 1) );
    };

    formula<FO> y = big_and(*_sigma, _yrequests, make_yz);
    formula<FO> z = big_and(*_sigma, _zrequests, make_yz);
    
    return ell && step && y && z;
  }

  bool encoder::is_strong(formula<LTLPFO> f) {
    return has_any_element_of(f, syntax_element::next);
  }

  bool encoder::is_weak(formula<LTLPFO> f) {
    bool has_next = has_any_element_of(f, syntax_element::next);
    bool has_wnext = has_any_element_of(f, syntax_element::wnext);

    return !has_next && has_wnext;
  }

  formula<FO> encoder::end_of_trace_prop(size_t i) {
    return _sigma->proposition(std::tuple{"_end_of_trace_prop"sv, i});
  }

  formula<FO> encoder::end_of_trace_semantics(
    formula<LTLPFO> f, formula<FO> stepped_f, size_t k
  ) {
    black_assert(!(is_strong(f) && is_weak(f)));

    if(is_weak(f)) {
      return !end_of_trace_prop(k) || stepped_f;
    }
    if(is_strong(f))
      return end_of_trace_prop(k) && stepped_f;

    return stepped_f;
  }

  std::optional<formula<FO>>
  encoder::start_of_trace_semantics(formula<LTLPFO> f, size_t k) {
    if(has_any_element_of(f, syntax_element::prev) && k == 0)
      return {f.sigma()->bottom()};

    if(has_any_element_of(f, syntax_element::wprev) && k == 0)
      return {f.sigma()->top()};

    return {};
  }

  // Turns the current formula into Stepped Normal Form
  // Note: this has to be run *after* the transformation to NNF (to_nnf() below)
  formula<FO> encoder::to_ground_snf(
    formula<LTLPFO> f, size_t k, bool quant
  ) {
    return f.match( // LCOV_EXCL_LINE
      [&](boolean b)      { return b; },
      [&](atom<LTLPFO> a) -> formula<FO> {   
        if(auto s = start_of_trace_semantics(a, k); s)
          return *s;

        std::vector<term<FO>> terms;
        for(term<LTLPFO> t : a.terms())
          terms.push_back(stepped(t, k));

        relation stepped_rel = stepped(a.rel(), k);
        return end_of_trace_semantics(a, atom(stepped_rel, terms), k);
      }, // LCOV_EXCL_LINE
      [&](equality<LTLPFO> e, auto terms) -> formula<FO> {
        std::vector<term<FO>> stepterms;
        for(auto t : terms)
          stepterms.push_back(stepped(t, k));
        
        return end_of_trace_semantics(
          e, equality<FO>(e.node_type(), stepterms), k
        );
      },
      [&](comparison<LTLPFO> c, auto left, auto right) -> formula<FO> {
        if(auto s = start_of_trace_semantics(c, k); s)
          return *s;

        term<FO> stepleft = stepped(left, k);
        term<FO> stepright = stepped(right, k);

        return end_of_trace_semantics(
          c, comparison<FO>(c.node_type(), stepleft, stepright), k
        );
      },
      [&](quantifier<LTLPFO> q) {
        nest_scope_t nest{_xi};
        
        _xi.declare(q.decl(), scope::rigid);

        auto result = quantifier<FO>(
          q.node_type(), q.decl(), to_ground_snf(q.matrix(), k)
        );

        return result;
      }, // LCOV_EXCL_LINE
      [&](proposition)  { return ground(f, k); },
      [&](negation<LTLPFO> n) { 
        return !to_ground_snf(n.argument(),k, quant); 
      },
      [&](conjunction<LTLPFO> c) {
        return big_and(*f.sigma(), c.operands(), [&](auto op) {
          return to_ground_snf(op, k, quant);
        });
      },
      [&](disjunction<LTLPFO> c) {
        return big_or(*f.sigma(), c.operands(), [&](auto op) {
          return to_ground_snf(op, k, quant);
        });
      },
      [&](implication<LTLPFO>) -> formula<FO> { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE 
      },
      [&](iff<LTLPFO>) -> formula<FO> { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE
      },
      [&](tomorrow<LTLPFO>, auto arg) -> formula<FO> { 
        if(!quant)
          return ground(f, k);
        return end_of_trace_prop(k) && to_ground_snf(arg, k+1, quant);
      },
      [&](w_tomorrow<LTLPFO>, auto arg) -> formula<FO> { 
        if(!quant)
          return ground(f, k);
        return !end_of_trace_prop(k) || to_ground_snf(arg, k+1, quant);
      },
      [&](yesterday<LTLPFO>, auto arg) -> formula<FO> { 
        if(!quant)
          return ground(f, k);
        return 
          k > 0 ? to_ground_snf(arg, k-1, quant) : f.sigma()->bottom();
      },
      [&](w_yesterday<LTLPFO>, auto arg) -> formula<FO> { 
        if(!quant)
          return ground(f, k);
        return k > 0 ? to_ground_snf(arg, k-1, quant) : f.sigma()->top();
      },
      [&](until<LTLPFO> u, auto left, auto right) {
        return to_ground_snf(right,k, quant) ||
            (to_ground_snf(left,k, quant) && ground(X(u), k));
      },
      [&](w_until<LTLPFO> w, auto left, auto right) {
        return to_ground_snf(right, k) ||
            (to_ground_snf(left,k, quant) && ground(wX(w), k));
      },
      [&](eventually<LTLPFO> e, auto op) {
        return to_ground_snf(op,k, quant) || ground(X(e), k);
      },
      [&](always<LTLPFO> a, auto op) {
        return to_ground_snf(op,k, quant) && ground(wX(a), k);
      },
      [&](release<LTLPFO> r, auto left, auto right) {
        return 
          (to_ground_snf(left, k, quant) && to_ground_snf(right, k, quant)) ||
            (to_ground_snf(right, k, quant) && ground(wX(r), k));
      },
      [&](s_release<LTLPFO> r, auto left, auto right) {
        return 
          (to_ground_snf(left, k, quant) && to_ground_snf(right, k, quant)) ||
            (to_ground_snf(right, k, quant) && ground(X(r), k));
      },
      [&](since<LTLPFO> s, auto left, auto right) {
        return to_ground_snf(right, k, quant) ||
            (to_ground_snf(left, k, quant) && ground(Y(s), k));
      },
      [&](triggered<LTLPFO> t, auto left, auto right) {
        return (
          to_ground_snf(left,k, quant) && 
          to_ground_snf(right,k, quant)
        ) || (to_ground_snf(right,k, quant) && ground(Z(t), k));
      },
      [&](once<LTLPFO> o, auto op) {
        return to_ground_snf(op,k, quant) || ground(Y(o), k);
      },
      [&](historically<LTLPFO> h, auto op) {
        return to_ground_snf(op,k, quant) && ground(Z(h), k);
      }
    );
  }

  // Duals for temporal operators used in to_nnf()
  static unary<LTLPFO>::type dual(unary<LTLPFO>::type t) {
    return t.match(
      [&](unary<LTLPFO>::type::negation) { // LCOV_EXCL_LINE
        return unary<LTLPFO>::type::negation{}; // LCOV_EXCL_LINE
      },
      [&](unary<LTLPFO>::type::tomorrow) {
        return unary<LTLPFO>::type::w_tomorrow{};
      },
      [&](unary<LTLPFO>::type::w_tomorrow) {
        return unary<LTLPFO>::type::tomorrow{};
      },
      [&](unary<LTLPFO>::type::yesterday) {
        return unary<LTLPFO>::type::w_yesterday{};
      },
      [&](unary<LTLPFO>::type::w_yesterday) {
        return unary<LTLPFO>::type::yesterday{};
      },
      [&](unary<LTLPFO>::type::always) {
        return unary<LTLPFO>::type::eventually{};
      },
      [&](unary<LTLPFO>::type::eventually) {
        return unary<LTLPFO>::type::always{};
      },
      [&](unary<LTLPFO>::type::once) {
        return unary<LTLPFO>::type::historically{};
      },
      [&](unary<LTLPFO>::type::historically) {
        return unary<LTLPFO>::type::once{};
      }
    );
  }

  static binary<LTLPFO>::type dual(binary<LTLPFO>::type t)
  {
    return t.match(
      [](binary<LTLPFO>::type::until) {
        return binary<LTLPFO>::type::release{};
      },
      [](binary<LTLPFO>::type::release) {
        return binary<LTLPFO>::type::until{};
      },
      [](binary<LTLPFO>::type::w_until) {
        return binary<LTLPFO>::type::s_release{};
      },
      [](binary<LTLPFO>::type::s_release) {
        return binary<LTLPFO>::type::w_until{};
      },
      [](binary<LTLPFO>::type::since) {
        return binary<LTLPFO>::type::triggered{};
      },
      [](binary<LTLPFO>::type::triggered) {
        return binary<LTLPFO>::type::since{};
      },
      [](otherwise) -> binary<LTLPFO>::type { // LCOV_EXCL_LINE
        black_unreachable(); // LCOV_EXCL_LINE
      }
    );
  }

  term<FO>
  encoder::stepped(term<LTLPFO> t, size_t k) 
  {
    return t.match( // LCOV_EXCL_LINE
      [](constant<LTLPFO> c) { return fragment_unsafe_cast<FO>(c); },
      [&](variable x) { 
        return stepped(x, k);
      },
      [&](application<LTLPFO> a) {
        std::vector<term<FO>> terms;
        for(term ti : a.terms())
          terms.push_back(stepped(ti, k));
        
        return application<FO>(stepped(a.func(), k), terms);
      }, // LCOV_EXCL_LINE
      [&](to_integer<LTLPFO>, auto arg) {
        return to_integer(stepped(arg, k));
      },
      [&](to_real<LTLPFO>, auto arg) {
        return to_real(stepped(arg, k));
      },
      [&](next<LTLPFO>, auto arg) {
        return stepped(arg, k + 1);
      },
      [&](wnext<LTLPFO>, auto arg) {
        return stepped(arg, k + 1);
      },
      [&](prev<LTLPFO>, auto arg) {
        black_assert(k > 0);
        return stepped(arg, k - 1);
      },
      [&](wprev<LTLPFO>, auto arg) {
        black_assert(k > 0);
        return stepped(arg, k - 1);
      },
      [&](negative<LTLPFO>, auto arg) {
        return negative<FO>(stepped(arg, k));
      },
      [&](binary_term<LTLPFO> b, auto left, auto right) {
        return binary_term<FO>(
          b.node_type(), stepped(left, k), stepped(right, k)
        );
      }
    );
  }

  variable encoder::stepped(variable x, size_t k) 
  {  
    if(_xi.is_rigid(x))
      return x;

    variable sx = _sigma->variable(std::pair{x, k});
    if(_xi.sort(x) && !_xi.sort(sx))
      _global_xi->declare(sx, *_xi.sort(x), scope::rigid);

    return sx;
  }

  relation encoder::stepped(relation r, size_t k) 
  {
    if(_xi.is_rigid(r))
      return r;

    relation sr = _sigma->relation(std::pair{r, k});
    if(_xi.signature(r) && !_xi.signature(sr))
      _global_xi->declare(sr, *_xi.signature(r), scope::rigid);

    return sr;
  }

  function encoder::stepped(function f, size_t k)
  {
    if(_xi.is_rigid(f))
      return f;

    function sf = _sigma->function(std::pair{f, k});
    if(_xi.signature(f) && _xi.sort(f) && !_xi.signature(sf))
      _global_xi->declare(
        sf, *_xi.sort(f), *_xi.signature(f), scope::rigid
      );

    return sf;
  }

  proposition encoder::ground(formula<LTLPFO> f, size_t k) {
    return f.sigma()->proposition(std::pair(f,k));
  }

  // Transformation in NNF
  formula<LTLPFO> encoder::to_nnf(formula<LTLPFO> f) {
    if(auto it = _nnf_cache.find(f); it != _nnf_cache.end())
      return it->second;     

    formula nnf = f.match( // LCOV_EXCL_LINE
      [](boolean b) { return b; },
      [](proposition p) { return p; },
      [](atom<LTLPFO> a) { return a; },
      [](equality<LTLPFO> e) { return e; },
      [](comparison<LTLPFO> c) { return c; },
      [&](quantifier<LTLPFO> q) {
        return quantifier<LTLPFO>(q.node_type(), q.decl(), to_nnf(q.matrix()));
      },
      // Push the negation down to literals
      [&](negation<LTLPFO> n) {
        return n.argument().match(
          [&](boolean)             { return n; },
          [&](proposition)         { return n; },
          [&](atom<LTLPFO>)        { return n; },
          [&](equality<LTLPFO>)    { return n; },
          [&](comparison<LTLPFO>)  { return n; },
          [&](quantifier<LTLPFO> q) {
            quantifier<LTLPFO>::type dual = quantifier<LTLPFO>::type::exists{};
            if(q.node_type() == quantifier<LTLPFO>::type::exists{})
              dual = quantifier<LTLPFO>::type::forall{};

            return quantifier<LTLPFO>(dual, q.decl(), to_nnf(!q.matrix()));
          },
          [&](negation<LTLPFO>, auto op) { // special case for double negation
            return to_nnf(op);
          },
          [&](unary<LTLPFO> u) {
            return unary<LTLPFO>(dual(u.node_type()), to_nnf(!u.argument()));
          },
          [&](implication<LTLPFO>, auto left, auto right) {
            return to_nnf(left) && to_nnf(!right);
          },
          [&](iff<LTLPFO>, auto left, auto right) {
            return to_nnf(!implies(left,right)) || to_nnf(!implies(right,left));
          },
          [&](conjunction<LTLPFO> c) {
            return big_or(*f.sigma(), c.operands(), [&](auto op) {
              return to_nnf(!op);
            });
          },
          [&](disjunction<LTLPFO> c) {
            return big_and(*f.sigma(), c.operands(), [&](auto op) {
              return to_nnf(!op);
            });
          },
          [&](binary<LTLPFO> b, auto left, auto right) {
            return binary<LTLPFO>(
                dual(b.node_type()),
                to_nnf(!left), to_nnf(!right)
            );
          }
        );
      },
      // other cases: just recurse down the formula
      [&](unary<LTLPFO> u) {
        return unary<LTLPFO>(u.node_type(), to_nnf(u.argument()));
      },
      [&](implication<LTLPFO>, auto left, auto right) {
        return to_nnf(!left) || to_nnf(right);
      },
      [&](iff<LTLPFO>, auto left, auto right) {
	      return to_nnf(implies(left, right)) && to_nnf(implies(right, left));
      },
      [&](conjunction<LTLPFO> c) {
        return big_and(*f.sigma(), c.operands(), [&](auto op) {
          return to_nnf(op);
        });
      },
      [&](disjunction<LTLPFO> c) {
        return big_or(*f.sigma(), c.operands(), [&](auto op) {
          return to_nnf(op);
        });
      },
      [&](binary<LTLPFO> b) {
        return binary<LTLPFO>(
          b.node_type(), to_nnf(b.left()), to_nnf(b.right())
        );
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
  void encoder::_add_xyz_requests(formula<LTLPFO> f)
  {    
    f.match(
      [&](tomorrow<LTLPFO> t)     { _xrequests.push_back(t);     },
      [&](w_tomorrow<LTLPFO> w)   { _xrequests.push_back(w);     },
      [&](yesterday<LTLPFO> y)    { _yrequests.push_back(y);     },
      [&](w_yesterday<LTLPFO> z)  { _zrequests.push_back(z);     },
      [&](until<LTLPFO> u)        { _xrequests.push_back(X(u));  },
      [&](release<LTLPFO> r)      { _xrequests.push_back(wX(r)); },
      [&](w_until<LTLPFO> r)      { _xrequests.push_back(wX(r)); },
      [&](s_release<LTLPFO> r)    { _xrequests.push_back(X(r));  },
      [&](always<LTLPFO> a)       { _xrequests.push_back(wX(a)); },
      [&](eventually<LTLPFO> e)   { _xrequests.push_back(X(e));  },
      [&](since<LTLPFO> s)        { _yrequests.push_back(Y(s));  },
      [&](once<LTLPFO> o)         { _yrequests.push_back(Y(o));  },
      [&](triggered<LTLPFO> t)    { _zrequests.push_back(Z(t));  },
      [&](historically<LTLPFO> h) { _zrequests.push_back(Z(h));  },
      [](otherwise)       { }
    );

    f.match(
      [](boolean) { },
      [](proposition) { },
      [](quantifier<LTLPFO>) { },
      [](atom<LTLPFO>) { },
      [](equality<LTLPFO>) { },
      [](comparison<LTLPFO>) { },
      [&](unary<LTLPFO>, auto op) {
        _add_xyz_requests(op);
      },
      [&](conjunction<LTLPFO> c) {
        for(auto op : c.operands())
          _add_xyz_requests(op);
      },
      [&](disjunction<LTLPFO> c) {
        for(auto op : c.operands())
          _add_xyz_requests(op);
      },
      [&](binary<LTLPFO>, auto left, auto right) {
        _add_xyz_requests(left);
        _add_xyz_requests(right);
      }
    );
  }
}
