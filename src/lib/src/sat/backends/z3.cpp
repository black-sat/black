//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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


#include <black/sat/backends/z3.hpp>
#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/logic/parser.hpp>

#include <black/sat/backends/fractionals.hpp>

#include <z3.h>
#include <tsl/hopscotch_map.h>

#include <limits>
#include <optional>
#include <string>
#include <memory>

BLACK_REGISTER_SAT_BACKEND(z3, {
  black::sat::feature::smt, black::sat::feature::quantifiers
})

namespace black_internal::z3
{
  inline proposition fresh(formula f) {
    return f.sigma()->proposition(f);
  }

  struct z3::_z3_t {
    logic::scope xi;

    _z3_t(logic::scope const& _xi) : xi{chain(_xi)} { }

    Z3_context context;
    Z3_solver solver;
    std::optional<Z3_model> model;
    bool solver_upgraded = false;

    tsl::hopscotch_map<formula, Z3_ast> formulas;
    tsl::hopscotch_map<term, Z3_ast> terms;

    Z3_func_decl to_z3(function);
    Z3_func_decl to_z3(relation);
    Z3_ast to_z3(var_decl);
    Z3_ast to_z3(formula);
    Z3_ast to_z3(term);
    Z3_sort to_z3(std::optional<sort>);
    Z3_ast to_z3_inner(formula);
    Z3_ast to_z3_inner(term);

    void upgrade_solver();
  };

  //
  // This trick, up to error_handler(), is needed to support Z3 4.4 
  // (the version shipped with Debian stable), which has a different API for
  // Z3_get_error_msg() w.r.t to the latest (4.8) version
  //
  namespace z3_compat {
    template<typename T = int>
    void Z3_get_error_msg(Z3_context, Z3_error_code, T = T{}) { }
  }

  template<typename T = void>
  constexpr bool z3_is_old() {
    using namespace z3_compat;
    using type = decltype(Z3_get_error_msg(nullptr,Z3_error_code{}));
    
    return std::is_same_v<type, void>;
  }

  namespace z3_compat_wrap {
    template<typename T = void>
      requires (!z3_is_old<T>())
    Z3_string Z3_get_error_msg(Z3_error_code) {
      black_unreachable(); // LCOV_EXCL_LINE
    }

    template<typename T = void>
      requires (z3_is_old<T>())
    Z3_string Z3_get_error_msg(Z3_context, Z3_error_code) {
      black_unreachable(); // LCOV_EXCL_LINE
    }
  }
  
  [[noreturn]]
  static void error_handler(Z3_context c, Z3_error_code e) { // LCOV_EXCL_LINE
    using namespace z3_compat_wrap;
    if constexpr (z3_is_old())
      fprintf(stderr, "Z3 error: %s\n", Z3_get_error_msg(e)); // LCOV_EXCL_LINE
    else
      fprintf(stderr, "Z3 error: %s\n", Z3_get_error_msg(c, e)); // LCOV_EXCL_LINE
    std::abort(); // LCOV_EXCL_LINE
  }

  // end trick

  z3::z3(class scope const& xi) 
    : _data{std::make_unique<_z3_t>(xi)} 
  { 
    Z3_config  cfg;
    
    cfg = Z3_mk_config();
    Z3_set_param_value(cfg, "model", "true");
    
    _data->context = Z3_mk_context(cfg);
    Z3_set_error_handler(_data->context, error_handler);

    Z3_del_config(cfg);

    _data->solver = Z3_mk_solver(_data->context);
    Z3_solver_inc_ref(_data->context, _data->solver);
  }

  z3::~z3() {
    Z3_solver_dec_ref(_data->context, _data->solver);
    Z3_del_context(_data->context);
  }

  void z3::assert_formula(formula f) { 
    Z3_ast ast = _data->to_z3(f); // this call must stay on its own line
    Z3_solver_assert(_data->context, _data->solver, ast);
  }
  
  tribool z3::is_sat_with(formula f) {
    Z3_solver_push(_data->context, _data->solver);
    assert_formula(iff(fresh(f), f));
    Z3_ast term = _data->to_z3(fresh(f));
    
    Z3_lbool res = 
      Z3_solver_check_assumptions(_data->context, _data->solver, 1, &term);

    if(res == Z3_L_TRUE) {
      if(_data->model)
        Z3_model_dec_ref(_data->context, *_data->model);
      
      _data->model = Z3_solver_get_model(_data->context, _data->solver);
    }

    Z3_solver_pop(_data->context, _data->solver, 1);

    return res == Z3_L_TRUE ? tribool{true} :
           res == Z3_L_FALSE ? tribool{false} :
           tribool::undef;
  }

  tribool z3::is_sat() {
    Z3_lbool res = Z3_solver_check(_data->context, _data->solver);

    if(res == Z3_L_TRUE) {
      if(_data->model)
        Z3_model_dec_ref(_data->context, *_data->model);
      
      _data->model = Z3_solver_get_model(_data->context, _data->solver);
      Z3_model_inc_ref(_data->context, *_data->model);
    }

    return res == Z3_L_TRUE ? tribool{true} :
           res == Z3_L_FALSE ? tribool{false} :
           tribool::undef;
  }

  tribool z3::value(proposition a) const {
    if(!_data->model)
      return tribool::undef;
    
    auto it = _data->formulas.find(a);
    if(it == _data->formulas.end())
      return tribool::undef;
    
    Z3_ast term = it->second;
    Z3_ast res;
    
    [[maybe_unused]] 
    auto ok = 
      Z3_model_eval(_data->context, *_data->model, term, false, &res);
    black_assert(ok);

    Z3_lbool lres = Z3_get_bool_value(_data->context, res);
    
    tribool result = 
      lres == Z3_L_TRUE ? tribool{true} :
      lres == Z3_L_FALSE ? tribool{false} : tribool::undef;

    return result;
  }

  void z3::clear() { 
    Z3_solver_reset(_data->context, _data->solver);
  }

  void z3::_z3_t::upgrade_solver() {
    // gcov false negative
    if(solver_upgraded)
      return; // LCOV_EXCL_LINE

    Z3_tactic qe = Z3_mk_tactic(context, "qe");
    Z3_tactic_inc_ref(context, qe);
    Z3_tactic smt = Z3_mk_tactic(context, "smt");
    Z3_tactic_inc_ref(context, smt);

    Z3_tactic qe_smt = Z3_tactic_and_then(context, qe, smt);
    Z3_tactic_inc_ref(context, qe_smt);

    Z3_solver new_solver = Z3_mk_solver_from_tactic(context, qe_smt);
    Z3_solver_inc_ref(context, new_solver);

    Z3_ast_vector assertions = Z3_solver_get_assertions(context, solver);

    for(unsigned i = 0; i < Z3_ast_vector_size(context, assertions); ++i) {
      Z3_ast assertion = Z3_ast_vector_get(context, assertions, i);
      Z3_solver_assert(context, new_solver, assertion);
    }

    Z3_solver_dec_ref(context, solver);
    solver = new_solver;
    solver_upgraded = true;
  }

  Z3_sort z3::_z3_t::to_z3(std::optional<sort> o) {
    black_assert(o.has_value());

    sort s = *o;

    return s.match(
      [&](integer_sort) {
        return Z3_mk_int_sort(context);
      },
      [&](real_sort) {
        return Z3_mk_real_sort(context);
      },
      [&](named_sort, auto name) {
        return Z3_mk_uninterpreted_sort(
          context, 
          Z3_mk_string_symbol(context, to_string(name).c_str())
        );
      }
    );
  }

  
  Z3_ast z3::_z3_t::to_z3(formula f) 
  {
    if(auto it = formulas.find(f); it != formulas.end()) 
      return it->second;

    Z3_ast z3_f = to_z3_inner(f);
    formulas.insert({f, z3_f});

    return z3_f;
  }

  Z3_ast z3::_z3_t::to_z3(term t) {
    if(auto it = terms.find(t); it != terms.end()) 
      return it->second;

    Z3_ast z3_t = to_z3_inner(t);
    terms.insert({t, z3_t});

    return z3_t;
  }

  Z3_func_decl z3::_z3_t::to_z3(function f) {
    Z3_symbol symbol = 
      Z3_mk_string_symbol(context, to_string(f.name()).c_str());
    
    Z3_sort result = to_z3(xi.sort(f));
    
    auto signature = xi.signature(f);
    black_assert(signature.has_value());

    unsigned arity = unsigned(signature->size());
    std::unique_ptr<Z3_sort[]> domain = std::make_unique<Z3_sort[]>(arity);
    for(size_t i = 0; i < arity; ++i) {
      domain[i] = to_z3(signature->at(i));
    }
    
    return Z3_mk_func_decl(context, symbol, arity, domain.get(), result);
  }
  
  Z3_func_decl z3::_z3_t::to_z3(relation r) {
    Z3_symbol symbol = 
      Z3_mk_string_symbol(context, to_string(r.name()).c_str());
    
    Z3_sort bool_s = Z3_mk_bool_sort(context);
    
    auto signature = xi.signature(r);
    black_assert(signature.has_value());

    unsigned arity = unsigned(signature->size());
    std::unique_ptr<Z3_sort[]> domain = std::make_unique<Z3_sort[]>(arity);
    for(size_t i = 0; i < arity; ++i) {
      domain[i] = to_z3(signature->at(i));
    }
    
    return Z3_mk_func_decl(context, symbol, arity, domain.get(), bool_s);
  }

  Z3_ast z3::_z3_t::to_z3(var_decl decl) {
    Z3_sort s = to_z3(decl.sort());

    Z3_symbol symbol = Z3_mk_string_symbol(
      context, to_string(decl.variable().unique_id()).c_str()
    );

    return Z3_mk_const(context, symbol, s);
  }

  Z3_ast z3::_z3_t::to_z3_inner(formula f) 
  {
    return f.match(
      [&](boolean b) {
        return b.value() ? Z3_mk_true(context) : Z3_mk_false(context);
      },
      [&](atom a) -> Z3_ast {
        std::vector<Z3_ast> z3_terms;
        for(term t : a.terms())
          z3_terms.push_back(to_z3(t));

        Z3_func_decl rel = to_z3(a.rel());
        
        return 
          Z3_mk_app(context, rel, unsigned(z3_terms.size()), z3_terms.data());
      }, // LCOV_EXCL_LINE
      [&](equal, auto args) {
        std::vector<Z3_ast> z3_terms;
        for(auto t : args)
          z3_terms.push_back(to_z3(t));
        
        std::vector<Z3_ast> z3_eqs;
        for(size_t i = 1; i < z3_terms.size(); ++i)
          z3_eqs.push_back(Z3_mk_eq(context, z3_terms[i-1], z3_terms[i]));

        return Z3_mk_and(context, unsigned(z3_eqs.size()), z3_eqs.data());
      },
      [&](distinct, auto args) {
        std::vector<Z3_ast> z3_terms;
        for(term t : args) 
          z3_terms.push_back(to_z3(t));

        return Z3_mk_distinct(context, 2, z3_terms.data());
      },
      [&](comparison c, auto left, auto right) {
        // we know how to encode known relations
        return c.match(
          [&](less_than) {
            return Z3_mk_lt(context, to_z3(left), to_z3(right));
          },
          [&](less_than_equal) {
            return Z3_mk_le(context, to_z3(left), to_z3(right));
          },
          [&](greater_than) {
            return Z3_mk_gt(context, to_z3(left), to_z3(right));
          },
          [&](greater_than_equal) {
            return Z3_mk_ge(context, to_z3(left), to_z3(right));
          }
        );
      },
      [&](quantifier q) {
        Z3_app var = Z3_to_app(context, to_z3(q.decl()));
        bool forall = q.node_type() == quantifier::type::forall{};
        if(forall)
          upgrade_solver();

        scope _xi = std::move(xi);
        xi = chain(_xi);
        xi.declare_variable(q.decl());

        auto result = Z3_mk_quantifier_const(
          context, forall, 0, 1, &var, 0, nullptr, to_z3(q.matrix())
        );

        xi = std::move(_xi);

        return result;
      },
      [&](proposition p) {
        Z3_sort sort = Z3_mk_bool_sort(context);
        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(p.unique_id()).c_str());
        
        return Z3_mk_const(context, symbol, sort);
      },
      [&](negation, auto arg) {
        return Z3_mk_not(context, to_z3(arg));
      },
      [&](conjunction c) {
        std::vector<Z3_ast> args;
        for(formula op : c.operands())
          args.push_back(to_z3(op));
        
        black_assert(args.size() <= std::numeric_limits<unsigned int>::max());

        return Z3_mk_and(context, 
          static_cast<unsigned int>(args.size()), args.data());
      }, // LCOV_EXCL_LINE
      [&](disjunction c) {
        std::vector<Z3_ast> args;
        for(formula op : c.operands())
          args.push_back(to_z3(op));
        
        black_assert(args.size() <= std::numeric_limits<unsigned int>::max());

        return Z3_mk_or(context, 
          static_cast<unsigned int>(args.size()), args.data());
      }, // LCOV_EXCL_LINE
      [&](implication, auto left, auto right) {
        return Z3_mk_implies(context, to_z3(left), to_z3(right));
      },
      [&](iff, auto left, auto right) {
        return Z3_mk_iff(context, to_z3(left), to_z3(right));
      }
    );
  }

  Z3_ast z3::_z3_t::to_z3_inner(term t) {
    return t.match(
      [&](constant, auto n) {
        return n.match(
          [&](integer, int64_t value) {
            return Z3_mk_int64(context, value, Z3_mk_int_sort(context));
          },
          [&](real, double value) {
            auto [num,denum] = black_internal::double_to_fraction(value);
            return Z3_mk_real(context, num, denum);
          }
        );
      },
      [&](variable v) {
        Z3_sort s = to_z3(xi.sort(v));

        // // to_z3 might have added the variable from a finite sort declaration
        // if(auto it = terms.find(t); it != terms.end()) 
        //   return it->second;

        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(v.unique_id()).c_str());

        return Z3_mk_const(context, symbol, s);
      },
      [&](application a) { 
        std::vector<Z3_ast> z3_terms;
        for(term t2 : a.terms())
          z3_terms.push_back(to_z3(t2));
        
        // Otherwise we go for uninterpreted functions
        Z3_func_decl func = to_z3(a.func());
        
        return 
          Z3_mk_app(context, func, unsigned(z3_terms.size()), z3_terms.data());
      },
      [&](unary_term u) {
        return u.match(
          [&](negative, auto arg) {
            return Z3_mk_unary_minus(context, to_z3(arg));
          },
          [&](to_integer, auto arg) {
            return Z3_mk_real2int(context, to_z3(arg));
          },
          [&](to_real, auto arg) {
            return Z3_mk_int2real(context, to_z3(arg));
          }
        );
      },
      [&](binary_term b) {
        return b.match(
          [&](subtraction, auto left, auto right) {
            Z3_ast z3_terms[2] = { to_z3(left), to_z3(right) };
            return Z3_mk_sub(context, 2, z3_terms);
          },
          [&](addition a) {
            std::vector<Z3_ast> z3_terms;
            for(auto child : a.operands()) {
              z3_terms.push_back(to_z3(child));
            }
            return Z3_mk_add(
              context, unsigned(z3_terms.size()), z3_terms.data()
            );
          },
          [&](multiplication m) {
            std::vector<Z3_ast> z3_terms;
            for(auto child : m.operands()) {
              z3_terms.push_back(to_z3(child));
            }
            return Z3_mk_mul(
              context, unsigned(z3_terms.size()), z3_terms.data()
            );
          },
          [&](division, auto left, auto right) {
            return Z3_mk_div(context, to_z3(left), to_z3(right));
          },
          [&](int_division, auto left, auto right) {
            return Z3_mk_div(context, to_z3(left), to_z3(right));
          }
        );
      }
    );
  }

  std::optional<std::string> z3::license() const {
    return
R"(
Z3
Copyright (c) Microsoft Corporation
All rights reserved. 
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
)";
  }

}
