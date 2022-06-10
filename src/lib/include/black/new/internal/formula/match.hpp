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

#ifndef BLACK_LOGIC_MATCH_HPP_
#define BLACK_LOGIC_MATCH_HPP_

#include <black/support/meta.hpp>

#include <functional>
#include <type_traits>
#include <tuple>

namespace black::internal::new_api {
  
  template<syntax_element Element>
  struct hierarchy_of_syntax_element_;

  template<syntax_element Element>
  inline constexpr auto hierarchy_of_syntax_element = 
    hierarchy_of_syntax_element_<Element>::value;

  #define declare(Base, Element) \
    template<> \
    struct hierarchy_of_syntax_element_<syntax_element::Element> { \
      static constexpr auto value = hierarchy_type::Base; \
    };

  #define declare_leaf_storage_kind(Base, Storage) declare(Base, Storage)
  #define has_no_hierarchy_elements(Base, Storage) declare(Base, Storage)
  #define declare_hierarchy_element(Base, Storage, Element) \
    declare(Base, Element)

  #include <black/new/internal/formula/hierarchy.hpp>

  #undef declare

  template<typename TypeList>
  struct are_uniform_elements : std::false_type { };

  template<syntax_element Element, syntax_element ...Elements>
  struct are_uniform_elements<syntax_list<Element, Elements...>>
    : std::bool_constant<
        ((hierarchy_of_syntax_element<Element> == 
          hierarchy_of_syntax_element<Elements>) && ...
        )> { };

  template<typename Syntax>
  struct is_uniform_syntax_ : are_uniform_elements<typename Syntax::list> { };

  template<typename Syntax>
  constexpr bool is_uniform_syntax = is_uniform_syntax_<Syntax>::value;

  template<typename Syntax, typename = void>
  struct hierarchy_of_uniform_syntax_;

  template<typename Syntax>
  struct hierarchy_of_uniform_syntax_<
    Syntax, std::enable_if_t<is_uniform_syntax<Syntax>>
  > {
    static constexpr auto value = 
      hierarchy_of_syntax_element<syntax_list_head_v<typename Syntax::list>>;
  };

  template<typename Syntax>
  inline constexpr auto hierarchy_of_uniform_syntax = 
    hierarchy_of_uniform_syntax_<Syntax>::value;

  template<typename TopLevel, typename Syntax, typename = void>
  struct only;

  template<typename TopLevel, typename Syntax>
  struct only<TopLevel, Syntax, std::enable_if_t<is_uniform_syntax<TopLevel>>> 
  {
    using Base = 
      hierarchy_type_of_t<Syntax, hierarchy_of_uniform_syntax<TopLevel>>;

    template<
      typename H, 
      REQUIRES(
        H::hierarchy == Base::hierarchy && 
        syntax_list_includes_v<
          typename TopLevel::list, 
          typename hierarchy_traits<H>::accepted_elements
        > && is_subfragment_of_v<typename H::syntax, typename Base::syntax>
      )
    >
    only(H h) : _base{h} { }

    operator Base() const { return _base; }

    template<typename H2>
    bool is() const {
      return _base.template is<H2>();
    }

    template<typename H2>
    std::optional<H2> to() const {
      return _base.template to<H2>();
    }

    template<typename ...Handlers>
    auto match(Handlers ...handlers) const {
      return 
        matcher<Base, typename Base::syntax, typename TopLevel::list>{}.match(
          _base, handlers...
        );
    }

  private:
    Base _base;
  };

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

#endif // BLACK_LOGIC_MATCH_HPP_
