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
  // As a first thing we define the real `alphabet` class, which is just a thin
  // addition over the `alphabet_base` class defined before, which does the
  // heavy lifting.
  //
  class alphabet : public alphabet_base
  {
  public:
    alphabet() : _default_sort{this->custom_sort("default")} { }
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

    sort default_sort() const { return _default_sort; }
    void set_default_sort(sort s) { _default_sort = s; }

  private:
    sort _default_sort;
  };
 
  //
  // Then the implementation of the two versions of operator() for both
  // functions and relations. The declarations are in `interface-fwd.hpp`
  //
  template<typename Derived>
  template<hierarchy Arg, hierarchy ...Args>
  auto function_call_op<Derived>::operator()(Arg arg, Args ...args) const {
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
  auto function_call_op<Derived>::operator()(R const& v) const {
    using Syntax = make_combined_fragment_t<
      typename Derived::syntax, typename std::ranges::range_value_t<R>::syntax
    >;

    return application<Syntax>(static_cast<Derived const&>(*this), v);
  }
  
  template<typename Derived>
  template<hierarchy Arg, hierarchy ...Args>
  auto relation_call_op<Derived>::operator()(Arg arg, Args ...args) const {
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
  auto relation_call_op<Derived>::operator()(R const& v) const {
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

  template<std::integral T>
  auto wrap_term_op_arg(alphabet *sigma, T t) { 
    return constant{sigma->integer(int64_t{t})};
  }

  template<std::floating_point T>
  auto wrap_term_op_arg(alphabet *sigma, T t) { 
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

    friend bool operator&&(bool b, term_equality_wrapper const& w) {
      return b && bool(w);
    }
    friend bool operator&&(term_equality_wrapper const& w, bool b) {
      return bool(w) && b;
    }
    friend bool operator||(bool b, term_equality_wrapper const& w) {
      return b || bool(w);
    }
    friend bool operator||(term_equality_wrapper const& w, bool b) {
      return bool(w) || b;
    }

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
      t1.unique_id() == t2.unique_id(), equal<S>(t1, t2)
    };
  }

  template<is_term T1, is_term T2>
  auto operator!=(T1 t1, T2 t2) {
    using S = deduce_fragment_for_storage_t<syntax_element::not_equal, T1, T2>;

    return term_equality_wrapper<S, syntax_element::not_equal>{
      t1.unique_id() != t2.unique_id(), not_equal<S>(t1, t2)
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
    return equal(wrap_term_op_arg(sigma, arg1), wrap_term_op_arg(sigma, arg2));
  }
  
  template<term_op_arg Arg1, term_op_arg Arg2>
    requires (!std::is_class_v<Arg1> || !std::is_class_v<Arg2>)
  auto operator !=(Arg1 arg1, Arg2 arg2) {
    alphabet *sigma = get_sigma(arg1, arg2);
    return 
      not_equal(wrap_term_op_arg(sigma, arg1), wrap_term_op_arg(sigma, arg2));
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
  declare_binary_formula_op(operator&&, conjunction)
  declare_binary_formula_op(operator||, disjunction)
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

    static auto identity(alphabet *sigma) {
      return constant{sigma->zero()};
    }
  };
  
  template<typename Derived>
  struct hierarchy_element_custom_members<
    syntax_element::multiplication, Derived
  > { 
    auto operands() const;

    static auto identity(alphabet *sigma) {
      return constant{sigma->one()};
    }
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
        typename R::syntax, make_fragment_t<Op>, typename decltype(id)::syntax
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
  
  template<std::ranges::range Range, typename F>
  auto sum(alphabet &sigma, Range const& r, F&& f) {
    return fold_op<syntax_element::addition>(
      sigma, r, std::forward<F>(f)
    );
  }
  
  template<std::ranges::range Range, typename F>
  auto product(alphabet &sigma, Range const& r, F&& f) {
    return fold_op<syntax_element::multiplication>(
      sigma, r, std::forward<F>(f)
    );
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
        hierarchy_of_storage_v<storage_of_element_v<E::element>>
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
    bool operator==(const_iterator const&it) const = default;

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
  // We also provide some accessor functions to `quantifier` that help matching
  // quantifier blocks instead of one quantifier at the time. Algorithmically,
  // this is conceptually simpler because we just have to recurse on the
  // children of the quantifier until we fine something different from the same
  // kind of quantifier. However, the things are a bit complex because we want
  // to support different use cases. See the comments above `quantifier_block`.
  //
  // We start from the lazy view over the variables of a quantifier block. The
  // view itself is a thin wrapper over the quantifier, that returns its
  // iterators at begin() and end().
  //
  template<fragment Syntax>
  class quantifier_block_view : public std::ranges::view_base {
  public:
    class const_iterator;
    using iterator = const_iterator;

    quantifier_block_view() = default;
    quantifier_block_view(quantifier<Syntax> q) : _quantifier{q} { }

    const_iterator begin() const { 
      black_assert(_quantifier.has_value());
      return const_iterator{*_quantifier}; 
    }
    const_iterator end() const { return const_iterator{}; }

  private:
    std::optional<quantifier<Syntax>> _quantifier;
  };

  // forward declaration
  template<fragment Syntax>
  class quantifier_block;

  //
  // The iterator is simpler than the one of `associative_op_view`, because of
  // what we said above.
  //
  template<fragment Syntax>
  class quantifier_block_view<Syntax>::const_iterator {
  public:
    using difference_type = ssize_t;
    using value_type = variable;

    // All due constructors. A default-constructed iterator equals to end()
    // because `_quantifier` is empty.
    const_iterator() = default;
    const_iterator(const_iterator const&) = default;
    const_iterator(const_iterator &&) = default;

    // constructor used by the view in begin()
    const_iterator(quantifier<Syntax> q) : _quantifier{q} { }

    const_iterator &operator=(const_iterator const&) = default;
    const_iterator &operator=(const_iterator &&) = default;

    bool operator==(const_iterator const&) const = default;

    // the increment just goes down a level into the `matrix()` of the current
    // quantifier. If we reach something that is not a quantifier or not a
    // quantifier of the right type, we set `_quantifier` to `nullopt` so we
    // become the end() iterator.
    const_iterator &operator++() {
      black_assert(_quantifier.has_value());
      auto t = _quantifier->node_type();

      auto child = _quantifier->matrix().template to<quantifier<Syntax>>();
      if(child.has_value() && child->node_type() == t)
        _quantifier = child;
      else
        _quantifier = std::nullopt;

      return *this;
    }

    // post-increment
    const_iterator operator++(int) {
      auto o = *this;
      operator++();
      return o;
    }

    // Here we just return the variable of the current quantifier
    variable operator*() const {
      black_assert(_quantifier.has_value());
      return _quantifier->var();
    }

  private:
    std::optional<quantifier<Syntax>> _quantifier;
  };

  //
  // The following class represents a block of quantifiers of the same kind. It
  // is a `storage_kind` in itself, of the same `storage_type` as
  // `quantifier<>`. This means we can convert from a quantifier block to a
  // quantifier freely, and viceversa. The accessors lazily go into the
  // quantifier to look for the variables and the matrix.
  //
  // This supports various use cases:
  // 1. you use quantifier_block's second constructor to build a chain of
  //    quantifier objects from a vector (a range) of variables and a matrix.
  // 2. you use the class as a result of `quantifier<>::block()` to view the
  //    quantifier as a quantifier block.
  // 3. you use `quantifier_block` instead of `quantifier` in a pattern matching
  //    case to use directly the quantifier block without having to match a
  //    `quantifier` and then call `block()` each time.
  //
  // first we declare some utility functions. This function creates a chain of
  // quantifiers given a matrix and a range of variables. `Q` is either
  // `quntifier<>`, `exists<>` or `forall<>`
  template<typename Q, std::ranges::range R>
  Q create_block(
    typename quantifier<typename Q::syntax>::type t, 
    R const&vars, formula<typename Q::syntax> matrix
  ) {
    black_assert(begin(vars) != end(vars));
    for(auto it = rbegin(vars); it != rend(vars); ++it) {
      matrix = quantifier<typename Q::syntax>(t, *it, matrix);
    }
    return *matrix.template to<Q>();
  }

  //
  // This function takes a `quantifier` that heads a quantifier chain and looks
  // for the innermost `quantifier` of the chain.
  //
  template<fragment Syntax>
  quantifier<Syntax> innermost_quantifier(quantifier<Syntax> q)
  {
    while(q.matrix().template is<quantifier<Syntax>>())
      q = *q.matrix().template to<quantifier<Syntax>>();
    return q;
  }
  
  //
  // Now the `quantifier_block` class itself.
  //
  template<fragment Syntax>
  class quantifier_block 
    : public
      storage_base<storage_type::quantifier, Syntax, quantifier_block<Syntax>>
  {
    using base_t = storage_base<
      storage_type::quantifier, Syntax, quantifier_block<Syntax>
    >;
  public:
    //
    // converting constructors from general and specific quantifiers
    //
    quantifier_block(quantifier<Syntax> q)
      : base_t{q.sigma(), q.node()} { }
    quantifier_block(exists<Syntax> q) 
      : base_t{q.sigma(), q.node()} { }
    quantifier_block(forall<Syntax> q) 
      : base_t{q.sigma(), q.node()} { }

    //
    // allocating constructor, mimicking the one of `quantifier<>`. Here we take
    // a range of variables instead of a single variable, and build the chain of
    // quantifiers by calling `create_block()`. We also already set `_last`
    // because we already know the block's matrix.
    //
    template<std::ranges::range R>
    quantifier_block(
      typename quantifier<Syntax>::type t,
      R const&vars, 
      formula<Syntax> matrix
    ) : quantifier_block{create_block<quantifier<Syntax>>(t, vars, matrix)} 
    { 
      black_assert(!empty(vars));
      _last = quantifier<Syntax>(t, *rbegin(vars), matrix);
    }

    // returns the view to the variables.
    quantifier_block_view<Syntax> variables() const {
      return {*this};
    }

    //
    // Here we return the matrix of the block. To avoid having to traverse the
    // block each time the function is called, we cache the result into the
    // mutable variable `_last`, which contains the innermost quantifier of the
    // block.
    formula<Syntax> matrix() const {
      if(_last)
        return _last->matrix();
      _last = innermost_quantifier<Syntax>(*this);
      return _last->matrix();
    }

  private:
     mutable std::optional<quantifier<Syntax>> _last;
  };

  //
  // Similar to `quantifier_block<>`, `exists_block<>` and `forall_block<>` do
  // the same thing but for specific quantifiers. Here we declare a common base
  // class for both. The interface and member functions are the same of
  // `quantifier_block`, excepting that here we derive from
  // `hierarchy_element_base`, so we are perfectly compatible with `exists<>`
  // and `forall<>`.
  //
  template<syntax_element E, fragment Syntax>
  class specific_quantifier_block 
    : public
      hierarchy_element_base<E, Syntax, specific_quantifier_block<E, Syntax>>
  {
    using element_t = element_type_of_t<Syntax, E>;
    using base_t = 
      hierarchy_element_base<E, Syntax, specific_quantifier_block<E, Syntax>>;
  public:
    specific_quantifier_block(element_t q) 
        : base_t{q.sigma(), q.node()} { }

    template<std::ranges::range R>
    specific_quantifier_block(
      R const&vars, 
      formula<Syntax> matrix
    ) : specific_quantifier_block{
      create_block<element_t>(fragment_enum_value<E>{}, vars, matrix)
    } { 
      black_assert(!empty(vars));
      _last = element_t(*rbegin(vars), matrix);
    }
    
    specific_quantifier_block(
      std::initializer_list<variable> const&vars,
      formula<Syntax> matrix
    ) : specific_quantifier_block{
      create_block<element_t>(fragment_enum_value<E>{}, vars, matrix)
    } { 
      black_assert(!empty(vars));
      _last = element_t(*rbegin(vars), matrix);
    }

    quantifier_block_view<Syntax> variables() const {
      return {*this};
    }

    formula<Syntax> matrix() const {
      if(_last)
        return _last->matrix();
      _last = innermost_quantifier<Syntax>(*this);
      return _last->matrix();
    }

  private:
    mutable std::optional<quantifier<Syntax>> _last;
  };

  //
  // The specific classses just derive from `specific_quantifier_block` without
  // anything else.
  //
  template<fragment Syntax>
  struct exists_block 
    : specific_quantifier_block<syntax_element::exists, Syntax> 
  {
    using base_t = specific_quantifier_block<syntax_element::exists, Syntax>;
    using base_t::base_t;
  };

  template<fragment Syntax>
  struct forall_block 
    : specific_quantifier_block<syntax_element::forall, Syntax> 
  {
    using base_t = specific_quantifier_block<syntax_element::forall, Syntax>;
    using base_t::base_t;
  };

  //
  // Finally, we can add our custom member `block()` to `quantifier<>`,
  // `exists<>` and `forall<>`.
  //
  template<fragment Syntax>
  struct storage_custom_members<storage_type::quantifier, quantifier<Syntax>>
  {
    quantifier_block<Syntax> block() const { 
      return {static_cast<quantifier<Syntax> const&>(*this)}; 
    }
  };
  
  template<fragment Syntax>
  struct hierarchy_element_custom_members<
    syntax_element::exists, exists<Syntax>
  > {
    exists_block<Syntax> block() const { 
      return {static_cast<quantifier<Syntax> const&>(*this)}; 
    }
  };
  
  template<fragment Syntax>
  struct hierarchy_element_custom_members<
    syntax_element::forall, forall<Syntax>
  > {
    forall_block<Syntax> block() const { 
      return {static_cast<quantifier<Syntax> const&>(*this)}; 
    }
  };
}

#endif // BLACK_LOGIC_SUGAR_HPP_
