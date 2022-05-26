//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_SOLVER_CORE_HPP
#define BLACK_SOLVER_CORE_HPP

#include <black/logic/formula.hpp>

#include <string>

namespace black::internal {
  
  BLACK_EXPORT
  formula unsat_core(formula f);

  struct core_placeholder_t {
    formula f;
  };

  bool operator==(core_placeholder_t p1, core_placeholder_t p2) {
    return p1.f == p2.f;
  }

  inline std::string to_string(core_placeholder_t) {
    return "{}";
  }
}

namespace black {
  using internal::unsat_core;
  using internal::core_placeholder_t;
}

namespace std {
  template<>
  struct hash<::black::internal::core_placeholder_t> {
    size_t operator()(::black::internal::core_placeholder_t const& p) const {
      return std::hash<::black::internal::formula>{}(p.f);
    }                                                       
  };
}

#endif