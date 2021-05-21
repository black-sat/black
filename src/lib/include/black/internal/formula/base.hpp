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

#ifndef BLACK_LOGIC_FORMULA_BASE_HPP_
#define BLACK_LOGIC_FORMULA_BASE_HPP_

#ifndef BLACK_LOGIC_FORMULA_HPP_
  #error "This header file cannot be included alone, "\
         "please include <black/logic/formula.hpp> instead"
#endif

#include <black/support/meta.hpp>
#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#include <type_traits>
#include <array>
#include <cstdint>
#include <string>
#include <optional>

namespace black::internal
{
  class alphabet; // forward declaration, declared in alphabet.hpp
  class formula;

  constexpr bool is_boolean_type(formula_type type) {
    return type == formula_type::boolean;
  }

  constexpr bool is_atom_type(formula_type type) {
    return type == formula_type::atom;
  }

  constexpr bool is_unary_type(formula_type type) {
    return to_underlying(type) >= to_underlying(formula_type::negation) &&
           to_underlying(type) <= to_underlying(formula_type::historically);
  }

  constexpr bool is_binary_type(formula_type type) {
    return to_underlying(type) >= to_underlying(formula_type::conjunction) &&
           to_underlying(type) <= to_underlying(formula_type::triggered);
  }

  struct formula_base
  {
    formula_base(formula_type t);

    const formula_type type{};
  };

  struct boolean_t : formula_base
  {
    static constexpr auto accepts_type = is_boolean_type;

    boolean_t(bool v)
      : formula_base{formula_type::boolean}, value(v) {}

    bool value{};
  };

  struct atom_t : formula_base
  {
    static constexpr auto accepts_type = is_atom_type;

    atom_t(any_hashable const& _label)
      : formula_base{formula_type::atom}, label{_label} {}

    any_hashable label;
  };

  struct unary_t : formula_base
  {
    static constexpr auto accepts_type = is_unary_type;

    unary_t(formula_type t, formula_base *f)
      : formula_base{t}, operand{f}
    {
      black_assert(is_unary_type(t));
      black_assert(f != nullptr);
    }

    formula_base *operand;
  };

  struct binary_t : formula_base
  {
    static constexpr auto accepts_type = is_binary_type;

    binary_t(formula_type t, formula_base*f1, formula_base*f2)
      : formula_base{t}, left{f1}, right{f2}
    {
      black_assert(is_binary_type(t));
      black_assert(f1 != nullptr);
      black_assert(f2 != nullptr);
    }

    formula_base*left;
    formula_base*right;
  };

  template<typename T, typename F = std::remove_pointer_t<T>>
  F *formula_cast(formula_base *f)
  {
    black_assert(f != nullptr);
    if(F::accepts_type(f->type))
      return static_cast<F *>(f);
    return nullptr;
  }

  /*
   * Dummy type for default case in formula matchers
   */
  struct otherwise {};

  /*
   * Unique opaque id for formulas
   */
  enum class formula_id : uintptr_t;

  // CRTP base class for handles
  template<typename H, typename F>
  struct handle_base
  {
    friend class formula;
    friend class alphabet;

    handle_base(alphabet *sigma, F *f) 
      : _alphabet{sigma}, _formula{f}
    { black_assert(_formula); }

    // This constructor takes a tuple instead of two arguments in order to
    // directly receive the return value of allocate_formula() below
    handle_base(std::pair<alphabet *, F *> args)
      : handle_base{args.first, args.second} { }

    operator otherwise() const { return {}; }

    operator formula() const;

    alphabet *alphabet() const;

    formula_id unique_id() const;

  protected:
    using handled_formula_t = F;

    static std::optional<H> cast(class alphabet *sigma, formula_base *f) {
      if(auto ptr = formula_cast<typename H::handled_formula_t *>(f); ptr)
        return std::optional<H>{H{sigma, ptr}};
      return std::nullopt;
    }

    // Implemented after alphabet class
    template<typename FType, typename Arg>
    static std::pair<class alphabet *, unary_t *>
    allocate_unary(FType type, Arg const&arg);

    template<typename FType, typename Arg1, typename Arg2>
    static std::pair<class alphabet *, binary_t *>
    allocate_binary(FType type, Arg1 const&arg1, Arg2 const&arg2);

    class alphabet *_alphabet;
    F *_formula;
  };

} // namespace black::internal

#endif // BLACK_LOGIC_FORMULA_BASE_HPP_
