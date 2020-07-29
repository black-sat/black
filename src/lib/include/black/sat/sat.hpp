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

#ifndef BLACK_SAT_SOLVER_HPP
#define BLACK_SAT_SOLVER_HPP

#include <black/logic/formula.hpp>

namespace black::internal::sat 
{  

  //
  // Generic interface to backend SAT solvers
  //  
  class solver 
  {
  public:
    struct backend_t { void * handle; };

    virtual ~solver() = default;

    // assert a formula, adding it to the current context
    virtual void assert_formula(formula f) = 0;

    // tell if the current set of assertions is satisfiable
    virtual bool is_sat() const = 0;

    // push the current context on the assertion stack
    virtual void push() = 0;

    // pop the current context from the assertion stack
    virtual void pop() = 0;

    // clear the current context completely
    virtual void clear() = 0;

    // retrieve a low-level handle to the underlying backend
    virtual backend_t backend() const = 0;
  };

}

namespace black {

  namespace sat = internal::sat;

}

#endif
