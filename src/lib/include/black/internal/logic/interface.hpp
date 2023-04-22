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

#include <black/support/common.hpp>

#include <memory>
#include <ranges>
#include <stack>

//
// This file contains helper classes and functions that integrate the interface
// provided by the API beyond what has been automatically generated in
// `generation.hpp`. Since this stuff is specific to the hierarchy defined in
// `hierarchy.hpp`, and not potentially reusable for other hierarchies, is
// declared in this file separate from `core.hpp`.
//
namespace black_internal::logic {

  //
  // As a first thing we define the real `alphabet` class, which derives from
  // the `alphabet_base` class defined before, which does the heavy lifting.
  //
  class alphabet : public alphabet_base
  {
  public:
    alphabet() = default;
    alphabet(alphabet const&) = delete;
    alphabet(alphabet &&) = default;

    alphabet &operator=(alphabet const&) = delete;
    alphabet &operator=(alphabet &&) = default;

    class boolean top() {
      return this->boolean(true);
    }

    class boolean bottom() {
      return this->boolean(false);
    }

    size_t nonce() { return next_nonce++; }

  private:
    size_t next_nonce = 1;
  };
 
  //
  // Then the implementation of the two versions of operator() for both
  // functions and relations. The declarations are in `interface-fwd.hpp`
  //
  template<typename T>
  auto call_op_get_arg(alphabet *, T v) {
    return v;
  }

  inline variable call_op_get_arg(alphabet *, var_decl x) {
    return x.variable();
  }

  template<std::integral T>
  inline auto call_op_get_arg(alphabet *sigma, T x) {
    return wrap_term_op_arg(sigma, x);
  }

  template<std::floating_point T>
  inline auto call_op_get_arg(alphabet *sigma, T x) {
    return wrap_term_op_arg(sigma, x);
  }

  template<typename T>
  using call_op_arg_t = 
    decltype(call_op_get_arg(std::declval<alphabet *>(), std::declval<T>()));

  template<storage_type S>
  struct call_op_return;

  template<storage_type S>
  inline constexpr auto call_op_return_v = call_op_return<S>::value;

  template<>
  struct call_op_return<storage_type::relation> {
    static constexpr auto value = storage_type::atom;
  };

  template<>
  struct call_op_return<storage_type::function> {
    static constexpr auto value = storage_type::application;
  };

  template<storage_type S, typename Derived>
  template<typename Arg, typename ...Args>
  auto call_op_interface<S, Derived>::operator()(Arg arg, Args ...args) const {
    using common_t = std::common_type_t<
      call_op_arg_t<Arg>, call_op_arg_t<Args>...
    >;

    using syntax = make_combined_fragment_t<
      make_singleton_fragment_t<element_of_storage_v<S>>,
      make_singleton_fragment_t<element_of_storage_v<call_op_return_v<S>>>,
      typename common_t::syntax
    >;
    
    using return_t = storage_type_of_t<syntax, call_op_return_v<S>>;

    Derived const&self = static_cast<Derived const&>(*this);

    std::vector<common_t> v{
      common_t{call_op_get_arg(self.sigma(), arg)}, 
      common_t{call_op_get_arg(self.sigma(), args)}...
    };
    return return_t{self, v};
  }

  template<storage_type S, typename Derived>
  template<std::ranges::range R>
    requires (
      hierarchy<std::ranges::range_value_t<R>> &&
      !std::is_same_v<std::ranges::range_value_t<R>, var_decl>
    )
  auto call_op_interface<S, Derived>::operator()(R const& v) const {
    using syntax = make_combined_fragment_t<
      make_singleton_fragment_t<element_of_storage_v<S>>,
      make_singleton_fragment_t<element_of_storage_v<call_op_return_v<S>>>,
      typename std::ranges::range_value_t<R>::syntax
    >;
    using return_t = storage_type_of_t<syntax, call_op_return_v<S>>;

    return return_t{static_cast<Derived const&>(*this), v};
  }
  
  template<storage_type S, typename Derived>
  template<std::ranges::range R>
    requires std::is_same_v<std::ranges::range_value_t<R>, var_decl>
  auto call_op_interface<S, Derived>::operator()(R const& v) const {
    using syntax = make_fragment_t<
      syntax_list<
        element_of_storage_v<S>,
        element_of_storage_v<call_op_return_v<S>>,
        syntax_element::variable
      >
    >;
    using return_t = storage_type_of_t<syntax, call_op_return_v<S>>;

    std::vector<variable> vars;
    for(auto decl : v)
      vars.push_back(decl.variable());

    return return_t{static_cast<Derived const&>(*this), vars};
  }
  
  //
  // And the implementation of the subscript operator for variables
  //
  template<storage_type S, typename Derived>
  var_decl variable_decl_op<S, Derived>::operator[](sort s) const {
    return s.sigma()->var_decl(static_cast<Derived const&>(*this), s);
  }

  //
  // Now we declare operators acting on terms. The complication here is that we
  // do not support only operators between terms (e.g. x < y) but also between
  // terms and literals (e.g. x < 42), there are two kinds of literals, integral
  // and floating-point, and they can appear on both sides of the operator (e.g.
  // x < 42 or 42 < x). Thus we have many combinations and we do not want to
  // litter everything with giant macros.
  //
  // At first we declare a concept to identify potential arguments to these
  // operators, i.e. terms or arithmetic types.
  template<typename T>
  concept term_op_arg = 
    is_term<T> || std::integral<T> || std::floating_point<T>;

  //
  // Then, since numeric literals have to be wrapped into `constant`s, we need a
  // function to wrap arguments if needed.
  //
  template<is_term T>
  T wrap_term_op_arg(alphabet *, T t) { return t; }

  using wrapped_int = make_fragment_t< 
    syntax_list<
      syntax_element::constant,
      syntax_element::integer
    >
  >;
  
  using wrapped_real = make_fragment_t< 
    syntax_list<
      syntax_element::constant,
      syntax_element::real
    >
  >;

  template<std::integral T>
  constant<wrapped_int> wrap_term_op_arg(alphabet *sigma, T t) {
    return constant{sigma->integer(int64_t{t})};
  }

  template<std::floating_point T>
  constant<wrapped_real> wrap_term_op_arg(alphabet *sigma, T t) { 
    return constant{sigma->real(double{t})};
  }

  //
  // Finally, we can define the operators that apply the given element to the
  // operands after wrapping them. We can assume that at least one of the two
  // operands is a hierarchy type and not an integral ora floating point,
  // because overloaded operators are not looked up for primitive types.
  //
  #define declare_term_op(Op, Element) \
    template<term_op_arg Arg1, term_op_arg Arg2> \
    auto operator Op(Arg1 arg1, Arg2 arg2) { \
      alphabet *sigma = get_sigma(arg1, arg2); \
      return \
        Element(wrap_term_op_arg(sigma, arg1), wrap_term_op_arg(sigma, arg2)); \
    }

  declare_term_op(<, less_than)
  declare_term_op(<=, less_than_equal)
  declare_term_op(>, greater_than)
  declare_term_op(>=, greater_than_equal)
  declare_term_op(-, subtraction)
  declare_term_op(+, addition)
  declare_term_op(*, multiplication)
  declare_term_op(/, division)

  #undef declare_term_op

  // similar function for integer division, but it's not an operator
  template<term_op_arg Arg1, term_op_arg Arg2>
  auto div(Arg1 arg1, Arg2 arg2) {
    alphabet *sigma = get_sigma(arg1, arg2);
    return
      int_division(
        wrap_term_op_arg(sigma, arg1), wrap_term_op_arg(sigma, arg2)
      );
  }

  //
  // The unary minus does not follow the above schema because it's unary, but
  // it's very simple.
  //
  template<is_term T>
  auto operator-(T t) {
    return negative(term{t});
  }

  //
  // The last operators we still need to provide for terms are == and !=. This
  // is delicate because `operator==` is also usually used to compare hierarchy
  // types for equality. So we need to make a special `operator==` that returns
  // a wrapper type which is either convertible to `bool` or to a formula
  // depending on what is needed. This is achieved by just inheriting from
  // equal<Syntax> or not_equal<Syntax>, which also gives us all the benefits of
  // being a `hierarchy` and a `storage_kind`, so we can use
  // `term_equality_wrapper` resulting from `operator==` everywhere a
  // `hierarchy` is expected.
  //
  // The only complication here is that we have to overload logical operators
  // for them to take priority over the && and || operators declared below for
  // formulas. In other words, `f && true` is a formula (a `conjunction<>`)for
  // any formula `f`, but `x == y && true` is a `bool`.
  //
  template<typename Syntax, syntax_element Element>
  struct term_equality_wrapper : element_type_of_t<Syntax, Element> {
    using base_t = element_type_of_t<Syntax, Element>;
    bool _eq;

    term_equality_wrapper(bool eq, base_t b) : base_t{b}, _eq{eq} { }

    bool operator!() const { return !_eq; }
    operator bool() const { return _eq; }
  };

  //
  // The actual operators are simply passing the bool and `comparison` values to
  // the wrapper once we compute the right fragment.
  //
  template<is_term T1, is_term T2>
  auto operator==(T1 t1, T2 t2) {
    using S = deduce_fragment_for_storage_t<syntax_element::equal, T1, T2>;

    return term_equality_wrapper<S, syntax_element::equal>{
      t1.unique_id() == t2.unique_id(), equal<S>(std::vector<term<S>>{t1, t2})
    };
  }

  template<is_term T1, is_term T2>
  auto operator!=(T1 t1, T2 t2) {
    using S = deduce_fragment_for_storage_t<syntax_element::distinct, T1, T2>;

    return term_equality_wrapper<S, syntax_element::distinct>{
      t1.unique_id() != t2.unique_id(), 
      distinct<S>(std::vector<term<S>>{t1, t2})
    };
  }

  //
  // For the other applications of `operator==` to mixed types (e.g. term and
  // double) we call again `term_op` defined above. Note the `requires` clause
  // that excludes the case where both arguments are terms, which is handled
  // with the wrapper above.
  //
  template<term_op_arg Arg1, term_op_arg Arg2>
    requires (!std::is_class_v<Arg1> || !std::is_class_v<Arg2>)
  auto operator ==(Arg1 arg1, Arg2 arg2) {
    alphabet *sigma = get_sigma(arg1, arg2);
    return wrap_term_op_arg(sigma, arg1) == wrap_term_op_arg(sigma, arg2);
  }
  
  template<term_op_arg Arg1, term_op_arg Arg2>
    requires (!std::is_class_v<Arg1> || !std::is_class_v<Arg2>)
  auto operator !=(Arg1 arg1, Arg2 arg2) {
    alphabet *sigma = get_sigma(arg1, arg2);
    return wrap_term_op_arg(sigma, arg1) != wrap_term_op_arg(sigma, arg2);
  }

  //
  // Unary and binary operators for formulas are similar. Here we allow
  // combining formulas with `bool` values. To avoid coding multiple
  // combinations we employ a mechanism similar to the one used above for term
  // operators.
  //
  template<typename T>
  concept formula_op_arg = is_formula<T> || std::is_same_v<T, bool>;

  template<is_formula T1>
  auto wrap_formula_arg(alphabet *, T1 t) { return t; }

  inline boolean wrap_formula_arg(alphabet *sigma, bool b) { 
    return sigma->boolean(b);
  }

  #define declare_unary_formula_op(Func, Op) \
    template<is_formula T> \
    auto Func(T f) { \
      return Op(formula{f}); \
    }

  #define declare_binary_formula_op(Func, Op) \
    template<formula_op_arg F1, formula_op_arg F2> \
      requires (std::is_class_v<F1> || std::is_class_v<F2>) \
    auto Func(F1 f1, F2 f2) { \
      alphabet *sigma = get_sigma(f1, f2); \
      return Op(wrap_formula_arg(sigma, f1), wrap_formula_arg(sigma, f2)); \
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
  declare_binary_formula_op(implies, implication)
  declare_binary_formula_op(U, until)
  declare_binary_formula_op(R, release)
  declare_binary_formula_op(W, w_until)
  declare_binary_formula_op(M, s_release)
  declare_binary_formula_op(S, since)
  declare_binary_formula_op(T, triggered)

  #undef declare_unary_formula_op
  #undef declare_binary_formula_op

  //
  // For conjunctions and disjunctions we have to specialize the macro above to
  // exclude `term_equality_wrapper` from the overloads, in order to avoid
  // problems in boolean expressions involving equality comparisons between
  // terms (e.g. in the code of `std::optional`).
  //
  template<typename T, typename U>
  struct no_bool_equality_wrapper : std::true_type { };

  template<typename S, syntax_element E>
  struct no_bool_equality_wrapper<bool, term_equality_wrapper<S, E>>
    : std::false_type { };

  template<typename S, syntax_element E>
  struct no_bool_equality_wrapper<term_equality_wrapper<S, E>, bool>
    : std::false_type { };

  template<typename T, typename U>
  inline constexpr bool no_bool_equality_wrapper_v = 
    no_bool_equality_wrapper<T, U>::value;

  template<formula_op_arg F1, formula_op_arg F2>
    requires 
      ((std::is_class_v<F1> || std::is_class_v<F2>) &&
        no_bool_equality_wrapper_v<F1, F2>)
  auto operator&&(F1 f1, F2 f2) {
    alphabet *sigma = get_sigma(f1, f2);
    return conjunction(
      wrap_formula_arg(sigma, f1), wrap_formula_arg(sigma, f2)
    );
  }

  template<formula_op_arg F1, formula_op_arg F2>
    requires 
      ((std::is_class_v<F1> || std::is_class_v<F2>) &&
        no_bool_equality_wrapper_v<F1, F2>)
  auto operator||(F1 f1, F2 f2) {
    alphabet *sigma = get_sigma(f1, f2);
    return disjunction(
      wrap_formula_arg(sigma, f1), wrap_formula_arg(sigma, f2)
    );
  }


  //
  // Here we add an `identity()` static member function to some operators such
  // as conjunctions and additions. This is not terribly useful by itself but
  // comes handy in a generic context. The `operands()` function is another
  // addition to the interface of those elements, see below.
  //
  template<typename Derived>
  struct hierarchy_element_custom_members<syntax_element::conjunction, Derived>
  {
    auto operands() const;

    static auto identity(alphabet *sigma) {
      return sigma->boolean(true);
    }
  };
  
  template<typename Derived>
  struct hierarchy_element_custom_members<syntax_element::disjunction, Derived>
  { 
    auto operands() const;

    static auto identity(alphabet *sigma) {
      return sigma->boolean(false);
    }
  };
  
  template<typename Derived>
  struct hierarchy_element_custom_members<syntax_element::addition, Derived>
  { 
    auto operands() const;
  };
  
  template<typename Derived>
  struct hierarchy_element_custom_members<
    syntax_element::multiplication, Derived
  > { 
    auto operands() const;
  };

  //
  // The following is the generic version of the `big_and`, `big_or`, `sum` and
  // `product` functions defined below.
  //
  template<
    syntax_element Op, std::ranges::range Range, typename F,
    typename T = std::ranges::range_value_t<Range>, 
    hierarchy R = std::invoke_result_t<F, T>
  >
  auto fold_op(alphabet &sigma, Range const& r, F&& f)
  {
    auto id = element_type_of_t<typename R::syntax, Op>::identity(&sigma);
    
    using H = hierarchy_type_of_t<
      make_combined_fragment_t<
        typename R::syntax, make_singleton_fragment_t<Op>, 
        typename decltype(id)::syntax
      >,
      R::hierarchy
    >;

    H acc = id;
    for(auto x : r) {
      H elem = std::forward<F>(f)(x);
      if(elem == id)
        continue;
      else if(acc == id)
        acc = elem;
      else
        acc = element_type_of_t<typename H::syntax, Op>(acc, elem);
    }

    return acc;
  }

  //
  // Here we declare an ad-hoc deduction guide to help invoke `exists` and
  // `forall` constructors with an initializer list argument, without specifying
  // the fragment
  //
  template<hierarchy H>
  exists(std::initializer_list<var_decl>, H) 
    -> exists<deduce_fragment_for_storage_t<syntax_element::exists, H>>;
  
  template<hierarchy H>
  forall(std::initializer_list<var_decl>, H) 
    -> forall<deduce_fragment_for_storage_t<syntax_element::forall, H>>;

  //
  // The following are instances of `fold_op`, useful functions to create long
  // conjunctions/disjunctions/sums/products by putting together the results of
  // applying a lambda to a range.
  //
  template<std::ranges::range Range, typename F>
  auto big_and(alphabet &sigma, Range const& r, F&& f) {
    return fold_op<syntax_element::conjunction>(
      sigma, r, std::forward<F>(f)
    );
  }
  
  template<std::ranges::range Range, typename F>
  auto big_or(alphabet &sigma, Range const& r, F&& f) {
    return fold_op<syntax_element::disjunction>(
      sigma, r, std::forward<F>(f)
    );
  }
  
  template<std::ranges::range Range>
  auto big_and(alphabet &sigma, Range const& r) {
    return big_and(sigma, r, [](auto x) { return x; });
  }
  
  template<std::ranges::range Range>
  auto big_or(alphabet &sigma, Range const& r) {
    return big_or(sigma, r, [](auto x) { return x; });
  }
  
  //
  // Here we define a utility class that helps to pattern match associative
  // binary elements such as conjunctions and disjunctions or sums and products.
  // It is a range view that iterates over all the left and right children of
  // descending nodes as long as they continue to be conjunctions or
  // disjunctions. This type is not intended to be used directly but only as a
  // return type of the `arguments()` member function of `conjunction<>`,
  // `disjunction<>` etc...
  //
  // The view type is just a thin wrapper over the conjunction/disjunction, that
  // returns its iterator on `begin()`.
  //
  template<typename E>
  class associative_op_view : public std::ranges::view_base
  {
  public:
    class const_iterator;
    using iterator = const_iterator;

    associative_op_view() = default;
    associative_op_view(E e) : _element{e} { }

    const_iterator begin() const { 
      black_assert(_element.has_value());
      return const_iterator{*_element}; 
    }
    const_iterator end() const { return const_iterator{}; }

  private:
    std::optional<E> _element;
  };

  //
  // The iterator does the hard job. We do a depth-first descent of the formula
  // tree stopping at nodes that are not conjunctions/disjunctions.
  //
  template<typename E>
  class associative_op_view<E>::const_iterator 
  {
  public:
    using difference_type = ssize_t;
    using value_type = 
      hierarchy_type_of_t<typename E::syntax, 
        hierarchy_of_storage(storage_of_element(E::element))
      >;

    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator &&) = default;

    //
    // the constructor called by the view. We setup `_current` to the first
    // element with `go_left()` starting from `e`, so that `operator*()` is
    // ready.
    //
    const_iterator(E e) : _stack{}, _current{go_left(e)} { }

    const_iterator &operator=(const_iterator const&) = default;
    const_iterator &operator=(const_iterator &&) = default;

    // We are the end iterator if there is no current element
    bool operator==(const_iterator const&it) const {
      auto result = _stack == it._stack && _current == it._current;

      return result;
    }

    //
    // operator++ advances the iterator. If the stack is empty we are at the
    // last element, so we set `_current` to `nullopt` and we become the `end()`
    // iterator. Otherwise, we get the next element from the stack and go left
    // as deep as possible with `go_left()` to find the new current element.
    //
    const_iterator &operator++() {
      if(_stack.empty()) {
        _current = std::nullopt;
        return *this;
      }

      value_type f = _stack.top();
      _stack.pop();
      _current = go_left(f);

      return *this;
    }

    // post-increment.
    const_iterator operator++(int) {
      auto o = *this;
      operator++();
      return o;
    }

    // the dereference just returns `_current`
    value_type operator*() const {
      black_assert(_current.has_value());
      return *_current;
    }

  private:
    //
    // Here we go left in the tree rooted at `f` as deep as possible until we
    // find a node which is not an `E`. While going to the left we push on the
    // stack the `right()` children.
    //
    value_type go_left(value_type f) {
      while(f.template is<E>()) {
        auto e = *f.template to<E>();
        _stack.push(e.right());
        f = e.left();
      }
      return f;
    }

    std::stack<value_type> _stack;
    std::optional<value_type> _current;
  };

  //
  // Now that everything is ready we add a member to binary elements returning
  // the above view. These were already declared above so we just define them
  // out-of-line.
  //
  template<typename Derived> 
  auto 
  hierarchy_element_custom_members<syntax_element::conjunction, Derived>::
  operands() const { 
    return associative_op_view<Derived>{static_cast<Derived const&>(*this)};
  }
  
  template<typename Derived> 
  auto 
  hierarchy_element_custom_members<syntax_element::disjunction, Derived>::
  operands() const { 
    return associative_op_view<Derived>{static_cast<Derived const&>(*this)};
  }
  
  template<typename Derived> 
  auto 
  hierarchy_element_custom_members<syntax_element::addition, Derived>::
  operands() const { 
    return associative_op_view<Derived>{static_cast<Derived const&>(*this)};
  }
  
  template<typename Derived> 
  auto 
  hierarchy_element_custom_members<syntax_element::multiplication, Derived>::
  operands() const { 
    return associative_op_view<Derived>{static_cast<Derived const&>(*this)};
  }

  //
  // Using operands() instead of left() and right() is essential to handle big
  // specifications, because going recursively for hundreds of elements can lead
  // easily to stack overflow. So here we specialize `for_each_child` to account
  // for this issue for the types that support `operands()`.
  //
  template<fragment Syntax, typename F>
  void for_each_child(conjunction<Syntax> c, F f) {
    for(auto child : c.operands())
      f(child);
  }
  
  template<fragment Syntax, typename F>
  void for_each_child(disjunction<Syntax> c, F f) {
    for(auto child : c.operands())
      f(child);
  }
  
  template<fragment Syntax, typename F>
  void for_each_child(addition<Syntax> c, F f) {
    for(auto child : c.operands())
      f(child);
  }
  
  template<fragment Syntax, typename F>
  void for_each_child(multiplication<Syntax> c, F f) {
    for(auto child : c.operands())
      f(child);
  }

  //
  // Utility function to replace some subterms of a term with some replacement
  //
  template<fragment Syntax>
  term<Syntax> replace(
    term<Syntax> src, 
    std::vector<term<Syntax>> patterns,
    std::vector<term<Syntax>> replacements
  ) {
    black_assert(patterns.size() == replacements.size());
    auto it = std::find(patterns.begin(), patterns.end(), src);
    if(it != patterns.end())
      return replacements[it - patterns.begin()];

    return src.match(
      [&](application<Syntax>, auto func, auto terms) {
        std::vector<term<Syntax>> newterms;
        for(auto t : terms)
          newterms.push_back(replace(t, patterns, replacements));
        return func(newterms);
      },
      [&](unary_term<Syntax> t, auto arg) {
        return 
          unary_term<Syntax>(
            t.node_type(), replace(arg, patterns, replacements)
          );
      },
      [&](binary_term<Syntax> t, auto left, auto right) {
        return
          binary_term<Syntax>(
            t.node_type(), 
            replace(left, patterns, replacements),
            replace(right, patterns, replacements)
          );
      },
      [&](otherwise) {
        return src;
      }
    );
  }

  //
  // Utility function to replace some term inside a formula with some
  // replacement
  //
  template<fragment Syntax>
  formula<Syntax> replace(
    formula<Syntax> src, 
    std::vector<term<Syntax>> patterns,
    std::vector<term<Syntax>> replacements
  ) {
    return src.match(
      [&](atom<Syntax>, auto rel, auto terms) {
        std::vector<term<Syntax>> newterms;
        for(auto t : terms)
          newterms.push_back(replace(t, patterns, replacements));
        return rel(newterms);
      },
      [&](equality<Syntax> e, auto terms) {
        std::vector<term<Syntax>> newterms;
        for(auto t : terms)
          newterms.push_back(replace(t, patterns, replacements));
        return equality<Syntax>(e.node_type(), newterms);
      },
      [&](comparison<Syntax> c, auto left, auto right) {
        return
          comparison<Syntax>(
            c.node_type(), 
            replace(left, patterns, replacements),
            replace(right, patterns, replacements)
          );
      },
      [&](unary<Syntax> u, auto op) {
        return unary<Syntax>(
          u.node_type(), replace(op, patterns, replacements)
        );
      },
      [&](binary<Syntax> b, auto left, auto right) {
        return
          binary<Syntax>(
            b.node_type(), 
            replace(left, patterns, replacements),
            replace(right, patterns, replacements)
          );
      },
      [&](otherwise) {
        return src;
      }
    );
  }
}

#endif // BLACK_LOGIC_SUGAR_HPP_
