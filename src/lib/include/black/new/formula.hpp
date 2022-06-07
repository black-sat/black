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
  template<hierarchy T, typename ...Args>
  alphabet *get_sigma(std::vector<T> const&v, Args ...) {
    black_assert(!v.empty());
    return v[0].sigma();
  }

  template<hierarchy T, typename ...Args>
  alphabet *get_sigma(T v, Args ...) {
      return v.sigma();
  }
  
  template<typename T, typename ...Args>
  alphabet *get_sigma(T, Args ...args) {
      return get_sigma(args...);
  }

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

  enum class hierarchy_type  : uint8_t {
    #define declare_hierarchy(Base) Base,
    #include <black/new/internal/formula/hierarchy.hpp>
  };

  enum class storage_type  : uint8_t {

    #define declare_storage_kind(Base, Storage) Storage,
    #include <black/new/internal/formula/hierarchy.hpp>

  };

  enum class syntax_element : uint8_t {
    #define declare_leaf_storage_kind(Base, Storage) Storage,
    #define has_no_hierarchy_elements(Base, Storage) Storage,
    #define declare_hierarchy_element(Base, Storage, Element) Element,

    #include <black/new/internal/formula/hierarchy.hpp>
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
