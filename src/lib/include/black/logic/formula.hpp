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

#ifndef BLACK_LOGIC_FORMULA_HPP_
#define BLACK_LOGIC_FORMULA_HPP_

#include <optional>

#include <black/internal/formula_base.hpp>

namespace black::details
{
  class formula
  {
  public:
    //
    // Constructors. Default construction is forbidden.
    // Formulas can only be created from methods of class `alphabet`,
    // or by combining other formulas
    //
    formula() = delete;
    formula(formula const&) = default;
    formula(formula &&) = default;

    // Equality operators.
    // Two `formula` objects compare equal iff they represent two
    // syntactically identical formulas
    friend bool operator==(formula f1, formula f2);
    friend bool operator!=(formula f1, formula f2);

    // Default assignment operators.
    formula &operator=(formula const&) = default;
    formula &operator=(formula &&) = default;

    // Enumeration of possible kinds of formulas
    using type = formula_type;

    // Gets to the type of the represented formula
    type formula_type() const;

    //
    // The to() function converts a formula into a specific kind of
    // formula type. For example:
    //
    // std::optional<tomorrow> xev = formula.to<tomorrow>();
    //
    // The result is empty if the given type is wrong.
    template<typename H>
    std::optional<H> to() const;

    // The is() function asks whether the formula is of a given specific type
    // Given a formula `f`,
    // `f.to<T>()` is guaranteed to be non-empty iff `f.is<T>()` returns true.
    template<typename H>
    bool is() const;

    //
    // Pattern matching function on formulas. See the docs for examples.
    //
    template<typename ...Cases>
    auto match(Cases&&...) const;

    // Hashing function for formulas, for use in hash tables.
    // Used for the implementation of `std::hash<formula>`, rarely needed alone.
    size_t hash() const;

  private:
    alphabet *_alphabet; // the alphabet the formula comes from
    formula_base const*_formula; // concrete object representing the formula

    template<typename, typename>
    friend struct handle_base;

  // Public constructor, but for internal use
  public:
    explicit formula(alphabet *sigma, formula_base const*f)
      : _alphabet{sigma}, _formula{f} { black_assert(f != nullptr); }
  };

  /*
   * handles for specific types of formulas
   */
  struct boolean : handle_base<boolean, boolean_t>
  {
    // inheriting base class constructors (for internal use)
    using handle_base<boolean, boolean_t>::handle_base;

    // The boolean value
    // (true for alphabet::top(), false for alphabet::bottom())
    bool value() const { return _formula->value; }
  };

  struct atom : handle_base<atom, atom_t>
  {
    // inheriting base class constructors (for internal use)
    using handle_base<atom, atom_t>::handle_base;

    // The generic label of the atom
    std::any label() const;

    // The label of the atom, as a specific type
    // The result is empty if the type is wrong.
    template<typename T>
    std::optional<T> label() const;
  };

  struct unary : handle_base<unary, unary_t>
  {
    // inheriting base class constructors (for internal use)
    using handle_base<unary, unary_t>::handle_base;

    // enumeration of possible unary operators
    using operator_type = unary_t::operator_type;

    // Constructor: takes the type of operator and the argument formula
    unary(operator_type t, formula f);

    // Actual type of unary operator represented by the object
    operator_type type() const;

    // Argument of the operator
    formula operand() const;
  };

  struct binary : handle_base<binary, binary_t>
  {
    // inheriting base class constructors (for internal use)
    using handle_base<binary, binary_t>::handle_base;

    // enumeration of possible binary operators
    using operator_type = binary_t::operator_type;

    // Constructor: takes the type of operator and the two argument formulas
    binary(operator_type t, formula f1, formula f2);

    // Actual type of binary operator represented by the object
    operator_type type() const;

    // Left hand side of the binary operator
    formula left() const;

    // Right hand side of the binary operator
    formula right() const;
  };

  //
  // Specific types for each single type of supported operator,
  // with statically known type of operator.
  //
  // They have the exact same public interface as,
  // respectively, the `unary` and `binary` types declared above.
  //

  // Unary operators. Same public interface as struct `unary`
  struct negation;
  struct tomorrow;
  struct yesterday;
  struct always;
  struct eventually;
  struct past;
  struct historically;

  // Binary operators. Same public interface as struct `binary`
  struct conjunction;
  struct disjunction;
  struct then;
  struct iff;
  struct until;
  struct release;
  struct since;
  struct triggered;

  //
  // Operators.
  // These provides for a natural syntax for building formulas.
  //
  // Besides the proper operator, we also provide a number of helper functions
  // that, similarly to prefix operators, can be use as nice shorthands to build
  // formulas.
  //
  // Note that, despite the very short names, there is no risk of name clashes
  // with user code, because these names are *not* exported by the `black`
  // namespace, but are found by argument-dependent lookup.
  //
  auto operator !(formula f);
  auto operator &&(formula f1, formula f2);
  auto operator ||(formula f1, formula f2);

  auto X(formula f);
  auto Y(formula f);
  auto F(formula f);
  auto G(formula f);
  auto P(formula f);
  auto H(formula f);

  auto U(formula f1, formula f2);
  auto R(formula f1, formula f2);
  auto S(formula f1, formula f2);
  auto T(formula f1, formula f2);

  auto XF(formula f);
  auto XG(formula f);
  auto GF(formula f);
  auto YP(formula f);
  auto YH(formula f);
}

// Names exported from the `black` namespace
namespace black {
  using details::boolean;
  using details::atom;
  using details::unary;
  using details::binary;
  using details::formula;
  using details::otherwise;
}

#include <black/internal/formula_impl.hpp>

#endif // BLACK_LOGIC_FORMULA_HPP_
