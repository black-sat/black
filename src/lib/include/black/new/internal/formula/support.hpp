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

#ifndef BLACK_LOGIC_SUPPORT_HPP_
#define BLACK_LOGIC_SUPPORT_HPP_

namespace black::internal::new_api {
  
  class alphabet;

  enum class hierarchy_type : uint8_t {
    #define declare_hierarchy(Base) Base,
    #include <black/new/internal/formula/hierarchy.hpp>
  };

  enum class syntax_element : uint8_t {
    no_type,

    #define declare_leaf_storage_kind(Base, Storage) Storage,
    #define has_no_hierarchy_elements(Base, Storage) Storage,
    #define declare_hierarchy_element(Base, Storage, Element) Element,

    #include <black/new/internal/formula/hierarchy.hpp>
  };

  template<syntax_element ...Elements>
  struct type_list { };

  template<typename T>
  struct is_type_list : std::false_type { };

  template<syntax_element ...Elements>
  struct is_type_list<type_list<Elements...>> : std::true_type { };

  template<typename T>
  constexpr bool is_type_list_v = is_type_list<T>::value;

  template<typename T>
  concept TypeList = is_type_list_v<T>;

  template<typename T>
  concept AcceptsType = requires(syntax_element e) {
    { T::doesit(e) } -> std::convertible_to<bool>;
  };

  template<template<AcceptsType> class T>
  concept FragmentType = true;

  template<typename T>
  concept Syntax = requires {
    requires TypeList<typename T::list>;
    requires FragmentType<T::template type>;
  };

  template<typename T>
  concept Hierarchy = requires(T t) {
    requires Syntax<typename T::syntax>;
    requires AcceptsType<typename T::accepts_type>;
    requires FragmentType<T::template type>;
    requires TypeList<typename T::syntax_elements>;
    { T::hierarchy } -> std::convertible_to<hierarchy_type>;
  };

}

#endif // BLACK_LOGIC_SUPPORT_HPP_