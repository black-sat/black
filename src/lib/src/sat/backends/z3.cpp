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

#include <z3.h>
#include <tsl/hopscotch_map.h>

#include <limits>

BLACK_REGISTER_SAT_BACKEND(z3)

namespace black::sat::backends 
{
  // TODO: generalize with the same function in cnf.cpp
  inline atom fresh(formula f) {
    if(f.is<atom>())
      return *f.to<atom>();
    return f.alphabet()->var(f);
  }

  struct z3::_z3_t {
    Z3_context context;
    Z3_solver solver;

    tsl::hopscotch_map<formula, Z3_ast> terms;

    Z3_ast to_z3(formula);
    Z3_ast to_z3_inner(formula);
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
      black_unreachable();
    }

    template<REQUIRES(z3_is_old())>
    Z3_string Z3_get_error_msg(Z3_context, Z3_error_code) {
      black_unreachable();
    }
  }
  
  [[noreturn]]
  static void error_handler(Z3_context c, Z3_error_code e) {
    using namespace z3_compat_wrap;
    if constexpr (z3_is_old())
      fprintf(stderr, "Z3 error: %s\n", Z3_get_error_msg(e));
    else
      fprintf(stderr, "Z3 error: %s\n", Z3_get_error_msg(c, e));
    std::abort();
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
    Z3_solver_assert(_data->context, _data->solver, _data->to_z3(f));
  }
  
  bool z3::is_sat_with(formula f) {
    Z3_solver_push(_data->context, _data->solver);
    assert_formula(iff(fresh(f), f));
    Z3_ast term = _data->to_z3(fresh(f));
    
    Z3_lbool res = 
      Z3_solver_check_assumptions(_data->context, _data->solver, 1, &term);
    Z3_solver_pop(_data->context, _data->solver, 1);

    return (res == Z3_L_TRUE);
  }

  bool z3::is_sat() { 
    Z3_lbool result = Z3_solver_check(_data->context, _data->solver);

    return (result == Z3_L_TRUE);
  }

  void z3::clear() { 
    Z3_solver_reset(_data->context, _data->solver);
  }

  // TODO: Factor out common logic with mathsat.cpp
  Z3_ast z3::_z3_t::to_z3(formula f) 
  {
    if(auto it = terms.find(f); it != terms.end()) 
      return it->second;

    Z3_ast term = to_z3_inner(f);
    terms.insert({f, term});

    return term;
  }

  Z3_ast z3::_z3_t::to_z3_inner(formula f) 
  {
    return f.match(
      [this](boolean b) {
        return b.value() ? Z3_mk_true(context) : Z3_mk_false(context);
      },
      [this](atom a) {
        Z3_sort sort = Z3_mk_bool_sort(context);
        Z3_symbol symbol = 
          Z3_mk_string_symbol(context, to_string(a.unique_id()).c_str());
        
        return Z3_mk_const(context, symbol, sort);
      },
      [this](negation, formula n) {
        return Z3_mk_not(context, to_z3(n));
      },
      [this](big_and c) {
        std::vector<Z3_ast> args;
        for(formula op : c.operands())
          args.push_back(to_z3(op));
        
        black_assert(args.size() <= std::numeric_limits<unsigned int>::max());

        return Z3_mk_and(context, 
          static_cast<unsigned int>(args.size()), args.data());
      },
      [this](big_or c) {
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
      [](temporal) -> Z3_ast {
        black_unreachable();
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
