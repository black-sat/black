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

#ifndef BLACK_LOGIC_FORMULA_HPP
#define BLACK_LOGIC_FORMULA_HPP

#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#include <type_traits>
#include <variant>

#include <black/new/internal/formula/support.hpp>

namespace black::internal::new_api {

  class alphabet;

}

#include <black/new/internal/formula/interface.hpp>
#include <black/new/internal/formula/alphabet.hpp>
#include <black/new/internal/formula/impl.hpp>

namespace black::internal::new_api {
  namespace matching_fragments {
    struct Future : make_fragment_t<
      syntax_element::tomorrow,
      syntax_element::w_tomorrow,
      syntax_element::always,
      syntax_element::eventually,
      syntax_element::until,
      syntax_element::release
    > { };

    struct Past : make_fragment_t<
      syntax_element::yesterday,
      syntax_element::w_yesterday,
      syntax_element::once,
      syntax_element::historically,
      syntax_element::since,
      syntax_element::triggered
    > { };

    struct Temporal : make_combined_fragment_t<Future, Past> { };
  }
}

#include <black/new/internal/formula/sugar.hpp>
#include <black/new/internal/formula/namespaces.hpp>


#endif // BLACK_LOGIC_FORMULA_HPP
