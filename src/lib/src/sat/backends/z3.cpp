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
#include <black/logic/alphabet.hpp>
#include <black/logic/parser.hpp>

#include <z3.h>
#include <tsl/hopscotch_map.h>

#include <limits>
#include <optional>
#include <string>
#include <memory>

BLACK_REGISTER_SAT_BACKEND(z3, {
  black::sat::feature::smt, black::sat::feature::quantifiers
})

namespace black::sat::backends 
{
  inline proposition fresh(formula f) {
    return f.sigma()->prop(f);
  }

  struct z3::_z3_t {
    Z3_context context;
    Z3_solver solver;
    std::optional<Z3_model> model;
    bool solver_upgraded = false;

    tsl::hopscotch_map<formula, Z3_ast> formulas;
    tsl::hopscotch_map<term, Z3_ast> terms;

    Z3_ast to_z3(formula);
    Z3_ast to_z3(term);
    Z3_sort to_z3(sort);
    Z3_ast to_z3_inner(formula);
    Z3_ast to_z3_inner(term);

    Z3_func_decl to_z3_func_decl(
      alphabet *sigma, std::string const&name, unsigned arity, bool is_relation
    );

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

  constexpr bool z3_is_old() {
    using namespace z3_compat;
    using type = decltype(Z3_get_error_msg(nullptr,Z3_error_code{}));
    
    return std::is_same_v<type, void>;
  }

  namespace z3_compat_wrap {
    template<REQUIRES(!z3_is_old())>
    Z3_string Z3_get_error_msg(Z3_error_code) {
      black_unreachable(); // LCOV_EXCL_LINE
    }

    template<REQUIRES(z3_is_old())>
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

  z3::z3() : _data{std::make_unique<_z3_t>()} 
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
    Z3_bool_opt ok = 
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
    if(solver_upgraded)
      return;

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

  Z3_sort z3::_z3_t::to_z3(sort s) {
    switch(s){
      case sort::Int:
        return Z3_mk_int_sort(context);
      case sort::Real:
        return Z3_mk_real_sort(context);
    }
    black_unreachable();
  }

  // TODO: Factor out common logic with mathsat.cpp
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

  Z3_func_decl z3::_z3_t::to_z3_func_decl(
    alphabet *sigma, std::string const&name, unsigned arity, bool is_relation
  ) {
    black_assert(arity > 0);
    black_assert(sigma->domain().has_value());

    Z3_symbol symbol = Z3_mk_string_symbol(context, name.c_str());
    Z3_sort s = to_z3(*sigma->domain());
    Z3_sort bool_s = Z3_mk_bool_sort(context);
    
    std::unique_ptr<Z3_sort[]> domain = std::make_unique<Z3_sort[]>(arity);
    std::fill(domain.get(), domain.get() + arity, s);
    
    return Z3_mk_func_decl(
      context, symbol, arity, domain.get(), is_relation ? bool_s : s
    );
  }

  Z3_ast z3::_z3_t::to_z3_inner(formula f) 
  {
    return f.match(
      [this](boolean b) {
        return b.value() ? Z3_mk_true(context) : Z3_mk_false(context);
      },
      [this](atom a) -> Z3_ast {
        std::vector<Z3_ast> z3_terms;
        for(term t : a.terms())
          z3_terms.push_back(to_z3(t));

        // we know how to encode known relations
        if(auto k = a.rel().known_type(); k) {
          black_assert(z3_terms.size() == 2);
          switch(*k) {
            case relation::equal:
              return Z3_mk_eq(context, z3_terms[0], z3_terms[1]);
            case relation::not_equal:
              return Z3_mk_distinct(
                context, unsigned(z3_terms.size()), z3_terms.data());
            case relation::less_than:
              return Z3_mk_lt(context, z3_terms[0], z3_terms[1]);
            case relation::less_than_equal: 
              return Z3_mk_le(context, z3_terms[0], z3_terms[1]);
            case relation::greater_than:
              return Z3_mk_gt(context, z3_terms[0], z3_terms[1]);
            case relation::greater_than_equal:
              return Z3_mk_ge(context, z3_terms[0], z3_terms[1]);
          }
        }

        // Otherwise we go for uninterpreted relations
        Z3_func_decl rel = 
          to_z3_func_decl(
            a.sigma(), a.rel().name(), unsigned(z3_terms.size()), true);
        
        return 
          Z3_mk_app(context, rel, unsigned(z3_terms.size()), z3_terms.data());
      },
      [this](quantifier q) {
        Z3_app var = Z3_to_app(context, to_z3(q.var()));
        bool forall = q.quantifier_type() == quantifier::type::forall;
        if(forall)
          upgrade_solver();

        return Z3_mk_quantifier_const(
          context, forall, 0, 1, &var, 0, nullptr, to_z3(q.matrix())
        );
      },
      [this](proposition p) {
        Z3_sort sort = Z3_mk_bool_sort(context);
        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(p.unique_id()).c_str());
        
        return Z3_mk_const(context, symbol, sort);
      },
      [this](negation, formula n) {
        return Z3_mk_not(context, to_z3(n));
      },
      [this](big_conjunction c) {
        std::vector<Z3_ast> args;
        for(formula op : c.operands())
          args.push_back(to_z3(op));
        
        black_assert(args.size() <= std::numeric_limits<unsigned int>::max());

        return Z3_mk_and(context, 
          static_cast<unsigned int>(args.size()), args.data());
      },
      [this](big_disjunction c) {
        std::vector<Z3_ast> args;
        for(formula op : c.operands())
          args.push_back(to_z3(op));
        
        black_assert(args.size() <= std::numeric_limits<unsigned int>::max());

        return Z3_mk_or(context, 
          static_cast<unsigned int>(args.size()), args.data());
      },
      [this](implication, formula left, formula right) {
        return Z3_mk_implies(context, to_z3(left), to_z3(right));
      },
      [this](iff, formula left, formula right) {
        return Z3_mk_iff(context, to_z3(left), to_z3(right));
      },
      [](temporal) -> Z3_ast { // LCOV_EXCL_LINE
        black_unreachable(); // LCOV_EXCL_LINE
      }
    );
  }

  //
  // Thanks to Leonardo Taglialegne
  //
  static std::pair<int, int> double_to_fraction(double n) {
    uint64_t a = (uint64_t)floor(n), b = 1;
    uint64_t c = (uint64_t)ceil(n), d = 1;

    uint64_t num = 1;
    uint64_t denum = 1;
    while(
      a + c <= (uint64_t)std::numeric_limits<int>::max() &&
      b + d <= (uint64_t)std::numeric_limits<int>::max() &&
      ((double)num/(double)denum != n)
    ) {
      num = a + c;
      denum = b + d;

      if((double)num/(double)denum > n) {
        c = num;
        d = denum;
      } else {
        a = num;
        b = denum;
      }
    }

    return {static_cast<int>(num), static_cast<int>(denum)};
  }

  Z3_ast z3::_z3_t::to_z3_inner(term t) {
    return t.match(
      [&](constant c) {
        if(std::holds_alternative<int64_t>(c.value())) {
          black_assert(c.sigma()->domain().has_value());

          Z3_ast number = Z3_mk_int64(
            context, std::get<int64_t>(c.value()), Z3_mk_int_sort(context)
          );

          if(c.sigma()->domain() == sort::Int)
            return number;
          return Z3_mk_int2real(context, number);
        } else {
          auto [num,denum] = double_to_fraction(std::get<double>(c.value()));
          return Z3_mk_real(context, num, denum);
        }
      },
      [&](variable v) {
        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(v.unique_id()).c_str());

        std::optional<sort> s = t.sigma()->domain();
        black_assert(s.has_value());
        return Z3_mk_const(context, symbol, to_z3(*s));
      },
      [&](application a) { 
        black_assert(a.sigma()->domain().has_value());
        std::vector<Z3_ast> z3_terms;
        for(term t2 : a.arguments())
          z3_terms.push_back(to_z3(t2));
        
        // We know how to encode known functions
        if(auto k = a.func().known_type(); k) {
          switch(*k) {
            case function::negation:
              black_assert(z3_terms.size() == 1);
              return Z3_mk_unary_minus(context, z3_terms[0]);
            case function::subtraction:
              return 
                Z3_mk_sub(context, unsigned(z3_terms.size()), z3_terms.data());
            case function::addition:
              return 
                Z3_mk_add(context, unsigned(z3_terms.size()), z3_terms.data());
            case function::multiplication:
              return 
                Z3_mk_mul(context, unsigned(z3_terms.size()), z3_terms.data());
            case function::division:
              black_assert(z3_terms.size() == 2);
              return Z3_mk_div(context, z3_terms[0], z3_terms[1]);
            case function::modulo:
              black_assert(z3_terms.size() == 2);
              return Z3_mk_mod(context, z3_terms[0], z3_terms[1]);
          }
          black_unreachable();
        }

        // Otherwise we go for uninterpreted functions
        Z3_func_decl func = 
          to_z3_func_decl(
            a.sigma(), a.func().name(), unsigned(z3_terms.size()), false);
        
        return 
          Z3_mk_app(context, func, unsigned(z3_terms.size()), z3_terms.data());
      },
      // We should not have any next(var) term at this point
      [&](next) -> Z3_ast { black_unreachable(); },
      [&](wnext) -> Z3_ast { black_unreachable(); }
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
