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

#include <ranges>

//
// This file contains helper classes and functions that provide useful syntactic
// sugar on top of the hierarchy types defined in `core.hpp`. Since this stuff
// is specific to the hierarchy defined in `hierarchy.hpp`, and not potentially
// reusable for other hierarchies, is declared in this file separate from
// `core.hpp`.
//
namespace black::internal::new_api {

  //
  // relations and functions support creating the associated atom or application
  // (respectively) with a simple call-like syntax such as f(x, y).
  //
  // Here we declare the needed specialization of `hierarchy_custom_members` for
  // the purpose. We have both a vararg version and one taking a range.
  //
  template<typename Derived>
  struct hierarchy_custom_members<hierarchy_type::function, Derived> {
    template<hierarchy Arg, hierarchy ...Args>
    auto operator()(Arg, Args ...) const;

    template<std::ranges::range R>
      requires hierarchy<std::ranges::range_value_t<R>>
    auto operator()(R const& v) const;
  };

  template<typename Derived>
  struct hierarchy_custom_members<hierarchy_type::relation, Derived> {
    template<hierarchy Arg, hierarchy ...Args>
    auto operator()(Arg, Args ...) const;

    template<std::ranges::range R>
      requires hierarchy<std::ranges::range_value_t<R>>
    auto operator()(R const& v) const;
  };

  //
  // Then the implementation of the two versions of operator() for both
  // functions and relations.
  //
  template<typename Derived>
  template<hierarchy Arg, hierarchy ...Args>
  auto 
  hierarchy_custom_members<hierarchy_type::function, Derived>::operator()(
    Arg arg, Args ...args
  ) const {
    using Syntax = 
      make_combined_fragment_t<
        typename Derived::syntax, typename Arg::syntax, typename Args::syntax...
      >;
    using Hierarchy = hierarchy_type_of_t<Syntax, Arg::hierarchy>;

    std::vector<Hierarchy> v{Hierarchy(arg), Hierarchy(args)...};
    return application<Syntax>(static_cast<Derived const&>(*this), v);
  }

  template<typename Derived>
  template<std::ranges::range R>
    requires hierarchy<std::ranges::range_value_t<R>>
  auto hierarchy_custom_members<hierarchy_type::function, Derived>::operator()(
    R const& v
  ) const {
    using Syntax = make_combined_fragment_t<
      typename Derived::syntax, typename std::ranges::range_value_t<R>::syntax
    >;

    return application<Syntax>(static_cast<Derived const&>(*this), v);
  }
  
  template<typename Derived>
  template<hierarchy Arg, hierarchy ...Args>
  auto 
  hierarchy_custom_members<hierarchy_type::relation, Derived>::operator()(
    Arg arg, Args ...args
  ) const {
    using Syntax = 
      make_combined_fragment_t<
        typename Derived::syntax, typename Arg::syntax, typename Args::syntax...
      >;
    using Hierarchy = hierarchy_type_of_t<Syntax, Arg::hierarchy>;

    std::vector<Hierarchy> v{Hierarchy(arg), Hierarchy(args)...};
    return atom<Syntax>(static_cast<Derived const&>(*this), v);
  }

  template<typename Derived>
  template<std::ranges::range R>
    requires hierarchy<std::ranges::range_value_t<R>>
  auto hierarchy_custom_members<hierarchy_type::relation, Derived>::operator()(
    R const& v
  ) const {
    using Syntax = make_combined_fragment_t<
      typename Derived::syntax, typename std::ranges::range_value_t<R>::syntax
    >;

    return atom<Syntax>(static_cast<Derived const&>(*this), v);
  }

  //
  // Now we declare operators acting on terms. The complication here is that we
  // do not support only operators between terms (e.g. x < y) but also between
  // terms and literals (e.g. x < 42), there are two kinds of literals, integral
  // and floating-point, and they can appear on both sides of the operator (e.g.
  // x < 42 or 42 < x). Thus we have many combinations and we do not litter
  // everything with giant macros.
  //
  // At first we declare a concept to identify potential arguments to these
  // operators, i.e. terms or arithmetic types.
  template<typename T>
  concept term_op_arg = 
    is_term<T> || std::integral<T> || std::floating_point<T>;

  //
  // Then we define a trait to find the fragment associated with an argument
  //
  template<term_op_arg T>
  struct term_op_arg_fragment;

  template<is_term T>
  struct term_op_arg_fragment<T> : std::type_identity<typename T::syntax> { };

  template<std::integral T>
  struct term_op_arg_fragment<T> 
    : make_fragment<syntax_element::constant, syntax_element::integer> { };
  
  template<std::floating_point T>
  struct term_op_arg_fragment<T> 
    : make_fragment<syntax_element::constant, syntax_element::real> { };

  template<term_op_arg T>
  using term_op_arg_fragment_t = typename term_op_arg_fragment<T>::type;

  //
  // And then a trait to compute the resulting fragment given an operator and
  // two arguments.
  //
  template<syntax_element Op, term_op_arg Arg1, term_op_arg Arg2>
  struct term_op_fragment
    : make_combined_fragment<
        make_fragment_t<Op>, 
        term_op_arg_fragment_t<Arg1>, term_op_arg_fragment_t<Arg2>
      > { };

  template<syntax_element Op, term_op_arg Arg1, term_op_arg Arg2>
  using term_op_fragment_t = typename term_op_fragment<Op, Arg1, Arg2>::type;

  //
  // Then, since numeric literals have to be wrapped into `constant`s, we need a
  // function to wrap arguments if needed.
  //
  template<is_term T>
  T wrap_term_op_arg(alphabet *, T t) { return t; }

  template<std::integral T>
  auto wrap_term_op_arg(alphabet *sigma, T t) { 
    return constant{sigma->integer(int64_t{t})};
  }

  template<std::floating_point T>
  auto wrap_term_op_arg(alphabet *sigma, T t) { 
    return constant{sigma->real(double{t})};
  }

  //
  // Finally, we can define a generic function that applies the given operator
  // to the operands in the correct way. We implicitly assume that at least one
  // argument is a term and not an integral or floating point. This will be
  // required explicitly in the actual operators.
  //
  template<
    storage_type Storage, syntax_element Op, term_op_arg Arg1, term_op_arg Arg2
  >
  auto term_op(Arg1 arg1, Arg2 arg2) {
    using syntax = term_op_fragment_t<Op, Arg1, Arg2>;
    using storage = storage_type_of_t<syntax, Storage>;

    alphabet *sigma = get_sigma(arg1, arg2);
    term<syntax> warg1 = wrap_term_op_arg(sigma, arg1);
    term<syntax> warg2 = wrap_term_op_arg(sigma, arg2);

    return storage(sigma->template element<Op>(), std::vector{warg1, warg2});
  }

  //
  // Now we need a macro to wrap `term_op` into the actual operators but it's
  // very thin.
  //
  #define declare_term_op(Storage, Op, Element) \
    template<term_op_arg Arg1, term_op_arg Arg2> \
      requires (std::is_class_v<Arg1> || std::is_class_v<Arg2>) \
    auto operator Op(Arg1 arg1, Arg2 arg2) { \
      return \
        term_op<storage_type::Storage, syntax_element::Element>(arg1, arg2); \
    }

  declare_term_op(atom, <, less_than)
  declare_term_op(atom, <=, less_than_equal)
  declare_term_op(atom, >, greater_than)
  declare_term_op(atom, >=, greater_than_equal)
  declare_term_op(application, -, subtraction)
  declare_term_op(application, +, addition)
  declare_term_op(application, *, multiplication)
  declare_term_op(application, /, division)

  #undef declare_term_op

  //
  // The unary minus does not follow the above schema because it's unary, but
  // it's very simple.
  //
  template<is_term T>
  auto operator-(T t) {
    return application(t.sigma()->negative(), std::vector{t});
  }

  //
  // The last operators we still need to provide for terms are == and !=. This
  // is delicate because `operator==` is also usually used to compare hierarchy
  // types. So we need to make a special `operator==` that returns a wrapper
  // type which is either convertible to `bool` or to a formula depending on
  // what is needed. This is achieved by just inheriting from atom<Syntax>,
  // which also gives us all the benefits of being a `hierarchy` and a
  // `storage_kind`, so we can use `term_equality_wrapper` resulting from
  // `operator==` everywhere a `hierarchy` is expected.
  //
  template<typename Syntax>
  struct term_equality_wrapper : atom<Syntax> {
    bool _eq;

    term_equality_wrapper(bool eq, atom<Syntax> a) 
      : atom<Syntax>{a}, _eq{eq} { }

    bool operator!() const { return !_eq; }
    operator bool() const { return _eq; }
  };

  //
  // The actual operators are simply passing the bool and atom values to the
  // wrapper once we compute the right fragment.
  //
  template<is_term T1, is_term T2>
  auto operator==(T1 t1, T2 t2) {
    using S = deduce_fragment_for_storage_t<syntax_element::equal, T1, T2>;

    return term_equality_wrapper<S>{
      t1.unique_id() == t1.unique_id(),
      atom<S>(t1.sigma()->equal(), std::vector<term<S>>{t1, t2})
    };
  }

  template<is_term T1, is_term T2>
  auto operator!=(T1 t1, T2 t2) {
    using S = deduce_fragment_for_storage_t<syntax_element::not_equal, T1, T2>;

    return term_equality_wrapper<S>{
      t1.unique_id() != t1.unique_id(),
      atom<S>(t1.sigma()->not_equal(), std::vector<term<S>>{t1, t2})
    };
  }

  //
  // For the other applications of `operator==` to mixed types (e.g. term and
  // double) we call again `term_op` defined above. Note the `requires` clause
  // which is different from the one used above.
  //
  template<term_op_arg Arg1, term_op_arg Arg2>
    requires (!std::is_class_v<Arg1> || !std::is_class_v<Arg2>)
  auto operator ==(Arg1 arg1, Arg2 arg2) {
    return
      term_op<storage_type::atom, syntax_element::equal>(arg1, arg2);
  }
  
  template<term_op_arg Arg1, term_op_arg Arg2>
    requires (!std::is_class_v<Arg1> || !std::is_class_v<Arg2>)
  auto operator !=(Arg1 arg1, Arg2 arg2) {
    return
      term_op<storage_type::atom, syntax_element::not_equal>(arg1, arg2);
  }

  //
  // Unary and binary operators for formulas are much simpler.
  //
  #define declare_unary_formula_op(Func, Op) \
    template<is_formula T> \
    auto Func(T f) { \
      return Op(f); \
    }

  #define declare_binary_formula_op(Func, Op) \
    template<is_formula F1, is_formula F2> \
    auto Func(F1 f1, F2 f2) { \
      return Op(f1, f2); \
    }
  
  declare_unary_formula_op(operator!, negation)
  declare_unary_formula_op(X, tomorrow)
  declare_unary_formula_op(wX, w_tomorrow)
  declare_unary_formula_op(Y, yesterday)
  declare_unary_formula_op(Z, w_yesterday)
  declare_unary_formula_op(G, always)
  declare_unary_formula_op(F, eventually)
  declare_unary_formula_op(O, once)
  declare_unary_formula_op(H, historically)
  declare_binary_formula_op(operator&&, conjunction)
  declare_binary_formula_op(operator||, disjunction)
  declare_binary_formula_op(implies, implication)
  declare_binary_formula_op(U, until)
  declare_binary_formula_op(R, release)
  declare_binary_formula_op(wU, w_until)
  declare_binary_formula_op(sR, s_release)
  declare_binary_formula_op(S, since)
  declare_binary_formula_op(T, triggered)

  #undef declare_unary_formula_op
  #undef declare_binary_formula_op

}

#endif // BLACK_LOGIC_SUGAR_HPP_
