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
#include <cstring>

BLACK_REGISTER_SAT_BACKEND(z3, {
  black::sat::feature::smt, black::sat::feature::quantifiers
})

namespace black_internal::z3
{
  inline proposition fresh(formula f) {
    return f.sigma()->proposition(f);
  }

  struct z3_sort_record_t {
    Z3_sort sort;
    std::shared_ptr<tsl::hopscotch_map<variable, Z3_ast>> elements;
    
    z3_sort_record_t(Z3_sort s, tsl::hopscotch_map<variable, Z3_ast> e)
      : sort{s}, 
        elements{
          std::make_shared<tsl::hopscotch_map<variable, Z3_ast>>(std::move(e))
        } { }
  };

  struct z3::_z3_t {
    logic::scope global_xi;
    logic::scope xi;

    _z3_t(logic::scope const& _xi) 
      : global_xi{chain(_xi)}, xi{chain(global_xi)} { }

    Z3_context context;
    Z3_solver solver;
    std::optional<Z3_model> model;
    bool solver_upgraded = false;

    tsl::hopscotch_map<proposition, Z3_ast> props;

    Z3_func_decl to_z3(function);
    Z3_func_decl to_z3(relation);
    Z3_ast to_z3(var_decl);
    Z3_ast to_z3(formula);
    Z3_ast to_z3(term);
    z3_sort_record_t to_z3(std::optional<sort>);
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
  
  static void error_handler(Z3_context c, Z3_error_code e) { // LCOV_EXCL_LINE
    using namespace z3_compat_wrap;
    const char *errmsg = nullptr;
    
    if constexpr (z3_is_old())
      errmsg = Z3_get_error_msg(e); // LCOV_EXCL_LINE
    else
      errmsg = Z3_get_error_msg(c, e); // LCOV_EXCL_LINE
    
    if(strcmp(errmsg, "canceled") == 0)
      return;
    
    fprintf(stderr, "Z3 error %d: %s\n", (int)e, errmsg);
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
    // the call to to_ze() must stay on its own line before `Z3_solver_assert`
    // because it might update _data->solver
    Z3_ast ast = _data->to_z3(f); 

    Z3_solver_assert(_data->context, _data->solver, ast);
  }
  
  tribool z3::is_sat_with(formula f) {
    Z3_ast asmptn = _data->to_z3(f);
    
    Z3_lbool res = 
      Z3_solver_check_assumptions(_data->context, _data->solver, 1, &asmptn);

    if(res == Z3_L_TRUE) {
      if(_data->model)
        Z3_model_dec_ref(_data->context, *_data->model);
      
      _data->model = Z3_solver_get_model(_data->context, _data->solver);
    }

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
    
    auto it = _data->props.find(a);
    if(it == _data->props.end())
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

  tribool z3::value(atom a) const {
    if(!_data->model)
      return tribool::undef;

    Z3_ast term = _data->to_z3(a);
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

  tribool z3::value(equality e) const {
    if(!_data->model)
      return tribool::undef;

    Z3_ast term = _data->to_z3(e);
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

  tribool z3::value(comparison c) const {
    if(!_data->model)
      return tribool::undef;

    Z3_ast term = _data->to_z3(c);
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

  void z3::interrupt() {
    Z3_interrupt(_data->context);
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

  z3_sort_record_t z3::_z3_t::to_z3(std::optional<sort> s) {
    black_assert(s.has_value());

    if(auto record = global_xi.data<z3_sort_record_t>(*s); record.has_value()) 
      return *record;

    z3_sort_record_t record = s->match(
      [&](integer_sort) -> z3_sort_record_t {
        return {Z3_mk_int_sort(context), {}};
      },
      [&](real_sort) -> z3_sort_record_t {
        return {Z3_mk_real_sort(context), {}};
      },
      [&](named_sort n) -> z3_sort_record_t {
        auto d = global_xi.domain(n);
        if(!d)
          return {Z3_mk_uninterpreted_sort(
            context, 
            Z3_mk_string_symbol(context, to_string(n.unique_id()).c_str())
          ), {}};

        Z3_symbol sort_name = 
          Z3_mk_string_symbol(context, to_string(n.unique_id()).c_str());
        std::vector<Z3_symbol> names;
        std::vector<Z3_constructor> ctors;
        
        for(auto x : d->elements()) {
          Z3_symbol xname = 
            Z3_mk_string_symbol(context, to_string(x.unique_id()).c_str());
          Z3_symbol xrec = Z3_mk_string_symbol(context, "");

          names.push_back(xname);
          ctors.push_back(
            Z3_mk_constructor(
              context, xname, xrec, 0, nullptr, nullptr, nullptr
            )
          );
        }

        Z3_sort enum_sort = Z3_mk_datatype(
          context, sort_name, unsigned(d->elements().size()), ctors.data()
        );

        tsl::hopscotch_map<variable, Z3_ast> apps;
        for(size_t i = 0; i < d->elements().size(); ++i) {
          Z3_func_decl decl;
          Z3_func_decl tester;
          Z3_query_constructor(context, ctors[i], 0, &decl, &tester, nullptr);
          apps.insert({
            d->elements()[i], Z3_mk_app(context, decl, 0, nullptr)
          });
        }

        return {enum_sort, std::move(apps)};
      }
    );

    global_xi.set_data(*s, record);
    return record;
  }

  Z3_func_decl z3::_z3_t::to_z3(function f) {
    Z3_symbol symbol = 
      Z3_mk_string_symbol(context, to_string(f.name()).c_str());
    
    Z3_sort result = to_z3(xi.sort(f)).sort;
    
    auto signature = xi.signature(f);
    black_assert(signature.has_value());

    unsigned arity = unsigned(signature->size());
    std::unique_ptr<Z3_sort[]> domain = std::make_unique<Z3_sort[]>(arity);
    for(size_t i = 0; i < arity; ++i) {
      domain[i] = to_z3(signature->at(i)).sort;
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
      domain[i] = to_z3(signature->at(i)).sort;
    }
    
    return Z3_mk_func_decl(context, symbol, arity, domain.get(), bool_s);
  }

  Z3_ast z3::_z3_t::to_z3(var_decl decl) {
    Z3_sort s = to_z3(decl.sort()).sort;

    Z3_symbol symbol = Z3_mk_string_symbol(
      context, to_string(decl.variable().unique_id()).c_str()
    );

    return Z3_mk_const(context, symbol, s);
  }

  Z3_ast z3::_z3_t::to_z3(formula f) 
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

        return 
          Z3_mk_distinct(context, unsigned(z3_terms.size()), z3_terms.data());
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
        bool forall = q.node_type() == quantifier::type::forall;
        if(forall)
          upgrade_solver();
        
        nest_scope_t nest{xi};

        std::vector<Z3_app> z3_apps;

        for(auto decl : q.variables()) {
          xi.declare(decl);
          Z3_ast var = to_z3(decl);
          xi.set_data(decl.variable(), var);
          z3_apps.push_back(Z3_to_app(context, var));
        }
        
        auto result = Z3_mk_quantifier_const(
          context, forall, 0, unsigned(z3_apps.size()), 
          z3_apps.data(), 0, nullptr, to_z3(q.matrix())
        );

        return result;
      },
      [&](proposition p) {
        if(auto it = props.find(p); it != props.end())
          return it->second;

        Z3_sort sort = Z3_mk_bool_sort(context);
        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(p.unique_id()).c_str());
        
        Z3_ast result = Z3_mk_const(context, symbol, sort);
        props.insert({p, result});

        return result;
      },
      [&](negation, auto arg) {
        return Z3_mk_not(context, to_z3(arg));
      },
      [&](conjunction c) {
        std::vector<Z3_ast> args;
        for(formula op : operands(c))
          args.push_back(to_z3(op));
        
        black_assert(args.size() <= std::numeric_limits<unsigned int>::max());

        return Z3_mk_and(context, 
          static_cast<unsigned int>(args.size()), args.data());
      }, // LCOV_EXCL_LINE
      [&](disjunction c) {
        std::vector<Z3_ast> args;
        for(formula op : operands(c))
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

  Z3_ast z3::_z3_t::to_z3(term t) {
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
        if(auto var = xi.data<Z3_ast>(v); var.has_value())
          return *var;

        auto vSort = xi.sort(v);
        black_assert(vSort);
        z3_sort_record_t record = to_z3(*vSort);

        // if the sort has an enumerated domain, the variable might be taken
        // from the relevant enum sort
        if(xi.domain(*vSort)) {
          if(auto it = record.elements->find(v); it != record.elements->end()) {
            Z3_ast term = it->second;
            global_xi.set_data(v, term);
            return term;
          }
        }

        // if not, this is a normal variable
        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(v.unique_id()).c_str());

        return Z3_mk_const(context, symbol, record.sort);
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
            for(auto child : operands(a)) {
              z3_terms.push_back(to_z3(child));
            }
            return Z3_mk_add(
              context, unsigned(z3_terms.size()), z3_terms.data()
            );
          },
          [&](multiplication m) {
            std::vector<Z3_ast> z3_terms;
            for(auto child : operands(m)) {
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
