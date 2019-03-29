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
  #error "This header file cannot included alone, "\
         "please include <black/logic/formula.hpp> instead"
#endif

#include <black/support/common.hpp>

#include <type_traits>
#include <array>
#include <cstdint>
#include <string>
#include <optional>

namespace black {
  class alphabet;
}

namespace black::details
{
  class formula;

  // No virtual destructor, but remember to use static deleter in class alphabet
  struct formula_base
  {
    enum class formula_type : uint8_t {
      boolean,
      atom,
      unary,
      binary
    };

    class alphabet &alphabet;
    formula_type type{};
  };

  struct boolean_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::boolean;

    boolean_t(class alphabet &sigma, bool v)
      : formula_base{sigma, formula_type::boolean}, value(v) {}

    bool value{};
  };

  struct atom_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::atom;

    atom_t(class alphabet &_sigma, any_hashable const& _label)
      : formula_base{_sigma, formula_type::atom}, label{_label} {}

    any_hashable label;
  };

  struct unary_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::unary;

    enum operator_type : uint8_t {
      negation = 0,
      tomorrow,
      yesterday,
      always,
      eventually,
      past,
      historically
    };

    unary_t(class alphabet &sigma, operator_type ot, formula_base const*f)
      : formula_base{sigma, formula_type::unary}, op_type{ot}, operand{f} {
      black_assert(f != nullptr);
    }

    operator_type op_type;
    formula_base const*operand;
  };

  struct binary_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::binary;

    enum operator_type : uint8_t {
      conjunction = 0,
      disjunction,
      then,
      iff,
      until,
      release,
      since,
      triggered
    };

    binary_t(class alphabet &sigma, operator_type type,
              formula_base const*f1, formula_base const*f2)
      : formula_base{sigma, formula_type::binary}, op_type(type),
        left{f1}, right{f2} {
      black_assert(f1 != nullptr);
      black_assert(f2 != nullptr);
    }

    operator_type op_type;
    formula_base const*left;
    formula_base const*right;
  };

  template<typename T, typename F = std::remove_cv_t<std::remove_pointer_t<T>>>
  F const*formula_cast(formula_base const*f)
  {
    black_assert(f != nullptr);
    if(f->type == F::formula_type)
      return static_cast<F const*>(f);
    return nullptr;
  }

  /*
   * Dummy type for default case in formula matchers
   */
  struct otherwise {};

  // CRTP base class for handles
  template<typename H, typename F>
  struct handle_base
  {
    friend class formula;
    friend class alphabet;

    handle_base(handle_base const&) = default;
    handle_base(handle_base &&) = default;

    explicit handle_base(F const*f) : _formula{f} {
      black_assert(_formula);
    }

    handle_base &operator=(handle_base const&h) = default;

    operator otherwise() const { return {}; }

    operator formula() const;

  protected:
    using handled_formula_t = F;

    static optional<H> cast(formula_base const*f) {
      if(auto ptr = formula_cast<typename H::handled_formula_t const*>(f); ptr)
        return optional<H>{H{ptr}};
      return nullopt;
    }

    // Implemented after alphabet class
    template<typename Arg, typename ...Args>
    static F *allocate_formula(Arg&&, Args&& ...);

    F const*_formula;
  };

  /*
   * trait to recognise handles
   */
  template<typename T>
  class is_handle_t
  {
    template<typename H, typename F>
    static std::true_type detect(handle_base<H, F> const&) {}
    static std::false_type detect(...) {}

  public:
    static const bool value = decltype(detect(std::declval<T>()))::value;
  };

  template<typename T>
  constexpr bool is_handle = is_handle_t<T>::value;

  // Encoding the arity of each operator in the enum value
  // Assumes the above three arities, so 2 bits to store a formula_arity value
  constexpr uint8_t type_encoding_shift = 6;

  enum class formula_type : uint8_t;

  template<typename T>
  struct formula_base_type_of_op {};

  template<>
  struct formula_base_type_of_op<unary_t::operator_type> {
    static const auto value = formula_base::formula_type::unary;
  };

  template<>
  struct formula_base_type_of_op<binary_t::operator_type> {
    static const auto value = formula_base::formula_type::binary;
  };

  template<typename R = formula_type, typename T>
  constexpr R op_to_formula_type(T op_type) {
    return static_cast<R>(
      (to_underlying(formula_base_type_of_op<T>::value) << type_encoding_shift)
        & to_underlying(op_type)
    );
  }

  template<typename R = formula_type>
  constexpr formula_type op_to_formula_type(formula_base::formula_type type) {
    return static_cast<R>(type);
  }

  #define enum_unary unary_t::operator_type
  #define enum_binary binary_t::operator_type
  #define declare_type(Name, Type) \
    Name = op_to_formula_type<uint8_t>(enum_##Type::Name)

  enum class formula_type : uint8_t {
    boolean = to_underlying(formula_base::formula_type::boolean),
    atom    = to_underlying(formula_base::formula_type::atom),
    declare_type(negation,     unary),
    declare_type(tomorrow,     unary),
    declare_type(yesterday,    unary),
    declare_type(always,       unary),
    declare_type(eventually,   unary),
    declare_type(past,         unary),
    declare_type(historically, unary),
    declare_type(conjunction,  binary),
    declare_type(disjunction,  binary),
    declare_type(then,         binary),
    declare_type(iff,          binary),
    declare_type(until,        binary),
    declare_type(release,      binary),
    declare_type(since,        binary),
    declare_type(triggered,    binary)
  };

  #undef declare_type
  #undef enum_binary
  #undef enum_unary

} // namespace black::details

#endif // BLACK_LOGIC_FORMULA_BASE_HPP_
