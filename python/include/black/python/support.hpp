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

#ifndef BLACK_PYTHON_SUPPORT_HPP_
#define BLACK_PYTHON_SUPPORT_HPP_

#include <black/logic/logic.hpp>

#include <pybind11/pybind11.h>

namespace pyblack 
{  
  namespace py = pybind11;
  namespace internal = black_internal::logic;

  using syntax = black::logic::LTLPFO;

  template<typename List>
  struct make_universal_variant;

  template<black::syntax_element ...Elements>
  struct make_universal_variant<black::syntax_list<Elements...>> {
    using type = std::variant<internal::element_type_of_t<syntax, Elements>...>;
  };

  using universal_variant_t = typename 
    make_universal_variant<internal::universal_fragment_t::list>::type;

  template<typename T>
  inline auto specialize(T&& t) {
    return t;
  }

  template<typename H>
    requires black::hierarchy<std::remove_cvref_t<H>>
  inline auto specialize(H&& h) {
    return h.match(
      [](auto x) {
        return universal_variant_t{x};
      }
    );
  }

  template<typename H>
    requires black::hierarchy<std::remove_cvref_t<H>>
  inline std::optional<universal_variant_t> specialize(std::optional<H> h) {
    if(!h)
      return std::nullopt;
    return std::optional{universal_variant_t{specialize(*h)}};
  }

}

#endif // BLACK_PYTHON_SUPPORT_HPP_

