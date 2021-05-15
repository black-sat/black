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

#include <vector>
#include <functional>
#include <type_traits>

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
    declare_destructuring_arity(w_yesterday,  1)
    declare_destructuring_arity(always,       1)
    declare_destructuring_arity(eventually,   1)
    declare_destructuring_arity(once,         1)
    declare_destructuring_arity(historically, 1)
    declare_destructuring_arity(conjunction,  2)
    declare_destructuring_arity(disjunction,  2)
    declare_destructuring_arity(implication,  2)
    declare_destructuring_arity(iff,          2)
    declare_destructuring_arity(until,        2)
    declare_destructuring_arity(release,      2)
    declare_destructuring_arity(since,        2)
    declare_destructuring_arity(triggered,    2)

  #undef declare_destructuring_arity

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
    -> decltype(dispatch(f, FWD(h2), FWD(handlers)...)) 
  {
    return dispatch(f, FWD(h2), FWD(handlers)...);
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
      
      black_unreachable(); // LCOV_EXCL_LINE
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
    w_yesterday,
    always,
    eventually,
    once,
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
    w_yesterday,
    always,
    eventually,
    once,
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
  declare_common_type(w_yesterday,  unary)
  declare_common_type(always,       unary)
  declare_common_type(eventually,   unary)
  declare_common_type(once,         unary)
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

namespace black::internal
{
  //
  // Matchers that do not correspond to concrete formula types
  //
  template<typename Formula>
  struct associative_matcher 
  {
    associative_matcher(Formula c) { flatten(c); }

    std::vector<formula> const&operands() const { return _operands; }

  private:
    std::vector<formula> _operands;

    void flatten(Formula c) {
      if(c.left().template is<Formula>())
        flatten(*c.left().template to<Formula>());
      else
        _operands.push_back(c.left());

      if(c.right().template is<Formula>())
        flatten(*c.right().template to<Formula>());
      else
        _operands.push_back(c.right());
    }
  };

  struct big_conjunction : associative_matcher<conjunction> {
    using associative_matcher<conjunction>::associative_matcher;
  };

  struct big_disjunction : associative_matcher<disjunction> {
    using associative_matcher<disjunction>::associative_matcher;
  };

  template<typename>
  struct fragment_matcher;

  template<typename ...Operators>
  struct fragment_matcher<syntax<Operators...>> {
    template<typename T, REQUIRES((std::is_same_v<T, Operators> || ...))>
    fragment_matcher(T t) : _f{t} { }

    class formula formula() const { return _f; }

    template<typename ...Handlers>
    auto match(Handlers&& ...handlers) const {
      return matcher<syntax<Operators...>>::match(_f, FWD(handlers)...);
    }

  private:
    class formula _f;
  };

  using past_ltl_ops = syntax<
    yesterday,
    w_yesterday,
    once,
    historically,
    since,
    triggered
  >;

  using future_ltl_ops = syntax<
    tomorrow,
    always,
    eventually,
    until,
    release
  >;

  using temporal_ltl_ops = syntax<
    tomorrow,
    always,
    eventually,
    until,
    release,
    yesterday,
    w_yesterday,
    once,
    historically,
    since,
    triggered
  >;
  
  using propositional_ops = syntax<
    boolean,
    atom,
    negation,
    conjunction,
    disjunction,
    implication,
    iff
  >;

  struct propositional : fragment_matcher<propositional_ops> { 
    using fragment_matcher<propositional_ops>::fragment_matcher;
  };

  struct temporal : fragment_matcher<temporal_ltl_ops> { 
    using fragment_matcher<temporal_ltl_ops>::fragment_matcher;
  };

  struct future : fragment_matcher<future_ltl_ops> { 
    using fragment_matcher<future_ltl_ops>::fragment_matcher;
  };

  struct past : fragment_matcher<past_ltl_ops> { 
    using fragment_matcher<past_ltl_ops>::fragment_matcher;
  };
  


}

#endif // BLACK_LOGIC_MATCH_HPP_
