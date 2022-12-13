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
      formula<FO> init = big_and(*_sigma, _requests, [&](req_t req) {
        if(req.past_strength == req_t::strong)
          return forall(req.signature, !ground(req, k));
        if(req.past_strength == req_t::weak)
          return forall(req.signature, ground(req, k));
      });

      return to_ground_snf(_frm, k) && init;
    }

    return big_and(*_sigma, _requests, [&](req_t req) {
      switch(req.type) {
        case req_t::future:
          return forall(req.signature,
            iff(ground(req, k - 1), to_ground_snf(req.target, k, req.signature))
          );
        case req_t::past:
          return forall(req.signature,
            iff(ground(req, k), to_ground_snf(req.target, k - 1, req.signature))
          );
        case req_t::atom:
          if(k > 1 || req.past_strength == req_t::none)
            return forall(req.signature,
              iff(ground(req, k - 1), stepped(a, k - 1))
            );
      }
      black_unreachable();
    });
  }

  //
  // next(x) = wprev(y)
  //
  // The encoding must be equivalent to the encoding of this decomposition:
  //
  // x' = next(x) && y' = wprev(y) && x' = y'
  //

  bool encoder::future_strength(formula<LTLPFO> f) {
    if(has_any_element_of(f, syntax_element::next))
      return req_t::strong;
    
    if(has_any_element_of(f, syntax_element::wnext))
      return req_t::weak;

    return req_t::none;
  }

  bool encoder::past_strength(formula<LTLPFO> f) {
    if(has_any_element_of(f, syntax_element::prev))
      return req_t::strong;
    
    if(has_any_element_of(f, syntax_element::wprev))
      return req_t::weak;

    return req_t::none;
  }
  
  bool encoder::has_lookahead(formula<LTLPFO> f) {
    return has_any_element_of(f, 
      syntax_element::next,
      syntax_element::wnext,
      syntax_element::prev,
      syntax_element::wnext
    );
  }

  // Turns the current formula into Stepped Normal Form
  // Note: this has to be run *after* the transformation to NNF (to_nnf() below)
  formula<FO> encoder::to_ground_snf(
    formula<LTLPFO> f, size_t k, std::vector<var_decl> env
  ) {
    return f.match( // LCOV_EXCL_LINE
      [&](boolean b)      { return b; },
      [&](atom<LTLPFO> a) -> formula<FO> {
        auto req = mk_req(a, env);
        if(req)
          return ground(*req, k);
        
        return stepped(a, k);
      }, // LCOV_EXCL_LINE
      [&](equality<LTLPFO> e, auto terms) -> formula<FO> {
        auto req = mk_req(e, env);
        if(req)
          return ground(*req, k);

        return stepped(e, k);
      },
      [&](comparison<LTLPFO> c, auto left, auto right) -> formula<FO> {
        auto req = mk_req(c, env);
        if(req)
          return ground(*req, k);

        return stepped(c, k);
      },
      [&](quantifier<LTLPFO> q) {
        nest_scope_t nest{_xi};
        
        for(auto decl : q.variables())
          _xi.declare(decl, scope::rigid);

        env.insert(env.end(), q.variables().begin(), q.variables().end());

        auto result = quantifier<FO>(
          q.node_type(), q.variables(), to_ground_snf(q.matrix(), k, env)
        );

        return result;
      }, // LCOV_EXCL_LINE
      [&](proposition)  { return stepped(f, k); },
      [&](negation<LTLPFO> n) { 
        return !to_ground_snf(n.argument(),k, env); 
      },
      [&](tomorrow<LTLPFO> t) -> formula<FO> { 
        return ground(mk_req(t, env), k);
      },
      [&](w_tomorrow<LTLPFO> t, auto arg) -> formula<FO> { 
        return ground(mk_req(t, env), k);
      },
      [&](yesterday<LTLPFO> y, auto arg) -> formula<FO> { 
        return ground(mk_req(y, env), k);
      },
      [&](w_yesterday<LTLPFO> y, auto arg) -> formula<FO> { 
        return ground(mk_req(y, env), k);
      },
      [&](conjunction<LTLPFO> c) {
        return big_and(*f.sigma(), c.operands(), [&](auto op) {
          return to_ground_snf(op, k, env);
        });
      },
      [&](disjunction<LTLPFO> c) {
        return big_or(*f.sigma(), c.operands(), [&](auto op) {
          return to_ground_snf(op, k, env);
        });
      },
      [&](until<LTLPFO> u, auto left, auto right) {
        return to_ground_snf(right || (left && X(u)), k, env);
      },
      [&](w_until<LTLPFO> w, auto left, auto right) {
        return to_ground_snf(right || (left && wX(u)), k, env);
      },
      [&](eventually<LTLPFO> e, auto op) {
        return to_ground_snf(op && X(e), k, env);
      },
      [&](always<LTLPFO> a, auto op) {
        return to_ground_snf(op && wX(a), k, env);
      },
      [&](release<LTLPFO> r, auto left, auto right) {
        return to_ground_snf((left && right) || (right && wX(r)), k, env);
      },
      [&](s_release<LTLPFO> r, auto left, auto right) {
        return to_ground_snf((left && right) || (right || X(r)), k, env);
      },
      [&](since<LTLPFO> s, auto left, auto right) {
        return to_ground_snf(right || (left && Y(s)), k, env);
      },
      [&](triggered<LTLPFO> t, auto left, auto right) {
        return to_ground_snf((left && right) || (right && Z(t)), k, env);
      },
      [&](once<LTLPFO> o, auto op) {
        return to_ground_snf(op || Y(o), k, env);
      },
      [&](historically<LTLPFO> h, auto op) {
        return to_ground_snf(op && Z(h));
      },
      [&](implication<LTLPFO>) -> formula<FO> { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE 
      },
      [&](iff<LTLPFO>) -> formula<FO> { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE
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
      [](constant<LTLPFO> c) { return *c.to<constant<FO>>(); },
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

  atom<FO> encoder::stepped(atom<LTLPFO> a, size_t k) {
    std::vector<term<FO>> terms;
    for(term<LTLPFO> t : a.terms())
      terms.push_back(stepped(t, k));

    relation stepped_rel = stepped(a.rel(), k);
    return atom<FO>(stepped_rel, terms);
  }

  equality<FO> encoder::stepped(equality<LTLPFO> e, size_t k) {
    std::vector<term<FO>> stepterms;
    for(auto t : terms)
      stepterms.push_back(stepped(t, k));
    
    return equality(e.node_type(), stepterms);
  }

  comparison<FO> encoder::stepped(comparison<LTLPFO> c, size_t k) {
    term<FO> stepleft = stepped(left, k);
    term<FO> stepright = stepped(right, k);

    return comparison(c.node_type(), stepleft, stepright);
  }


  proposition encoder::stepped(proposition p, size_t k) {
    return f.sigma()->proposition(std::pair{formula<LTLPFO>{p}, k});
  }

  formula<FO> encoder::ground(req_t req, size_t k) {
    if(env.empty())
      return f.sigma()->proposition(std::pair{req, k});
    
    auto rel = f.sigma()->relation(std::pair{req, k});
    if(!xi->signature(rel))
      global_xi->declare(rel, env, scope::rigid);
    
    return rel(env);
  }

  formula<FO> encoder::forall(std::vector<var_decl> env, formula<FO> f) {
    if(env.empty())
      return f;
    
    return logic::forall(env, f);
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
        return quantifier<LTLPFO>(
          q.node_type(), q.variables(), to_nnf(q.matrix())
        );
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

            return quantifier<LTLPFO>(dual, q.variables(), to_nnf(!q.matrix()));
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

  req_t encoder::mk_req(tomorrow<LTLPFO> f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::future, req_t::strong, req_t::none};
  }

  req_t encoder::mk_req(w_tomorrow<LTLPFO> f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::future, req_t::weak, req_t::none};
  }

  req_t encoder::mk_req(yesterday<LTLPFO> f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::past, req_t::none, req_t::strong};
  }

  req_t encoder::mk_req(w_yesterday<LTLPFO> f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::past, req_t::none, req_t::weak};
  }

  std::optional<req_t>
  encoder::mk_req(atom<LTLPFO> f, std::vector<var_decl> env) {
    req_t::strength_t future_w = future_strength(f);
    req_t::strength_t past_w = past_strength(f);

    if(future_w == req_t::none && past_w == req_t::none)
      return std::nullopt;
    
    return req_t{f, env, req_t::atom, future_w, past_w};
  }

  std::optional<req_t>
  encoder::mk_req(equality<LTLPFO> f, std::vector<var_decl> env) {
    req_t::strength_t future_w = future_strength(f);
    req_t::strength_t past_w = past_strength(f);

    if(future_w == req_t::none && past_w == req_t::none)
      return std::nullopt;
    
    return req_t{f, env, req_t::atom, future_w, past_w};
  }

  std::optional<req_t>
  encoder::mk_req(comparison<LTLPFO> f, std::vector<var_decl> env) {
    req_t::strength_t future_w = future_strength(f);
    req_t::strength_t past_w = past_strength(f);

    if(future_w == req_t::none && past_w == req_t::none)
      return std::nullopt;
    
    return req_t{f, env, req_t::atom, future_w, past_w};
  }



  void encoder::_collect_requests(formula<LTLPFO> f, std::vector<var_decl> env)
  { 
    std::optional<req_t> req;
    f.match(
      [&](tomorrow<LTLPFO>)        { req = mk_req(f, env));     },
      [&](w_tomorrow<LTLPFO>)      { req = mk_req(f, env));     },
      [&](yesterday<LTLPFO>)       { req = mk_req(f, env));     },
      [&](w_yesterday<LTLPFO>)     { req = mk_req(f, env));     },
      [&](atom<LTLPFO> a)          { req = mk_req(f, env));     },
      [&](equality<LTLPFO> e)      { req = mk_req(f, env));     },
      [&](comparison<LTLPFO> c)    { req = mk_req(f, env));     },
      [&](until<LTLPFO> u)         { req = mk_req(X(u), env));  },
      [&](release<LTLPFO> r)       { req = mk_req(wX(r), env)); },
      [&](w_until<LTLPFO> r)       { req = mk_req(wX(r), env)); },
      [&](s_release<LTLPFO> r)     { req = mk_req(X(r), env));  },
      [&](always<LTLPFO> a)        { req = mk_req(wX(a), env)); },
      [&](eventually<LTLPFO> e)    { req = mk_req(X(e), env));  },
      [&](since<LTLPFO> s)         { req = mk_req(Y(s), env));  },
      [&](once<LTLPFO> o)          { req = mk_req(Y(o), env));  },
      [&](triggered<LTLPFO> t)     { req = mk_req(Z(t), env));  },
      [&](historically<LTLPFO> h)  { req = mk_req(Z(h), env));  },
      [](otherwise) { }
    );

    if(req)
      _requests.push_back(*req);

    f.match(
      [](quantifier<LTLPFO>, auto vars, auto matrix) { 
        env.insert(env.end(), vars.begin(), vars.end());
        _collect_requests(matrix, env);
      },
      [&](unary<LTLPFO>, auto op) {
        _collect_requests(op, env);
      },
      [&](conjunction<LTLPFO> c) {
        for(auto op : c.operands())
          _collect_requests(op, env);
      },
      [&](disjunction<LTLPFO> c) {
        for(auto op : c.operands())
          _collect_requests(op, env);
      },
      [&](binary<LTLPFO>, auto left, auto right) {
        _collect_requests(left, env);
        _collect_requests(right, env);
      },
      [](otherwise) { }
    );
  }
}
