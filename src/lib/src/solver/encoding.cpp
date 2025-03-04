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
  formula encoder::prune(size_t k)
  {
    if(_finite)
      return big_or(*_sigma, range(0, k), [&](size_t l) {
        return l_to_k_loop(l, k, false);
      });

    return big_or(*_sigma, range(0, k), [&](size_t l) {
      return big_or(*_sigma, range(l + 1, k), [&](size_t j) {
        return l_to_k_loop(l,j,false) && l_to_k_loop(j,k, false) && 
               l_j_k_prune(l,j,k);
      });
    });
  }


  // Generates the _lPRUNE_j^k encoding
  formula encoder::l_j_k_prune(size_t l, size_t j, size_t k) {
    return big_and(*_sigma, _requests, [&](req_t req) -> formula 
    {
      std::optional<formula> ev = _get_ev(req.target); 
      if(!ev)
        return _sigma->top();

      // Creating the encoding
      formula inner_impl = 
        big_or(*_sigma, range(j + 1, k + 1), [&](size_t i) {
          return to_ground_snf(*ev, i, req.signature);
        });

      formula first_conj = ground(req, k) && inner_impl;
      formula second_conj = 
        big_or(*_sigma, range(l + 1, j + 1), [&](size_t i) {
          return to_ground_snf(*ev, i, req.signature);
        });

      return forall(req.signature, implies(first_conj, second_conj));
    });
  }


  // Generates the encoding for EMPTY_k
  formula encoder::k_empty(size_t k) {
    return big_and(*_sigma, _requests, [&,this](req_t req) -> formula {
      if(req.type == req_t::future) {
        if(!_finite || req.strength == req_t::strong)
          return forall(req.signature, !ground(req, k));
        else if(req.strength == req_t::weak)
          return forall(req.signature, ground(req, k));
      }
      return _sigma->top();
    }) && !not_last_prop(k);
  }

  // extract the requested formula from an eventuality
  std::optional<formula> encoder::_get_ev(formula f) {
    return f.match(
      [](eventually e) { return std::optional{e.argument()}; },
      [](until u) { return std::optional{u.right()}; },
      [](otherwise) { return std::nullopt; }
    );
  }

  proposition encoder::loop_prop(alphabet *sigma, size_t l, size_t k) {
    return sigma->proposition(std::tuple{"_loop_prop"sv, l, k});
  }

  // Generates the encoding for LOOP_k
  // This is modified to allow the extraction of the loop index when printing
  // the model of the formula
  formula encoder::k_loop(size_t k) {
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
  formula encoder::l_to_k_period(size_t l, size_t k) {

    return big_and(*_sigma, _requests, [&](req_t req) -> formula {
      std::optional<formula> ev = _get_ev(req.target);
      if(!ev)
        return _sigma->top();
      
      // Creating the encoding
      formula proposition_phi_k = ground(req, k);
      formula body_impl = 
        big_or(*_sigma, range(l + 1, k + 1), [&](size_t i) {
          return to_ground_snf(*ev, i, req.signature);
        });

      return forall(req.signature, implies(proposition_phi_k, body_impl));
    });
  }


  // Generates the encoding for _lR_k
  formula encoder::l_to_k_loop(size_t l, size_t k, bool close_yesterdays) {
    return big_and(*_sigma, _requests, [&](req_t req) {
      formula f = forall(req.signature,
        iff( ground(req, l), ground(req, k) )
      );
      if(req.type == req_t::past && close_yesterdays)
        f = f && forall(req.signature,
          iff( ground(req, l+1), to_ground_snf(req.target, k, req.signature) )
        );

      return f;
    });
  }


  // Generates the k-unraveling step for the given k.
  formula encoder::k_unraveling(size_t k) {
    if (k == 0) {
      auto init = big_and(*_sigma, _requests, [&](req_t req) -> formula {
        if(req.type == req_t::past) {
          switch(req.strength) {
            case req_t::strong:
              return forall(req.signature, !ground(req, k));
            case req_t::weak:
              return forall(req.signature, ground(req, k));
          }
        }
        return _sigma->top();
      });

      return to_ground_snf(_frm, k, {}) && !not_first_prop(0) && init;
    }

    auto reqs = big_and(*_sigma, _requests, [&](req_t req) {
      nest_scope_t next{_xi};

      for(auto d : req.signature)
        _xi.declare(d, scope::rigid);
      
      switch(req.type) {
        case req_t::future:
          return forall(req.signature,
            iff(ground(req, k - 1), to_ground_snf(req.target, k, req.signature))
          );
        case req_t::past:
          return forall(req.signature,
            iff(ground(req, k), to_ground_snf(req.target, k - 1, req.signature))
          );
      }
      black_unreachable();
    });

    auto lookaheads = big_and(*_sigma, _lookaheads, [&](lookahead_t lh) {
      switch(lh.type) {
        case req_t::future:
          return ground(lh, k - 1) == stepped(lh.target, k);
        case req_t::past:
          return ground(lh, k) == stepped(lh.target, k - 1);
      }
      black_unreachable();
    });

    return not_last_prop(k - 1) && not_first_prop(k) && reqs && lookaheads;
  }
  
  // Turns the current formula into Stepped Normal Form
  // Note: this has to be run *after* the transformation to NNF (to_nnf() below)
  formula encoder::to_ground_snf(
    formula f, size_t k, std::vector<var_decl> env
  ) {
    return f.match( // LCOV_EXCL_LINE
      [&](boolean b)      { return b; },
      [&](atom a) -> formula {
        return wrapped(a, k);
      }, // LCOV_EXCL_LINE
      [&](equality e) -> formula {
        return wrapped(e, k);
      },
      [&](comparison c) -> formula {
        return wrapped(c, k);
      },
      [&](quantifier q) {
        nest_scope_t nest{_xi};
        
        for(auto decl : q.variables())
          _xi.declare(decl, scope::rigid);

        env.insert(env.end(), q.variables().begin(), q.variables().end());

        auto result = quantifier(
          q.node_type(), q.variables(), to_ground_snf(q.matrix(), k, env)
        );

        return result;
      }, // LCOV_EXCL_LINE
      [&](proposition p)  { return stepped(p, k); },
      [&](negation n) { 
        return !to_ground_snf(n.argument(),k, env); 
      },
      [&](tomorrow t) -> formula { 
        return ground(mk_req(t, env), k);
      },
      [&](w_tomorrow t) -> formula { 
        return ground(mk_req(t, env), k);
      },
      [&](yesterday y) -> formula { 
        return ground(mk_req(y, env), k);
      },
      [&](w_yesterday y) -> formula { 
        return ground(mk_req(y, env), k);
      },
      [&](conjunction c) {
        return big_and(*f.sigma(), operands(c), [&](auto op) {
          return to_ground_snf(op, k, env);
        });
      },
      [&](disjunction c) {
        return big_or(*f.sigma(), operands(c), [&](auto op) {
          return to_ground_snf(op, k, env);
        });
      },
      [&](until u, auto left, auto right) {
        return to_ground_snf(right || (left && X(u)), k, env);
      },
      [&](w_until w, auto left, auto right) {
        return to_ground_snf(right || (left && wX(w)), k, env);
      },
      [&](eventually e, auto op) {
        return to_ground_snf(op || X(e), k, env);
      },
      [&](always a, auto op) {
        return to_ground_snf(op && wX(a), k, env);
      },
      [&](release r, auto left, auto right) {
        return to_ground_snf((left && right) || (right && wX(r)), k, env);
      },
      [&](s_release r, auto left, auto right) {
        return to_ground_snf((left && right) || (right && X(r)), k, env);
      },
      [&](since s, auto left, auto right) {
        return to_ground_snf(right || (left && Y(s)), k, env);
      },
      [&](triggered t, auto left, auto right) {
        return to_ground_snf((left && right) || (right && Z(t)), k, env);
      },
      [&](once o, auto op) {
        return to_ground_snf(op || Y(o), k, env);
      },
      [&](historically h, auto op) {
        return to_ground_snf(op && Z(h), k, env);
      },
      [&](implication) -> formula { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE 
      },
      [&](iff) -> formula { // LCOV_EXCL_LINE 
        black_unreachable(); // LCOV_EXCL_LINE
      }
    );
  }

  // Duals for temporal operators used in to_nnf()
  static unary::type dual(unary::type t) {
    switch(t) {
      case unary::type::negation:
        return unary::type::negation;
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
  }

  static binary::type dual(binary::type t)
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
      case binary::type::conjunction:
      case binary::type::disjunction:
      case binary::type::implication:
      case binary::type::iff:
        black_unreachable(); // LCOV_EXCL_LINE
    }
  }

  term
  encoder::stepped(term t, size_t k) 
  {
    return t.match( // LCOV_EXCL_LINE
      [](constant c) { return *c.to<constant>(); },
      [&](variable x) {
        return stepped(x, k);
      },
      [&](application a) {
        std::vector<term> terms;
        for(term ti : a.terms())
          terms.push_back(stepped(ti, k));
        
        return application(stepped(a.func(), k), terms);
      }, // LCOV_EXCL_LINE
      [&](to_integer, auto arg) {
        return to_integer(stepped(arg, k));
      },
      [&](to_real, auto arg) {
        return to_real(stepped(arg, k));
      },
      [&](next, term arg) {
        return ground(lookahead_t{
          *arg.to<variable>(), req_t::future, req_t::strong
        }, k);
      },
      [&](wnext, term arg) {
        return ground(lookahead_t{
          *arg.to<variable>(), req_t::future, req_t::weak
        }, k);
      },
      [&](prev, term arg) {
        return ground(lookahead_t{
          *arg.to<variable>(), req_t::past, req_t::strong
        }, k);
      },
      [&](wprev, term arg) {
        return ground(lookahead_t{
          *arg.to<variable>(), req_t::past, req_t::weak
        }, k);
      },
      [&](negative, auto arg) {
        return negative(stepped(arg, k));
      },
      [&](binary_term b, auto left, auto right) {
        return binary_term(
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

  atom encoder::stepped(atom a, size_t k) {
    std::vector<term> terms;
    for(term t : a.terms())
      terms.push_back(stepped(t, k));

    relation stepped_rel = stepped(a.rel(), k);
    return atom(stepped_rel, terms);
  }

  equality encoder::stepped(equality e, size_t k) {
    std::vector<term> stepterms;
    for(auto t : e.terms())
      stepterms.push_back(stepped(t, k));
    
    return equality(e.node_type(), stepterms);
  }

  comparison encoder::stepped(comparison c, size_t k) {
    term stepleft = stepped(c.left(), k);
    term stepright = stepped(c.right(), k);

    return comparison(c.node_type(), stepleft, stepright);
  }


  proposition encoder::stepped(proposition p, size_t k) {
    return p.sigma()->proposition(std::pair{formula{p}, k});
  }

  proposition encoder::not_last_prop(size_t k) {
    return stepped(_sigma->proposition("__not_last"sv), k);
  }
  
  proposition encoder::not_first_prop(size_t k) {
    return stepped(_sigma->proposition("__not_first"sv), k);
  }

  formula encoder::wrapped(formula f, size_t k) {

    auto s = f.match(
      [&](atom a) { return stepped(a, k); },
      [&](equality e) { return stepped(e, k); },
      [&](comparison c) { return stepped(c, k); },
      [](otherwise) -> formula { black_unreachable(); }
    );

    auto [future, past] = strength(f);
    
    if(future == req_t::strong)
      s = not_last_prop(k) && s;
    if(future == req_t::weak)
      s = implies(not_last_prop(k), s);
    
    if(past == req_t::strong)
      s = not_first_prop(k) && s;
    if(past == req_t::weak)
      s = implies(not_first_prop(k), s);

    return s;
  }

  variable encoder::ground(lookahead_t lh, size_t k) {
    std::string_view sv = 
      lh.type == req_t::future ? 
        (lh.strength == req_t::weak ? "__wnext" : "__next") :
        (lh.strength == req_t::weak ? "__wprev" : "__prev");

    variable g = _sigma->variable(std::tuple{sv, lh.target, k});
    if(_xi.sort(lh.target) && !_xi.sort(g))
      _global_xi->declare(g, *_xi.sort(lh.target), scope::rigid);

    return g;
  }

  formula encoder::ground(req_t req, size_t k) {
    if(req.signature.empty())
      return req.target.sigma()->proposition(std::pair{req, k});
    
    auto rel = req.target.sigma()->relation(std::pair{req, k});
    if(!_xi.signature(rel))
      _global_xi->declare(rel, req.signature, scope::rigid);
    
    return rel(req.signature);
  }

  formula encoder::forall(std::vector<var_decl> env, formula f) {
    if(env.empty())
      return f;
    
    return logic::forall(env, f);
  }

  // Transformation in NNF
  formula encoder::to_nnf(formula f) {
    if(auto it = _nnf_cache.find(f); it != _nnf_cache.end())
      return it->second;     

    formula nnf = f.match( // LCOV_EXCL_LINE
      [](boolean b) { return b; },
      [](proposition p) { return p; },
      [](atom a) { return a; },
      [](equality e) { return e; },
      [](comparison c) { return c; },
      [&](quantifier q) {
        return quantifier(
          q.node_type(), q.variables(), to_nnf(q.matrix())
        );
      },
      // Push the negation down to literals
      [&](negation n) {
        return n.argument().match(
          [&](boolean)             { return n; },
          [&](proposition)         { return n; },
          [&](atom)        { return n; },
          [&](equality)    { return n; },
          [&](comparison)  { return n; },
          [&](quantifier q) {
            quantifier::type dual = quantifier::type::exists;
            if(q.node_type() == quantifier::type::exists)
              dual = quantifier::type::forall;

            return quantifier(dual, q.variables(), to_nnf(!q.matrix()));
          },
          [&](negation, auto op) { // special case for double negation
            return to_nnf(op);
          },
          [&](unary u) {
            return unary(dual(u.node_type()), to_nnf(!u.argument()));
          },
          [&](implication, auto left, auto right) {
            return to_nnf(left) && to_nnf(!right);
          },
          [&](iff, auto left, auto right) {
            return to_nnf(!implies(left,right)) || to_nnf(!implies(right,left));
          },
          [&](conjunction c) {
            return big_or(*f.sigma(), operands(c), [&](auto op) {
              return to_nnf(!op);
            });
          },
          [&](disjunction c) {
            return big_and(*f.sigma(), operands(c), [&](auto op) {
              return to_nnf(!op);
            });
          },
          [&](binary b, auto left, auto right) {
            return binary(
                dual(b.node_type()),
                to_nnf(!left), to_nnf(!right)
            );
          }
        );
      },
      // other cases: just recurse down the formula
      [&](unary u) {
        return unary(u.node_type(), to_nnf(u.argument()));
      },
      [&](implication, auto left, auto right) {
        return to_nnf(!left) || to_nnf(right);
      },
      [&](iff, auto left, auto right) {
	      return to_nnf(implies(left, right)) && to_nnf(implies(right, left));
      },
      [&](conjunction c) {
        return big_and(*f.sigma(), operands(c), [&](auto op) {
          return to_nnf(op);
        });
      },
      [&](disjunction c) {
        return big_or(*f.sigma(), operands(c), [&](auto op) {
          return to_nnf(op);
        });
      },
      [&](binary b) {
        return binary(
          b.node_type(), to_nnf(b.left()), to_nnf(b.right())
        );
      }
    );

    _nnf_cache.insert({f, nnf});
    return nnf;
  }

  encoder::formula_strength_t encoder::strength(formula f) {
    std::optional<req_t::strength_t> future;
    std::optional<req_t::strength_t> past;

    if(has_any_element_of(f, syntax_element::next))
      future = req_t::strong;
    else if(has_any_element_of(f, syntax_element::wnext))
      future = req_t::weak;

    if(has_any_element_of(f, syntax_element::prev))
      past = req_t::strong;
    else if(has_any_element_of(f, syntax_element::wprev))
      past = req_t::weak;

    return {future, past};
  }

  req_t encoder::mk_req(tomorrow f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::future, req_t::strong};
  }

  req_t encoder::mk_req(w_tomorrow f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::future, req_t::weak};
  }

  req_t encoder::mk_req(yesterday f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::past, req_t::strong};
  }

  req_t encoder::mk_req(w_yesterday f, std::vector<var_decl> env) {
    return req_t{f.argument(), env, req_t::past, req_t::weak};
  }

  formula to_formula(req_t req) {
    switch(req.type) {
      case req_t::future:
        if(req.strength == req_t::strong)
          return X(req.target);
        return wX(req.target);
      case req_t::past:
        if(req.strength == req_t::strong)
          return Y(req.target);
        return Z(req.target);
    }
    black_unreachable();
  }

  void encoder::_collect_requests(formula f, std::vector<var_decl> env)
  { 
    std::optional<req_t> req;
    f.match(
      [&](tomorrow t)      { req = mk_req(t, env);     },
      [&](w_tomorrow t)    { req = mk_req(t, env);     },
      [&](yesterday y)     { req = mk_req(y, env);     },
      [&](w_yesterday y)   { req = mk_req(y, env);     },
      [&](until u)         { req = mk_req(X(u), env);  },
      [&](release r)       { req = mk_req(wX(r), env); },
      [&](w_until r)       { req = mk_req(wX(r), env); },
      [&](s_release r)     { req = mk_req(X(r), env);  },
      [&](always a)        { req = mk_req(wX(a), env); },
      [&](eventually e)    { req = mk_req(X(e), env);  },
      [&](since s)         { req = mk_req(Y(s), env);  },
      [&](once o)          { req = mk_req(Y(o), env);  },
      [&](triggered t)     { req = mk_req(Z(t), env);  },
      [&](historically h)  { req = mk_req(Z(h), env);  },
      [&](atom, auto terms) {
        for(auto t : terms) 
          _collect_lookaheads(t);
      },
      [&](equality, auto terms) {
        for(auto t : terms) 
          _collect_lookaheads(t);
      },
      [&](comparison, auto left, auto right) {
        _collect_lookaheads(left);
        _collect_lookaheads(right);
      },
      [](otherwise) { }
    );

    if(req)
      _requests.push_back(*req);

    f.match(
      [&](quantifier, auto vars, auto matrix) { 
        env.insert(env.end(), vars.begin(), vars.end());
        _collect_requests(matrix, env);
      },
      [&](unary, auto op) {
        _collect_requests(op, env);
      },
      [&](conjunction c) {
        for(auto op : operands(c))
          _collect_requests(op, env);
      },
      [&](disjunction c) {
        for(auto op : operands(c))
          _collect_requests(op, env);
      },
      [&](binary, auto left, auto right) {
        _collect_requests(left, env);
        _collect_requests(right, env);
      },
      [](otherwise) { }
    );
  }

  void encoder::_collect_lookaheads(term t) {
    
    std::optional<lookahead_t> lh;
    t.match(
      [&](next, term arg) { 
        lh = lookahead_t{*arg.to<variable>(), req_t::future, req_t::strong}; 
      },
      [&](wnext, term arg) { 
        lh = lookahead_t{*arg.to<variable>(), req_t::future, req_t::weak}; 
      },
      [&](prev, term arg) { 
        lh = lookahead_t{*arg.to<variable>(), req_t::past, req_t::strong}; 
      },
      [&](wprev, term arg) { 
        lh = lookahead_t{*arg.to<variable>(), req_t::past, req_t::weak}; 
      },
      [](otherwise) { }
    );

    if(lh)
      _lookaheads.push_back(*lh);

    for_each_child(t, [&](auto child) {
      child.match(
        [&](term c) {
          _collect_lookaheads(c);
        },
        [](otherwise) { }
      );
    });
  }
}
