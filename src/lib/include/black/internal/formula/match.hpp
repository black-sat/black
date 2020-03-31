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

#ifndef BLACK_LOGIC_FORMULA_HPP_
  #error "This header file cannot be included alone, "\
         "please include <black/logic/formula.hpp> instead"
#endif

namespace black::internal
{
  // First-match-first-called apply function
  template<typename ...Args, typename F, typename ...Fs>
  auto apply_first(std::tuple<Args...> args, F f, Fs ...fs)
  {
    if constexpr(std::is_invocable_v<F, Args...>) {
      return std::apply(f, args);
    } else if constexpr(sizeof...(Fs) > 0)
      return apply_first(args, fs...);
  }

  // Convenience version with a single argument
  template<typename Arg, typename ...Fs>
  auto apply_first(Arg&& arg, Fs ...fs)
  {
    return apply_first(std::make_tuple(FWD(arg)), fs...);
  }

  template<typename ...Operators>
  struct syntax { };

  template<typename ...Cases>
  struct matcher;

  template<typename Case>
  struct matcher<syntax<Case>> {
    template<typename ...Handlers>
    static auto match(formula f, Handlers&& ...handlers)
      -> decltype(apply_first(*f.to<Case>(), FWD(handlers)...))
    {
      if(f.is<Case>())
        return apply_first(*f.to<Case>(), FWD(handlers)...);
      
      black_unreachable();
    }
  };

  template<typename Case, typename ...Cases>
  struct matcher<syntax<Case, Cases...>>
  {
    template<typename ...Handlers>
    static auto match(formula f, Handlers&& ...handlers) 
      -> std::common_type_t<
        decltype(apply_first(*f.to<Case>(), FWD(handlers)...)),
        decltype(matcher<syntax<Cases...>>::match(f, FWD(handlers)...))
      >
    {
      if(f.is<Case>())
        return apply_first(*f.to<Case>(), FWD(handlers)...);
      else if constexpr(sizeof...(Cases) > 0)
        return matcher<syntax<Cases...>>::match(f, FWD(handlers)...);
      
      black_unreachable();
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
    then,
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
    then,
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
  declare_common_type(then,         binary)
  declare_common_type(iff,          binary)
  declare_common_type(until,        binary)
  declare_common_type(release,      binary)
  declare_common_type(since,        binary)
  declare_common_type(triggered,    binary)

  #undef declare_common_type
  #undef declare_formula_ct
}

#endif // BLACK_LOGIC_MATCH_HPP_
