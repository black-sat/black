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
  struct has_member_element : std::false_type { };

  template<typename T>
  struct has_member_element<T, 
    std::void_t<decltype(std::declval<T>()._element)>
  > : std::true_type { };

  template<typename List, typename SubList>
  struct syntax_list_includes_ : std::false_type { };

  template<typename List, syntax_element ...Types>
  struct syntax_list_includes_<List, syntax_list<Types...>> {
    static constexpr bool value = (syntax_list_contains_v<List, Types> && ...);
  };

  template<typename List, typename Sublist>
  constexpr bool syntax_list_includes = syntax_list_includes_<List, Sublist>::value;

  template<typename List, typename AcceptsType>
  struct syntax_list_filter_;

  template<typename List, typename AcceptsType>
  using syntax_list_filter = typename syntax_list_filter_<List, AcceptsType>::type;

  template<typename AcceptsType>
  struct syntax_list_filter_<syntax_list<>, AcceptsType> {
    using type = syntax_list<>;
  };

  template<typename AcceptsType, syntax_element Type, syntax_element ...Types>
  struct syntax_list_filter_<syntax_list<Type, Types...>, AcceptsType> {
    using type = std::conditional_t<
      AcceptsType::doesit(Type), 
      syntax_list_concat_t<
        syntax_list<Type>, syntax_list_filter<syntax_list<Types...>, AcceptsType>
      >,
      syntax_list_filter<syntax_list<Types...>, AcceptsType>
    >;
  };

  template<typename Syntax, typename Allowed>
  constexpr bool is_syntax_allowed = 
    syntax_list_includes<
      typename Allowed::list,
      typename Syntax::list
    >;

  template<typename ...Syntaxes>
  struct are_syntaxes_equivalent_ : std::true_type { };

  template<typename ...Syntaxes>
  constexpr bool are_syntaxes_equivalent = 
    are_syntaxes_equivalent_<Syntaxes...>::value;

  template<typename Syntax1, typename Syntax2>
  struct are_syntaxes_equivalent_<Syntax1, Syntax2> : 
    std::bool_constant<
      is_syntax_allowed<Syntax1, Syntax2> && is_syntax_allowed<Syntax2,Syntax1>
    > { };

  template<typename Syntax, typename ...Syntaxes>
  struct are_syntaxes_equivalent_<Syntax, Syntaxes...> :
    std::conjunction<std::is_same<Syntax, Syntaxes>...> { };


  template<syntax_element Type, typename Allowed>
  constexpr bool is_type_allowed = 
    syntax_list_contains_v<typename Allowed::list, Type>;

  template<typename Derived>
  struct function_call_operator_t {
    template<typename Arg, typename ...Args>
    auto operator()(Arg, Args ...) const;

    template<typename T>
    auto operator()(std::vector<T> const& v) const;
  };

  template<typename Derived>
  struct relation_call_operator_t {
    template<typename Arg, typename ...Args>
    auto operator()(Arg, Args ...) const;

    template<typename T>
    auto operator()(std::vector<T> const& v) const;
  };
}

#include <black/new/internal/formula/fragments.hpp>
#include <black/new/internal/formula/interface.hpp>
#include <black/new/internal/formula/alphabet.hpp>
#include <black/new/internal/formula/impl.hpp>
#include <black/new/internal/formula/match.hpp>
#include <black/new/internal/formula/namespaces.hpp>
#include <black/new/internal/formula/sugar.hpp>

namespace black::internal::new_api {
  template<typename Derived>
  template<typename Arg, typename ...Args>
  auto 
  function_call_operator_t<Derived>::operator()(Arg arg, Args ...args) const 
  {
    using Syntax = 
      make_combined_fragment<
        typename Derived::syntax, typename Arg::syntax, typename Args::syntax...
      >;
    using Hierarchy = hierarchy_type_of<Syntax, Arg::hierarchy>;

    std::vector<Hierarchy> v{Hierarchy(arg), Hierarchy(args)...};
    return application<Syntax>(static_cast<Derived const&>(*this), v);
  }

  template<typename Derived>
  template<typename T>
  auto 
  function_call_operator_t<Derived>::operator()(std::vector<T> const& v) const {
    using Syntax = make_combined_fragment<
      typename Derived::syntax, typename T::syntax
    >;

    return application<Syntax>(static_cast<Derived const&>(*this), v);
  }
  
  template<typename Derived>
  template<typename Arg, typename ...Args>
  auto 
  relation_call_operator_t<Derived>::operator()(Arg arg, Args ...args) const 
  {
    using Syntax = 
      make_combined_fragment<
        typename Derived::syntax, typename Arg::syntax, typename Args::syntax...
      >;
    using Hierarchy = hierarchy_type_of<Syntax, Arg::hierarchy>;

    std::vector<Hierarchy> v{Hierarchy(arg), Hierarchy(args)...};
    return atom<Syntax>(static_cast<Derived const&>(*this), v);
  }

  template<typename Derived>
  template<typename T>
  auto 
  relation_call_operator_t<Derived>::operator()(std::vector<T> const& v) const {
    using Syntax = make_combined_fragment<
      typename Derived::syntax, typename T::syntax
    >;

    return atom<Syntax>(static_cast<Derived const&>(*this), v);
  }
}

#endif // BLACK_LOGIC_FORMULA_HPP
