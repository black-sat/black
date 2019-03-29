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
    msat_env env = msat_create_env(cfg);

    return env;
  }

  msat_term to_mathsat(msat_env env, formula f) {
    return f.match(
      [&](boolean b) {
        return b.value() ? msat_make_true(env) : msat_make_false(env);
      },
      [&](atom a) {
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
      [&](negation n) {
        return msat_make_not(env, to_mathsat(env, n.operand()));
      },
      [&](conjunction c) {
        return
          msat_make_and(env,
            to_mathsat(env, c.left()),
            to_mathsat(env, c.right()));
      },
      [&](disjunction c) {
        return
          msat_make_or(env,
            to_mathsat(env, c.left()),
            to_mathsat(env, c.right()));
      },
      [&](then t) {
        return
          msat_make_or(env,
            msat_make_not(env, to_mathsat(env, t.left())),
            to_mathsat(env, t.right())
          );
      },
      [&](iff i) {
        return
          msat_make_iff(env,
            to_mathsat(env, i.left()),
            to_mathsat(env, i.right())
          );
      },
      [&](otherwise) -> msat_term {
        black_unreachable();
      }
    );
  }

}
