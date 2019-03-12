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

#ifndef BLACK_LOGIC_HPP_
#define BLACK_LOGIC_HPP_

#include <type_traits>

namespace black::details {

// Base formula type.
//
// All the specific types of formulae, derived from this base, are declared
// at the bottom of the file with the NULLARY, UNARY, or BINARY macros
// depending on the type of connective they represent.

template<typename T>
struct is_formula_t : std::false_type {};

template<typename T>
struct formula_type_t {};

template<typename T>
struct is_nullary_formula_t : std::false_type {};

template<typename T>
struct is_unary_formula_t : std::false_type {};

template<typename T>
struct is_binary_formula_t : std::false_type {};

template<typename T>
constexpr bool is_formula = is_formula_t<T>::value;

template<typename T>
constexpr auto formula_type = formula_type_t<T>::value;

template<typename T>
constexpr bool is_nullary_formula = is_nullary_formula_t<T>::value;

template<typename T>
constexpr bool is_unary_formula = is_unary_formula_t<T>::value;

template<typename T>
constexpr bool is_binary_formula = is_binary_formula_t<T>::value;

class alphabet;

class formula_t
{
public:
  enum type_t {
    falsity = 0,
    truth,
    atom,
    negation,
    tomorrow,
    yesterday,
    conjunction,
    disjunction,
    then,
    iff,
    until,
    release,
    always,
    eventually,
    past,
    historically,
    since,
    triggered
  };

  formula_t(formula_t const&) = delete;

protected:
  formula_t(alphabet &alphabet, type_t t) : _alphabet(alphabet), _type(t) {}

public:
  type_t type() const { return _type; }
  class alphabet &alphabet() const { return _alphabet; }

private:
  class alphabet &_alphabet;
  type_t _type;
};

class atom_t;

class atom_wrapper;

template<typename>
class nullary_wrapper;

template<typename>
class unary_wrapper;

template<typename>
class binary_wrapper;

template<typename F>
using wrapper_base =
  std::conditional_t<std::is_same_v<F,atom_t>,
  atom_wrapper,
  std::conditional_t<is_nullary_formula<F>,
    nullary_wrapper<F>,
    std::conditional_t<is_unary_formula<F>,
      unary_wrapper<F>,
      binary_wrapper<F>>>>;

template<typename F>
class wrapper : public wrapper_base<F> { using wrapper_base<F>::wrapper_base; };

template<typename F>
class nullary_wrapper {
  static_assert(is_formula<F>);

  friend class formula;

  friend class atom_wrapper;

  template<typename T>
  friend class nullary_wrapper;

  template<typename T>
  friend class unary_wrapper;

  template<typename T>
  friend class binary_wrapper;

public:
  explicit nullary_wrapper(F *f) : _formula(f) {}
  nullary_wrapper(nullary_wrapper const&) = default;

  alphabet &alphabet() const { return _formula->alphabet(); }

protected:
  F *_formula = nullptr;
};

template<typename F>
class unary_wrapper {
  static_assert(is_formula<F>);

  friend class formula;

  friend class atom_wrapper;

  template<typename T>
  friend class nullary_wrapper;

  template<typename T>
  friend class unary_wrapper;

  template<typename T>
  friend class binary_wrapper;

public:
  explicit unary_wrapper(F *f) : _formula(f) {}
  unary_wrapper(unary_wrapper const&) = default;

  template<typename F1, REQUIRES(is_formula<F1>)>
  unary_wrapper(wrapper<F1> f) {
    *this = f.alphabet().template create<unary_wrapper<F>>(f._formula);
    black_assert(_formula);
  }

  alphabet &alphabet() const { return _formula->alphabet(); }
  formula_t *lhs() const { return _formula->lhs; }

protected:
  F *_formula = nullptr;
};

template<typename F>
class binary_wrapper {
  static_assert(is_formula<F>);

  friend class formula;

  friend class atom_wrapper;

  template<typename T>
  friend class nullary_wrapper;

  template<typename T>
  friend class unary_wrapper;

  template<typename T>
  friend class binary_wrapper;

public:
  explicit binary_wrapper(F *f) : _formula(f) {}
  binary_wrapper(binary_wrapper const&) = default;

  template<typename F1, typename F2, REQUIRES(is_formula<F1>, is_formula<F2>)>
  binary_wrapper(wrapper<F1> f1, wrapper<F2> f2) {
    black_assert(&f1.alphabet() == &f2.alphabet());

    *this = f1.alphabet().template create<binary_wrapper<F>>(f1._formula,
                                                             f2._formula);
    black_assert(_formula);
  }

  alphabet &alphabet() const { return _formula->alphabet(); }
  formula_t *lhs() const { return _formula->lhs; }
  formula_t *rhs() const { return _formula->rhs; }

protected:
  F *_formula = nullptr;
};

template<typename W>
struct unwrap_t;

template<template<typename> class W, typename T>
struct unwrap_t<W<T>> {
  using type = T;
};

template<typename W>
using unwrap = typename unwrap_t<W>::type;

#define REGISTER_FORMULA(Name, Arity)                                 \
  template<>                                                          \
  struct is_formula_t<Name##_t> : std::true_type {};                  \
                                                                      \
  template<>                                                          \
  struct formula_type_t<Name##_t> :                                   \
    std::integral_constant<formula_t::type_t, formula_t::Name> {};    \
                                                                      \
  template<>                                                          \
  struct is_##Arity##_formula_t<Name##_t> : std::true_type {};        \
                                                                      \
  using Name = wrapper<Name##_t>;                                     \
} namespace black { using details::Name; } namespace black::details {

#define NULLARY(Name)                                       \
  class Name##_t : public formula_t {                       \
  public:                                                   \
    Name##_t(class alphabet &a)                             \
    : formula_t(a, formula_t::Name) {}                      \
  };                                                        \
  REGISTER_FORMULA(Name, nullary)

#define UNARY(Name)                           \
  class Name##_t : public formula_t {         \
  public:                                     \
    Name##_t(class alphabet &a, formula_t *l) \
    : formula_t(a, formula_t::Name), lhs(l) { \
      black_assert(lhs);                      \
    }                                         \
                                              \
    formula_t *lhs;                           \
  };                                          \
  REGISTER_FORMULA(Name, unary)

#define BINARY(Name)                                        \
  class Name##_t : public formula_t {                       \
  public:                                                   \
    Name##_t(class alphabet &a, formula_t *l, formula_t *r) \
    : formula_t(a, formula_t::Name), lhs(l), rhs(r) {       \
        black_assert(lhs);                                  \
        black_assert(rhs);                                  \
      }                                                     \
                                                            \
    formula_t *lhs;                                         \
    formula_t *rhs;                                         \
  };                                                        \
  REGISTER_FORMULA(Name, binary)

NULLARY(falsity)
NULLARY(truth)
UNARY(negation)
UNARY(tomorrow)
UNARY(yesterday)
UNARY(always)
UNARY(eventually)
UNARY(past)
UNARY(historically)
BINARY(conjunction)
BINARY(disjunction)
BINARY(then)
BINARY(iff)
BINARY(until)
BINARY(release)
BINARY(since)
BINARY(triggered)

// The atom class is special because of the key field
class atom_t : public formula_t {
public:
  atom_t(class alphabet &a, std::string const&name)
  : formula_t(a, formula_t::atom), _name(name) {}

  std::string_view name() const { return _name; }

private:
  std::string _name;
};

class atom_wrapper
{
  friend class formula;

  template<typename T>
  friend class nullary_wrapper;

  template<typename T>
  friend class unary_wrapper;

  template<typename T>
  friend class binary_wrapper;

public:
  explicit atom_wrapper(atom_t *f) : _formula(f) {}
  atom_wrapper(atom_wrapper const&) = default;

  class alphabet &alphabet() const { return _formula->alphabet(); }
  std::string_view name() const { return _formula->name(); }

private:
  atom_t *_formula;
};
REGISTER_FORMULA(atom, nullary)

#undef REGISTER_FORMULA
#undef NULLARY
#undef UNARY
#undef BINARY

class formula {
public:
  formula(formula const&) = default;
  formula(formula_t *f) : _formula(f) {}

  template<typename F, REQUIRES(is_formula<F>)>
  formula(wrapper<F> w) : _formula(w._formula) {}

  template<typename W, typename F = unwrap<W>, REQUIRES(is_formula<F>)>
  bool isa() const {
    return _formula->type() == formula_type<F>;
  }

  template<typename W, typename F = unwrap<W>, REQUIRES(is_formula<F>)>
  optional<W> cast() const {
    if(isa<W>())
      return optional{W{static_cast<F*>(_formula)}};
    return nullopt;
  }

  template<typename... Func>
  auto match(Func &&...func)
  {
    #define MATCH_CASE(Name) \
      case formula_t::Name: { \
        optional<Name> casted = cast<Name>(); \
        black_assert(casted); \
        return std::invoke( \
          overloaded { std::forward<Func>(func)...}, \
          casted.value() \
        ); \
      } \

    switch(_formula->type())
    {
      MATCH_CASE(falsity)
      MATCH_CASE(truth)
      MATCH_CASE(atom)
      MATCH_CASE(negation)
      MATCH_CASE(tomorrow)
      MATCH_CASE(yesterday)
      MATCH_CASE(conjunction)
      MATCH_CASE(disjunction)
      MATCH_CASE(then)
      MATCH_CASE(iff)
      MATCH_CASE(until)
      MATCH_CASE(release)
      MATCH_CASE(always)
      MATCH_CASE(eventually)
      MATCH_CASE(past)
      MATCH_CASE(historically)
      MATCH_CASE(since)
      MATCH_CASE(triggered)
    }

    black_unreachable();
    #undef MATCH_CASE
  }

private:
  formula_t *_formula;
};

// operators to combine formulas

template<typename F>
negation operator !(wrapper<F> w1) {
  return negation{w1};
}

#define BINARY_OP(Op, Ctor)                          \
  template<typename F1, typename F2>                 \
  Ctor operator Op(wrapper<F1> w1, wrapper<F2> w2) { \
    return Ctor{w1, w2};                             \
  }

BINARY_OP(and,conjunction)
BINARY_OP(or,disjunction)

} // namespace black::details

namespace black {
  using details::formula;
}

#endif // BLACK_LOGIC_HPP_
