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
  
  struct otherwise {
    template<typename T>
    otherwise(T) { }
  };

  namespace match {

    using std::get;

    // this is just like std::apply but applies the formula f before the args
    template<typename Handler, typename Formula, size_t ...I>
    auto unpack_(
      Handler&& handler, Formula f, std::index_sequence<I...>
    ) -> RETURNS_DECLTYPE(FWD(handler)(f, get<I>(f)...))

    template<typename Handler, typename Formula>
    auto unpack(Handler&& handler, Formula f)
    -> RETURNS_DECLTYPE(
      unpack_(
        FWD(handler), f, std::make_index_sequence<std::tuple_size_v<Formula>>{}
      )
    )

    template<typename Handler, typename Formula, typename = void>
    struct can_be_unpacked : std::false_type { };

    template<typename Handler, typename Formula>
    struct can_be_unpacked<
      Handler, Formula, 
      std::void_t<
        decltype(
          unpack(std::declval<Handler>(), std::declval<Formula>())
        )
      >
    > : std::true_type { };
    
    //
    // The dispatch() function is what does the hard job
    //
    template<
      typename Formula, typename Handler, typename ... Handlers,
      REQUIRES(std::is_invocable_v<Handler, Formula>)
    >
    auto dispatch(Formula f, Handler&& handler, Handlers&& ...) {
      return std::invoke(FWD(handler), f);
    }

    template<
      typename Formula, typename Handler, typename ...Handlers,
      REQUIRES(!std::is_invocable_v<Handler, Formula>),
      REQUIRES(can_be_unpacked<Handler, Formula>::value)
    >
    auto dispatch(Formula f, Handler&& handler, Handlers&& ...) {
      return unpack(FWD(handler), f);
    }

    template<
      typename Formula, typename H1, typename H2, typename ...Handlers,
      REQUIRES(!std::is_invocable_v<H1, Formula>),
      REQUIRES(!can_be_unpacked<H1, Formula>::value)
    >
    auto dispatch(Formula f, H1&&, H2&& h2, Handlers&& ...handlers)
    {
      return dispatch(f, FWD(h2), FWD(handlers)...);
    }
  }

  template<hierarchy H, fragment Syntax, syntax_element Case>
  struct matcher<H, Syntax, syntax_list<Case>> 
  {
    using case_t = element_type_of_t<Syntax, Case>;

    template<typename ...Handlers>
    static auto match(H f, Handlers&& ...handlers)
      -> decltype(match::dispatch(*f.template to<case_t>(), FWD(handlers)...))
    {
      if(f.template is<case_t>())
        return match::dispatch(*f.template to<case_t>(), FWD(handlers)...);
      
      black_unreachable(); // LCOV_EXCL_LINE
    }
  };

  template<
    hierarchy H, fragment Syntax, 
    syntax_element Case, syntax_element ...Cases
  >
  struct matcher<H, Syntax, syntax_list<Case, Cases...>>
  {
    using case_t = element_type_of_t<Syntax, Case>;

    template<typename ...Handlers>
    static auto match(H f, Handlers&& ...handlers) 
      -> std::common_type_t<
        decltype(match::dispatch(*f.template to<case_t>(), FWD(handlers)...)),
        decltype(matcher<H, Syntax, syntax_list<Cases...>>::match(
          f, FWD(handlers)...
        ))
      >
    {
      if(f.template is<case_t>())
        return match::dispatch(*f.template to<case_t>(), FWD(handlers)...);
      else
        return matcher<H, Syntax, syntax_list<Cases...>>::match(
          f, FWD(handlers)...
        );
    }
  };

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
