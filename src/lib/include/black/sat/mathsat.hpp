//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
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

#ifndef BLACK_SOLVER_MATHSAT_HPP
#define BLACK_SOLVER_MATHSAT_HPP

#include <black/sat/sat.hpp>
#include <black/logic/formula.hpp>

#include <mathsat.h>
#include <unordered_map>

namespace black {
  class alphabet;
}

namespace black::internal::sat::backends
{
  class mathsat : ::black::internal::sat::solver
  {
  public:
    mathsat();

    virtual      void assert_formula(formula f);
    virtual      bool is_sat()  const;
    virtual      void push();
    virtual      void pop();
    virtual      void clear();
    virtual backend_t backend() const;

  private:
    mutable msat_env _env;
    std::unordered_map<formula, msat_term> _terms;

    msat_term to_mathsat(formula);
    msat_term to_mathsat_inner(formula);
  };

}

#endif // BLACK_SOLVER_MATHSAT_HPP
