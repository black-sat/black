//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#include <functional>

#ifndef BLACK_LOGIC_FORMULA_HPP_
  #error "This header file cannot be included alone, "\
         "please include <black/logic/formula.hpp> instead"
#endif

namespace black::internal
{
  template<int I, REQUIRES(I == 0)>
  formula get(unary u) {
    return u.operand();
  }

  template<int I, REQUIRES(I <= 1)>
  formula get(binary b) {
    if constexpr(I == 0)
      return b.left();
    else if constexpr(I == 1)
      return b.right();
  }
}

namespace std {

  #define declare_destructuring_arity(Formula, Arity)    \
    template<>                                           \
    struct tuple_size<black::internal::Formula>          \
      : std::integral_constant<int, Arity> { };          \
                                                         \
    template<size_t I>                                   \
    struct tuple_element<I, black::internal::Formula> {  \
      using type = black::internal::formula;             \
    };

    declare_destructuring_arity(boolean,      0)
    declare_destructuring_arity(atom,         0)
    declare_destructuring_arity(negation,     1)
    declare_destructuring_arity(tomorrow,     1)
    declare_destructuring_arity(yesterday,    1)
    declare_destructuring_arity(always,       1)
    declare_destructuring_arity(eventually,   1)
    declare_destructuring_arity(past,         1)
    declare_destructuring_arity(historically, 1)
    declare_destructuring_arity(conjunction,  2)
    declare_destructuring_arity(disjunction,  2)
    declare_destructuring_arity(implication,  2)
    declare_destructuring_arity(iff,          2)
    declare_destructuring_arity(until,        2)
    declare_destructuring_arity(release,      2)
    declare_destructuring_arity(since,        2)
    declare_destructuring_arity(triggered,    2)

  #undef declare_destructuring

}

namespace black::internal
{
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
  
  template<typename Formula, typename Handler, typename ...Handlers>
  auto dispatch(Formula f, Handler&& handler, Handlers&& ...handlers) {
    if constexpr(std::is_invocable_v<Handler, Formula>)
      return std::invoke(FWD(handler), f);
    else if constexpr(can_be_unpacked<Handler, Formula>::value)
      return unpack(FWD(handler), f);
    else if constexpr(sizeof...(Handlers) > 0)
      return dispatch(f, FWD(handlers)...);

    black_unreachable();
  }

  template<typename ...Operators>
  struct syntax { };

  template<typename ...Cases>
  struct matcher;

  template<typename Case>
  struct matcher<syntax<Case>> {
    template<typename ...Handlers>
    static auto match(formula f, Handlers&& ...handlers)
      -> decltype(dispatch(*f.to<Case>(), FWD(handlers)...))
    {
      if(f.is<Case>())
        return dispatch(*f.to<Case>(), FWD(handlers)...);
      
      black_unreachable();
    }
  };

  template<typename Case, typename ...Cases>
  struct matcher<syntax<Case, Cases...>>
  {
    template<typename ...Handlers>
    static auto match(formula f, Handlers&& ...handlers) 
      -> std::common_type_t<
        decltype(dispatch(*f.to<Case>(), FWD(handlers)...)),
        decltype(matcher<syntax<Cases...>>::match(f, FWD(handlers)...))
      >
    {
      if(f.is<Case>())
        return dispatch(*f.to<Case>(), FWD(handlers)...);
      else
        return matcher<syntax<Cases...>>::match(f, FWD(handlers)...);
    }
  };

  using ltl = syntax<
    boolean,
    atom,
    negation,
    tomorrow,
    yesterday,
    always,
    eventually,
    past,
    historically,
    conjunction,
    disjunction,
    implication,
    iff,
    until,
    release,
    since,
    triggered
  >;

  using unary_ltl_ops = syntax<
    negation,
    tomorrow,
    yesterday,
    always,
    eventually,
    past,
    historically
  >;

  using binary_ltl_ops = syntax<
    conjunction,
    disjunction,
    implication,
    iff,
    until,
    release,
    since,
    triggered
  >;

  template<typename ...Handlers>
  auto formula::match(Handlers&& ...handlers) const {
    return matcher<ltl>::match(*this, FWD(handlers)...);
  }

  template<typename ...Handlers>
  auto unary::match(Handlers&& ...handlers) const {
    return matcher<unary_ltl_ops>::match(*this, FWD(handlers)...);
  }

  template<typename ...Handlers>
  auto binary::match(Handlers&& ...handlers) const {
    return matcher<binary_ltl_ops>::match(*this, FWD(handlers)...);
  }
}

namespace std {

  #define declare_common_type(Particular, General)                           \
    template<>                                                               \
    struct common_type<black::Particular, black::Particular> {               \
      using type = black::Particular;                                        \
    };                                                                       \
                                                                             \
    template<typename T>                                                     \
    struct common_type<                                                      \
      enable_if_t<is_convertible_v<T, black::General>, black::Particular>, T \
    > {                                                                      \
      using type = black::General;                                           \
    };                                                                       \
                                                                             \
    template<typename T>                                                     \
    struct common_type<                                                      \
      enable_if_t<                                                           \
        !is_convertible_v<T, black::General> &&                              \
        is_convertible_v<T, black::formula>,                                 \
        black::Particular                                                    \
      >, T                                                                   \
    > {                                                                      \
      using type = black::formula;                                           \
    };

  #define declare_formula_ct(Type)           \
    template<typename T>                     \
    struct common_type<                      \
      enable_if_t<                           \
        is_convertible_v<T, black::formula>, \
        black::Type                          \
      >, T                                   \
    > {                                      \
      using type = black::formula;           \
    };

  declare_formula_ct(formula)
  declare_formula_ct(boolean)
  declare_formula_ct(atom)
  declare_formula_ct(unary)
  declare_formula_ct(binary)
  declare_common_type(negation,     unary)
  declare_common_type(tomorrow,     unary)
  declare_common_type(yesterday,    unary)
  declare_common_type(always,       unary)
  declare_common_type(eventually,   unary)
  declare_common_type(past,         unary)
  declare_common_type(historically, unary)
  declare_common_type(conjunction,  binary)
  declare_common_type(disjunction,  binary)
  declare_common_type(implication,  binary)
  declare_common_type(iff,          binary)
  declare_common_type(until,        binary)
  declare_common_type(release,      binary)
  declare_common_type(since,        binary)
  declare_common_type(triggered,    binary)

  #undef declare_common_type
  #undef declare_formula_ct
}

#endif // BLACK_LOGIC_MATCH_HPP_
