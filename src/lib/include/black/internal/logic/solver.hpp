//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_LOGIC_SOLVER_HPP
#define BLACK_LOGIC_SOLVER_HPP

#include <black/support>
#include <black/logic>

namespace black::logic {

  class solver {
  public:
    solver() = default;
    solver(solver const&) = delete;
    solver(solver &&) = delete;

    virtual ~solver() = default;

    solver &operator=(solver const&) = delete;
    solver &operator=(solver &&) = delete;

    virtual void import(module m) = 0;

    virtual void require(term r) = 0;

    virtual void push() = 0;

    virtual void pop() = 0;

    virtual support::tribool check() = 0;
    
    virtual support::tribool check_with(term t) = 0;
  };

}

#endif // BLACK_LOGIC_SOLVER_HPP
