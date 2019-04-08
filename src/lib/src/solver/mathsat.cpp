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


#include <black/solver/mathsat.hpp>
#include <black/logic/parser.hpp>

#include <fmt/format.h>

namespace black::details {

  msat_env mathsat_init() {
    msat_config cfg = msat_create_config();
    msat_set_option(cfg, "model_generation", "true");
    msat_set_option(cfg, "unsat_core_generation","3");
    msat_env env = msat_create_env(cfg);

    return env;
  }

  msat_term to_mathsat(formula f) {
    return f.match(
      [](boolean b) {
        msat_env env = b.alphabet()->mathsat_env();
        return b.value() ? msat_make_true(env) : msat_make_false(env);
      },
      [](atom a) {
        msat_env env = a.alphabet()->mathsat_env();
        std::string name;

        if(auto aname = a.label<std::string>(); aname.has_value())
          name = *aname;
        else
          if(auto fname = a.label<std::pair<formula,int>>(); fname.has_value())
          name = fmt::format("<{},{}>", to_string(fname->first), fname->second);

        msat_decl msat_atom =
          msat_declare_function(env, name.c_str(), msat_get_bool_type(env));

        return msat_make_constant(env, msat_atom);
      },
      [](negation n) {
        msat_env env = n.alphabet()->mathsat_env();
        return msat_make_not(env, n.operand().to_sat());
      },
      [](conjunction c) {
        msat_env env = c.alphabet()->mathsat_env();
        return msat_make_and(env, c.left().to_sat(), c.right().to_sat());
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


  void print_model(msat_env env)
  {
    /* we use a model iterator to retrieve the model values for all the
     * variables, and the necessary function instantiations */
    msat_model_iterator iter = msat_create_model_iterator(env);
    assert(!MSAT_ERROR_MODEL_ITERATOR(iter));

    fmt::print("Model:\n");
    while (msat_model_iterator_has_next(iter)) {
        msat_term t, v;
        char *s;
        msat_model_iterator_next(iter, &t, &v);
        s = msat_term_repr(t);
        assert(s);
        printf(" %s = ", s);
        msat_free(s);
        s = msat_term_repr(v);
        assert(s);
        printf("%s\n", s);
        msat_free(s);
    }
    msat_destroy_model_iterator(iter);
  }


}
