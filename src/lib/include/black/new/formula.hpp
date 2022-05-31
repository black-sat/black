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

#include <variant>

namespace std {
  template<typename T>
  struct hash<std::vector<T>>
  {
    size_t operator()(std::vector<T> const&v) const {
      hash<T> h;
      size_t result = 0;
      for(size_t i = 0; i < v.size(); ++i)
        result = ::black::internal::hash_combine(result, h(v[i]));

      return result;
    }
  };
}

namespace black::internal::new_api {

  class alphabet;
  class application;
  //
  // Helper function to call sigma() on the first argument that supports
  // the call
  //
  template<typename T, typename = void>
  struct has_sigma : std::false_type {  };

  template<typename T>
  struct has_sigma<T, std::void_t<decltype(std::declval<T>().sigma())>>
    : std::true_type { };

  template<typename T>
  alphabet *get_sigma(T v) {
    return v.sigma();
  }

  template<typename T, REQUIRES(has_sigma<T>::value)>
  alphabet *get_sigma(std::vector<T> const&v) {
    black_assert(!v.empty());
    return v[0].sigma();
  }

  template<typename T, typename ...Args>
  alphabet *get_sigma(T v, Args ...args) {
    if constexpr(has_sigma<T>::value)
      return v.sigma();
    else
      return get_sigma(args...);
  }

  //
  // Helper trait to tell if a type has an `_element` member
  //
  template<typename T, typename = void>
  struct has_element : std::false_type { };

  template<typename T>
  struct has_element<T, std::void_t<decltype(std::declval<T>()._element)>>
    : std::true_type { };

  template<typename ...Ops>
  struct syntax { };

  
}

#include <black/new/internal/formula/interface.hpp>
#include <black/new/internal/formula/alphabet.hpp>
#include <black/new/internal/formula/impl.hpp>

namespace black::internal::new_api {
  using LTL = syntax<
    #define has_no_leaf_hierarchy_elements(Base, Storage) \
      Storage,
    #define has_no_hierarchy_elements(Base, Storage) \
      Storage<void>,
    #define declare_hierarchy_element(Base, Storage, Element) \
      Element<void>,
    #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      Element,
    #include <black/new/internal/formula/hierarchy.hpp>
  void>;
}

#endif // BLACK_LOGIC_FORMULA_HPP
