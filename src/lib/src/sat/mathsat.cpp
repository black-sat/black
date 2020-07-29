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

namespace black::internal {

  msat_env mathsat_init() {
    msat_config cfg = msat_create_config();
    msat_set_option(cfg, "model_generation", "true");
    msat_set_option(cfg, "unsat_core_generation","3");
    msat_env env = msat_create_env(cfg);

    return env;
  }

  //
  // TODO: generalize efficient match of nested conjunctions/disjunctions
  //
  static inline msat_term to_mathsat(conjunction c)
  {
    msat_env env = c.alphabet()->mathsat_env();
    msat_term acc = msat_make_true(env);

    formula next = c;
    std::optional<conjunction> cnext{c};
    do {
      formula left = cnext->left();
      next = cnext->right();
      acc = msat_make_and(env, acc, left.to_sat());
    } while((cnext = next.to<conjunction>()));

    return msat_make_and(env, acc, next.to_sat());
  }

  msat_term to_mathsat(formula f) {
    return f.match(
      [](boolean b) {
        msat_env env = b.alphabet()->mathsat_env();
        return b.value() ? msat_make_true(env) : msat_make_false(env);
      },
      [](atom a) {
        msat_env env = a.alphabet()->mathsat_env();

        msat_decl msat_atom =
          msat_declare_function(env, to_string(a.unique_id()).c_str(),
          msat_get_bool_type(env));

        return msat_make_constant(env, msat_atom);
      },
      [](negation n) {
        msat_env env = n.alphabet()->mathsat_env();
        return msat_make_not(env, n.operand().to_sat());
      },
      [](conjunction c) {
        return to_mathsat(c);
      },
      [](disjunction d) {
        msat_env env = d.alphabet()->mathsat_env();
        return msat_make_or(env, d.left().to_sat(), d.right().to_sat());
      },
      [](then t) {
        msat_env env = t.alphabet()->mathsat_env();
        return
          msat_make_or(env,
            msat_make_not(env, t.left().to_sat()), t.right().to_sat()
          );
      },
      [](iff i) {
        msat_env env = i.alphabet()->mathsat_env();
        return msat_make_iff(env, i.left().to_sat(), i.right().to_sat());
      },
      [](otherwise) -> msat_term {
        black_unreachable();
      }
    );
  }

}
