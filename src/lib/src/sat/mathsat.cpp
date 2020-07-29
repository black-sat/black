//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
// (C) 2019 Nicola Gigante
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


#include <black/sat/mathsat.hpp>

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>

#include <fmt/format.h>

#include <string>

namespace black::internal::sat::backends
{

  mathsat::mathsat() 
  {  
    msat_config cfg = msat_create_config();
    msat_set_option(cfg, "model_generation", "true");
    msat_set_option(cfg, "unsat_core_generation","3");
    
    _env = msat_create_env(cfg);
  }

  void mathsat::assert_formula(formula f) {
    msat_assert_formula(_env, to_mathsat(f));
  }

  bool mathsat::is_sat() const { 
    msat_result res = msat_solve(_env);
    return (res == MSAT_SAT);
  }

  void mathsat::push() {
    msat_push_backtrack_point(_env);
  }

  void mathsat::pop() {
    msat_pop_backtrack_point(_env);
  }

  void mathsat::clear() {
    msat_reset_env(_env);
  }

  auto mathsat::backend() const -> backend_t {
    return { _env.repr };
  }


  msat_term mathsat::to_mathsat(formula f) 
  {
    if(auto it = _terms.find(f); it != _terms.end()) 
      return it->second;

    msat_term term = to_mathsat_inner(f);
    _terms.insert({f, term});

    return term;
  }

  msat_term mathsat::to_mathsat_inner(formula f) 
  {
    return f.match(
      [this](boolean b) {
        return b.value() ? msat_make_true(_env) : msat_make_false(_env);
      },
      [this](atom a) {
        msat_decl msat_atom =
          msat_declare_function(_env, to_string(a.unique_id()).c_str(),
          msat_get_bool_type(_env));

        return msat_make_constant(_env, msat_atom);
      },
      [this](negation n) {
        return msat_make_not(_env, to_mathsat(n.operand()));
      },
      [this](conjunction c) {
        msat_term acc = msat_make_true(_env);

        formula next = c;
        std::optional<conjunction> cnext{c};
        do {
          formula left = cnext->left();
          next = cnext->right();
          acc = msat_make_and(_env, acc, to_mathsat(left));
        } while((cnext = next.to<conjunction>()));

        return msat_make_and(_env, acc, to_mathsat(next));
      },
      [this](disjunction d) {
        return msat_make_or(_env, to_mathsat(d.left()), to_mathsat(d.right()));
      },
      [this](then t) {
        return
          msat_make_or(_env,
            msat_make_not(_env, to_mathsat(t.left())), to_mathsat(t.right())
          );
      },
      [this](iff i) {
        return msat_make_iff(_env, to_mathsat(i.left()), to_mathsat(i.right()));
      },
      [](otherwise) -> msat_term {
        black_unreachable();
      }
    );
  }

}
