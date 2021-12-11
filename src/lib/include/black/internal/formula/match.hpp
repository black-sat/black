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

#ifndef BLACK_LOGIC_FORMULA_MATCH_HPP_
#define BLACK_LOGIC_FORMULA_MATCH_HPP_

#include <vector>
#include <functional>
#include <type_traits>
#include <queue>

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
    declare_destructuring_arity(proposition,  0)
    declare_destructuring_arity(atom,         0)
    declare_destructuring_arity(quantifier,   0)
    declare_destructuring_arity(negation,     1)
    declare_destructuring_arity(tomorrow,     1)
    declare_destructuring_arity(w_tomorrow,   1)
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
    declare_destructuring_arity(w_until,      2)
    declare_destructuring_arity(s_release,    2)
    declare_destructuring_arity(since,        2)
    declare_destructuring_arity(triggered,    2)

  #undef declare_destructuring_arity

}

namespace black::internal
{

  using ltl = syntax<
    boolean,
    proposition,
    atom,
    quantifier,
    negation,
    tomorrow,
    w_tomorrow,
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
    w_until,
    s_release,
    since,
    triggered
  >;

  using unary_ltl_ops = syntax<
    negation,
    tomorrow,
    w_tomorrow,
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
    w_until,
    s_release,
    since,
    triggered
  >;

  template<typename ...Handlers>
  auto formula::match(Handlers&& ...handlers) const {
    return matcher<formula, ltl>::match(*this, FWD(handlers)...);
  }

  template<typename ...Handlers>
  auto unary::match(Handlers&& ...handlers) const {
    return matcher<formula, unary_ltl_ops>::match(*this, FWD(handlers)...);
  }

  template<typename ...Handlers>
  auto binary::match(Handlers&& ...handlers) const {
    return matcher<formula, binary_ltl_ops>::match(*this, FWD(handlers)...);
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

  #define declare_formula_ct(Type)                   \
    template<>                                       \
    struct common_type<black::Type, black::Type> {   \
      using type = black::Type;                      \
    };                                               \
                                                     \
    template<typename T>                             \
    struct common_type<                              \
      enable_if_t<                                   \
        is_convertible_v<T, black::formula>,         \
        black::Type                                  \
      >, T                                           \
    > {                                              \
      using type = black::formula;                   \
    };

  declare_formula_ct(formula)
  declare_formula_ct(boolean)
  declare_formula_ct(proposition)
  declare_formula_ct(atom)
  declare_formula_ct(quantifier)
  declare_formula_ct(unary)
  declare_formula_ct(binary)
  declare_common_type(negation,     unary)
  declare_common_type(tomorrow,     unary)
  declare_common_type(w_tomorrow,   unary)
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
  declare_common_type(w_until,      binary)
  declare_common_type(s_release,    binary)
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
      std::queue<Formula> queue;
      queue.push(c);

      while(!queue.empty()) {
        Formula f = queue.front();
        queue.pop();
        
        if(f.left().template is<Formula>())
          queue.push(*f.left().template to<Formula>());
        else
          _operands.push_back(f.left());

        if(f.right().template is<Formula>())
          queue.push(*f.right().template to<Formula>());
        else
          _operands.push_back(f.right());
      }
    }
  };

  struct big_conjunction : associative_matcher<conjunction> {
    using associative_matcher<conjunction>::associative_matcher;
  };

  struct big_disjunction : associative_matcher<disjunction> {
    using associative_matcher<disjunction>::associative_matcher;
  };

  struct quantifier_block {
    quantifier_block(quantifier q) 
      : _type{q.quantifier_type()}, _matrix{parse(q)} { }

    quantifier::type quantifier_type() const { return _type; }
    std::vector<variable> const&vars() const { return _vars; }
    formula matrix() const { return _matrix; }

  private:
    formula parse(quantifier q) {
      _vars.push_back(q.var());
      formula m = q.matrix();
      return m.match(
        [&](quantifier q2) {
          if(q.quantifier_type() == q2.quantifier_type())
            return parse(q2);
          return m;
        },
        [&](otherwise) { return m; }
      );
    }

    quantifier::type _type;
    std::vector<variable> _vars;
    formula _matrix;
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
      return matcher<class formula, syntax<Operators...>>::match(_f, FWD(handlers)...);
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
    w_tomorrow,
    always,
    eventually,
    until,
    release,
    w_until,
    s_release
  >;

  using temporal_ltl_ops = syntax<
    tomorrow,
    w_tomorrow,
    always,
    eventually,
    until,
    release,
    w_until,
    s_release,
    yesterday,
    w_yesterday,
    once,
    historically,
    since,
    triggered
  >;
  
  using propositional_ops = syntax<
    boolean,
    proposition,
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

#endif // BLACK_LOGIC_FORMULA_MATCH_HPP_
