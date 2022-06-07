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

#ifndef BLACK_LOGIC_SUGAR_HPP_
#define BLACK_LOGIC_SUGAR_HPP_

namespace black::internal::new_api {

  #define declare_term_sugar(Kind, Op, Rel) \
    template< \
      typename T1, typename T2, \
      REQUIRES( \
        T1::hierarchy == hierarchy_type::term && \
        T2::hierarchy == hierarchy_type::term \
      ) \
    > \
    auto operator Op(T1 t1, T2 t2) { \
      using S = make_combined_fragment_t< \
        make_combined_fragment_t<typename T1::syntax, typename T2::syntax>, \
        make_fragment_t<syntax_element::Rel> \
      >; \
      return Kind<S>(t1.sigma()->Rel(), std::vector<term<S>>{t1, t2}); \
    } \
    \
    template<typename T1, typename T2, \
      REQUIRES( \
        T1::hierarchy == hierarchy_type::term && \
        std::is_integral_v<T2> \
      ) \
    > \
    auto operator Op(T1 t1, T2 t2) { \
      using S = make_combined_fragment_t< \
        typename T1::syntax, \
        make_combined_fragment_t< \
          integer::syntax, make_fragment_t<syntax_element::Rel> \
        > \
      >; \
      \
      constant c = constant(t1.sigma()->integer(int64_t{t2})); \
      return Kind<S>(t1.sigma()->Rel(), std::vector<term<S>>{t1, c}); \
    } \
    \
    template<typename T1, typename T2, \
      REQUIRES( \
        T1::hierarchy == hierarchy_type::term && \
        std::is_integral_v<T2> \
      ) \
    > \
    auto operator Op(T2 t2, T1 t1) { \
      using S = make_combined_fragment_t< \
        typename T1::syntax, \
        make_combined_fragment_t< \
          integer::syntax, make_fragment_t<syntax_element::Rel> \
        > \
      >; \
      \
      constant c = constant(t1.sigma()->integer(int64_t{t2})); \
      return Kind<S>(t1.sigma()->Rel(), std::vector<term<S>>{c, t1}); \
    } \
    \
    template<typename T1, typename T2, \
      REQUIRES( \
        T1::hierarchy == hierarchy_type::term && \
        std::is_floating_point_v<T2> \
      ) \
    > \
    auto operator Op(T1 t1, T2 t2) { \
      using S = make_combined_fragment_t< \
        typename T1::syntax, \
        make_combined_fragment_t< \
          real::syntax, make_fragment_t<syntax_element::Rel> \
        > \
      >; \
      \
      constant c = constant(t1.sigma()->real(double{t2})); \
      return Kind<S>(t1.sigma()->Rel(), std::vector<term<S>>{t1, c}); \
    } \
    \
    template<typename T1, typename T2, \
      REQUIRES( \
        T1::hierarchy == hierarchy_type::term && \
        std::is_floating_point_v<T2> \
      ) \
    > \
    auto operator Op(T2 t2, T1 t1) { \
      using S = make_combined_fragment_t< \
        typename T1::syntax, \
        make_combined_fragment_t< \
          real::syntax, make_fragment_t<syntax_element::Rel> \
        > \
      >; \
      \
      constant c = constant(t1.sigma()->real(double{t2})); \
      return Kind<S>(t1.sigma()->Rel(), std::vector<term<S>>{c, t1}); \
    }

  declare_term_sugar(atom, <, less_than)
  declare_term_sugar(atom, <=, less_than_equal)
  declare_term_sugar(atom, >, greater_than)
  declare_term_sugar(atom, >=, greater_than_equal)
  declare_term_sugar(application, -, subtraction)
  declare_term_sugar(application, +, addition)
  declare_term_sugar(application, *, multiplication)
  declare_term_sugar(application, /, division)

  #undef declare_term_sugar

  template<typename T, REQUIRES(T::hierarchy == hierarchy_type::term)>
  auto operator-(T t) {
    return application(t.sigma()->negative(), std::vector{t});
  }

  template<typename Syntax>
  struct term_equality_wrapper {
    bool _eq;
    atom<Syntax> _atom;

    bool operator!() const { return !_eq; }
    operator bool() const { return _eq; }
    
    template<typename S, REQUIRES(is_subfragment_of_v<Syntax, S>)>
    operator atom<S>() const { return _atom; }

    template<typename S, REQUIRES(is_subfragment_of_v<Syntax, S>)>
    operator formula<S>() const { return _atom; }
  };

  template<typename Syntax>
  atom(term_equality_wrapper<Syntax>) -> atom<Syntax>;
  template<typename Syntax>
  formula(term_equality_wrapper<Syntax>) -> formula<Syntax>;

  template<
    typename T1, typename T2, 
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      T2::hierarchy == hierarchy_type::term
    )
  >
  auto operator==(T1 t1, T2 t2) {
    using S = make_combined_fragment_t<
      make_combined_fragment_t<typename T1::syntax, typename T2::syntax>,
      make_fragment_t<syntax_element::equal>
    >;
    return term_equality_wrapper<S>{
      t1.unique_id() == t1.unique_id(),
      atom<S>(t1.sigma()->equal(), std::vector<term<S>>{t1, t2})
    };
  }

  template<
    typename T1, typename T2, 
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      T2::hierarchy == hierarchy_type::term
    )
  >
  auto operator!=(T1 t1, T2 t2) {
    using S = make_combined_fragment_t<
      make_combined_fragment_t<typename T1::syntax, typename T2::syntax>,
      make_fragment_t<syntax_element::not_equal>
    >;
    return term_equality_wrapper<S>{
      t1.unique_id() != t1.unique_id(),
      atom<S>(t1.sigma()->not_equal(), std::vector<term<S>>{t1, t2})
    };
  }

  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_integral_v<T2>
    )
  >
  auto operator==(T1 t1, T2 t2) {
    return t1 == constant{t1.sigma()->integer(int64_t{t2})};
  }
  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_integral_v<T2>
    )
  >
  auto operator!=(T1 t1, T2 t2) {
    return t1 != constant{t1.sigma()->integer(int64_t{t2})};
  }

  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_integral_v<T2>
    )
  >
  auto operator==(T2 t2, T1 t1) {
    return constant{t1.sigma()->integer(int64_t{t2})} == t1;
  }
  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_integral_v<T2>
    )
  >
  auto operator!=(T2 t2, T1 t1) {
    return constant{t1.sigma()->integer(int64_t{t2})} != t1;
  }
  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_floating_point_v<T2>
    )
  >
  auto operator==(T1 t1, T2 t2) {
    return t1 == constant{t1.sigma()->real(double{t2})};
  }
  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_floating_point_v<T2>
    )
  >
  auto operator!=(T1 t1, T2 t2) {
    return t1 != constant{t1.sigma()->real(double{t2})};
  }

  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_floating_point_v<T2>
    )
  >
  auto operator==(T2 t2, T1 t1) {
    return constant{t1.sigma()->real(double{t2})} == t1;
  }
  template<
    typename T1, typename T2,
    REQUIRES(
      T1::hierarchy == hierarchy_type::term &&
      std::is_floating_point_v<T2>
    )
  >
  auto operator!=(T2 t2, T1 t1) {
    return constant{t1.sigma()->real(double{t2})} != t1;
  }

  #define declare_unary_formula_sugar(Func, Op) \
    template<typename T, REQUIRES(T::hierarchy == hierarchy_type::formula)> \
    auto Func(T f) { \
      return Op(f); \
    } \
    template<typename Syntax> \
    auto Func(term_equality_wrapper<Syntax> w) { \
      return Op(w._atom); \
    }
  
  declare_unary_formula_sugar(operator!, negation)
  declare_unary_formula_sugar(X, tomorrow)
  declare_unary_formula_sugar(wX, w_tomorrow)
  declare_unary_formula_sugar(Y, yesterday)
  declare_unary_formula_sugar(Z, w_yesterday)
  declare_unary_formula_sugar(G, always)
  declare_unary_formula_sugar(F, eventually)
  declare_unary_formula_sugar(O, once)
  declare_unary_formula_sugar(H, historically)

  #undef declare_unary_formula_sugar

  #define declare_binary_formula_sugar(Func, Op) \
    template< \
      typename F1, typename F2,  \
      REQUIRES( \
        F1::hierarchy == hierarchy_type::formula && \
        F2::hierarchy == hierarchy_type::formula \
      ) \
    > \
    auto Func(F1 f1, F2 f2) { \
      return Op(f1, f2); \
    } \
    template< \
      typename F1, typename Syntax, \
      REQUIRES(F1::hierarchy == hierarchy_type::formula) \
    > \
    auto Func(F1 f1, term_equality_wrapper<Syntax> w) { \
      return Op(f1, w._atom); \
    } \
    template< \
      typename F1, typename Syntax, \
      REQUIRES(F1::hierarchy == hierarchy_type::formula) \
    > \
    auto Func(term_equality_wrapper<Syntax> w, F1 f1) { \
      return Op(w._atom, f1); \
    }

  declare_binary_formula_sugar(operator&&, conjunction)
  declare_binary_formula_sugar(operator||, disjunction)
  declare_binary_formula_sugar(implies, implication)
  declare_binary_formula_sugar(U, until)
  declare_binary_formula_sugar(R, release)
  declare_binary_formula_sugar(wU, w_until)
  declare_binary_formula_sugar(sR, s_release)
  declare_binary_formula_sugar(S, since)
  declare_binary_formula_sugar(T, triggered)

  #undef declare_binary_formula_sugar

}

#endif // BLACK_LOGIC_SUGAR_HPP_
