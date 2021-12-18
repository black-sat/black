//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#include <black/frontend/support.hpp>

namespace black::frontend 
{
  bool has_next(term t) {
    return t.match(
      [](constant) { return false; },
      [](variable) { return false; },
      [](application a) {
        for(term t2 : a.arguments())
          if(has_next(t2))
            return true;
        return false;
      },
      [](next) { return true; },
      [](wnext) { return true; }
    );
  }

  uint8_t formula_features(formula f) {
    return f.match(
      [](boolean) -> uint8_t { return 0; },
      [](proposition) -> uint8_t { return 0; },
      [](atom a) -> uint8_t {
        uint8_t nextvar = 0;
        for(term t : a.terms())
          if(has_next(t))
            nextvar = (uint8_t)feature_t::nextvar;
        return nextvar | (uint8_t)feature_t::first_order;
      },
      [](quantifier) -> uint8_t {
        return (uint8_t)feature_t::first_order |
               (uint8_t)feature_t::quantifiers;
      },
      [](temporal t) -> uint8_t {
        return (uint8_t)feature_t::temporal |
          t.match(
            [](past) -> uint8_t { return (int8_t)feature_t::past; },
            [](otherwise) -> uint8_t { return 0; }
          ) | t.match(
            [](unary, formula arg) -> uint8_t {
              return formula_features(arg);
            },
            [](binary, formula left, formula right) -> uint8_t {
              return formula_features(left) | formula_features(right);
            }
          );
      },
      [](unary, formula arg) -> uint8_t { 
        return formula_features(arg);
      },
      [](binary, formula left, formula right) -> uint8_t {
        return formula_features(left) | formula_features(right);
      }
    );
  }
}