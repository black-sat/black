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

#include <black/support/common.hpp>

#include <type_traits>
#include <array>
#include <cstdint>
#include <string>
#include <optional>


namespace black::details
{
  class formula_storage; // forward declaration

  // No virtual destructor, but remember to use static deleter in class alphabet
  struct formula_base
  {
    enum class formula_type : uint8_t {
      boolean,
      atom,
      unary,
      binary
    };

    formula_storage &alphabet;
    formula_type type{};
  };

  struct boolean_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::boolean;

    boolean_t(formula_storage &sigma, bool v)
      : formula_base{sigma, formula_type::boolean}, value(v) {}

    bool value{};
  };

  struct atom_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::atom;

    atom_t(formula_storage &sigma, std::string const& n)
      : formula_base{sigma, formula_type::atom}, name(n) {}

    std::string name;
  };

  struct unary_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::unary;

    enum operator_type : uint8_t {
      negation,
      tomorrow,
      yesterday,
      always,
      eventually,
      past,
      historically
    };

    unary_t(formula_storage &sigma, operator_type ot, formula_base const*f)
      : formula_base{sigma, formula_type::unary}, op_type{ot}, operand{f} { }

    operator_type op_type;
    formula_base const*operand;
  };

  struct binary_t : formula_base
  {
    static const enum formula_type formula_type = formula_type::binary;

    enum operator_type : uint8_t {
      conjunction,
      disjunction,
      then,
      iff,
      until,
      release,
      since,
      triggered
    };

    binary_t(formula_storage &sigma, operator_type type,
              formula_base const*f1, formula_base const*f2)
      : formula_base{sigma, formula_type::binary}, op_type(type),
        left{f1}, right{f2} { }

    operator_type op_type;
    formula_base const*left;
    formula_base const*right;
  };

  template<typename T, typename F = std::remove_cv_t<std::remove_pointer_t<T>>>
  F const*formula_cast(formula_base const*f)
  {
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

    template<typename, typename>
    friend struct handle_base;

    handle_base(handle_base const&) = default;
    handle_base(handle_base &&) = default;

    template<typename H1, typename F1, typename H2, typename F2>
    friend bool
    operator==(handle_base<H1,F1> const&h1, handle_base<H2,F2> const&f2);

    operator otherwise() const { return {}; }

  protected:
    using handled_formula_t = F;

    explicit handle_base(F const*f) : _formula{f} {}

    static optional<H> cast(formula_base const*f) {
      if(auto ptr = formula_cast<typename H::handled_formula_t const*>(f); ptr)
        return optional<H>{H{ptr}};
      return nullopt;
    }

    // Implemented after alphabet class
    template<typename Arg, typename ...Args>
    static F *allocate_formula(Arg&&, Args&& ...);

    template<typename H2>
    static F const* unwrap_handle(handle_base<H2, F> const& h) noexcept {
      return h._formula;
    }

    F const*_formula;
  };

  template<typename H1, typename F1, typename H2, typename F2>
  bool operator==(handle_base<H1,F1> const& h1, handle_base<H2,F2> const& h2) {
    return static_cast<formula_base const*>(h1._formula) ==
           static_cast<formula_base const*>(h2._formula);
  }

  template<typename H1, typename F1, typename H2, typename F2>
  bool operator!=(handle_base<H1,F1> const& h1, handle_base<H2,F2> const& h2) {
    return !(h1 == h2);
  }

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

  template<typename T>
  constexpr uint8_t combine_type(formula_base::formula_type type, T op_type) {
    return
      (to_underlying(type) << type_encoding_shift) & to_underlying(op_type);
  }

  #define enum_unary unary_t::operator_type
  #define enum_binary binary_t::operator_type
  #define declare_type(Name, Type) \
    Name = combine_type(formula_base::formula_type::Type, enum_##Type::Name)

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

  // User-facing formula class
  class formula
  {
    friend class formula_storage;
    template<typename, typename>
    friend struct handle_base;

  public:
    formula() = delete;
    formula(formula const&) = default;

    formula(formula_base const*f) : _formula{f} {}

    using type = formula_type;

    template<typename H, REQUIRES(is_handle<H>)>
    formula(H const&h) : _formula(h._formula) {}

    friend bool operator==(formula f1, formula f2);

    type get_type() const {
      if(auto *unary = formula_cast<unary_t const*>(_formula); unary)
        return formula_type{combine_type(unary->type, unary->op_type)};
      if(auto *binary = formula_cast<binary_t const*>(_formula); binary)
        return formula_type{combine_type(binary->type, binary->op_type)};

      return formula_type{to_underlying(_formula->type)};
    }

    template<typename H>
    optional<H> to() const {
      black_assert(_formula != nullptr);

      return H::cast(_formula);
    }

    template<typename H>
    bool is() const {
      black_assert(_formula != nullptr);
      return H::cast(_formula).has_value();
    }

    template<typename ...Cases>
    auto match(Cases&&...) const;

  private:
    formula_base const*_formula;
  };

  inline bool operator==(formula f1, formula f2) {
    return f1._formula == f2._formula;
  }

  inline bool operator!=(formula f1, formula f2) {
    return !(f1 == f2);
  }

  /*
   * trait to recognise handles and formulas
   */
  template<typename T>
  constexpr bool is_formula = std::is_convertible_v<T,formula> || is_handle<T>;

  /*
   * handles for specific types of formulas
   */
  struct boolean
    : handle_base<boolean, boolean_t>
  {
    friend class formula;
    friend class alphabet;

    using base_t = handle_base<boolean, boolean_t>;
    using base_t::base_t;
    friend base_t;

    bool value() const { return _formula->value; }
  };

  struct atom : handle_base<atom, atom_t>
  {
    friend class formula;
    friend class alphabet;

    using base_t = handle_base<atom, atom_t>;
    using base_t::base_t;
    friend base_t;

    std::string_view name() const { return _formula->name; }
  };

  struct unary
    : handle_base<unary, unary_t>
  {
    friend class formula;
    friend class alphabet;

    using base_t = handle_base<unary, unary_t>;
    using base_t::base_t;
    friend base_t;

    using operator_type = unary_t::operator_type;

    template<typename H>
    unary(handle_base<H,unary_t> const& h)
      : base_t{unwrap_handle(h)} { }

    template<typename F, REQUIRES(is_formula<F>)>
    unary(operator_type t, F const& h) : base_t{allocate_formula(t, h)} { }

    operator_type op_type() const { return _formula->op_type; }

    formula operand() const {
      return formula{_formula->operand};
    }
  };

  struct binary
    : handle_base<binary, binary_t>
  {
    friend class formula;
    friend class alphabet;

    using base_t = handle_base<binary, binary_t>;
    using base_t::base_t;
    friend base_t;

    using operator_type = binary_t::operator_type;

    template<typename H>
    binary(handle_base<H,binary_t> const& h)
      : base_t{unwrap_handle(h)} { }

    template<typename F1, REQUIRES(is_formula<F1>),
             typename F2, REQUIRES(is_formula<F2>)>
    binary(operator_type t, F1 const& h1, F2 const& h2)
      : base_t{allocate_formula(t, h1, h2)} { }

    operator_type op_type() const { return _formula->op_type; }

    formula left() const {
      return formula{_formula->left};
    }

    formula right() const {
      return formula{_formula->right};
    }
  };

  /*
   * Type-specific handles, similar to unary and binary, but with the operator
   * known at compile-type
   */
  template<typename H, typename F, auto OT>
  struct operator_base : handle_base<H, F>
  {
    friend class formula;
    friend class alphabet;

    using base_t = handle_base<H, F>;
    using base_t::base_t;

    formula operand() const { return formula{this->_formula->operand}; }

  protected:
    static optional<H> cast(formula_base const*f) {
      if(auto ptr = formula_cast<F const*>(f); ptr && ptr->op_type == OT)
        return optional<H>{H{ptr}};
      return nullopt;
    }
  };

  template<typename H, unary_t::operator_type OT>
  struct unary_operator : operator_base<H, unary_t, OT>
  {
    using base_t = operator_base<H, unary_t, OT>;
    using base_t::base_t;
    using base_t::cast;

    template<typename F, REQUIRES(is_formula<F>)>
    explicit unary_operator(F const& h) : base_t{this->allocate_formula(OT, h)}
    {
      black_assert(this->_formula->type == formula_base::formula_type::unary);
      black_assert(this->_formula->op_type == OT);
    }
  };

  template<typename H, binary_t::operator_type OT>
  struct binary_operator : operator_base<H, binary_t, OT>
  {
    using base_t = operator_base<H, binary_t, OT>;
    using base_t::base_t;
    using base_t::cast;

    template<typename F1, REQUIRES(is_formula<F1>),
             typename F2, REQUIRES(is_formula<F2>)>
    binary_operator(F1 const& h1, F2 const& h2)
      : base_t{this->allocate_formula(OT, h1, h2)}
    {
      black_assert(this->_formula->type == formula_base::formula_type::binary);
      black_assert(this->_formula->op_type == OT);
    }
  };

  #define declare_unary_operator_methods \
    formula operand() const { 			 \
      return formula{_formula->operand}; \
    }

  #define declare_binary_operator_methods  \
    formula left() const {                 \
      return formula{_formula->left};      \
    }                                      \
                                           \
    formula right() const {                \
      return formula{_formula->right};     \
    }


  #define declare_operator(Op, Arity)                                 \
    struct Op : Arity##_operator<Op, Arity##_t::Op> {                 \
      using base_t = Arity##_operator<Op, Arity##_t::Op>;             \
      using base_t::base_t;                                           \
      friend operator_base<Op, Arity##_t, Arity##_t::Op>;             \
																	  \
      declare_##Arity##_operator_methods 							  \
    };                                                                \
  } namespace black { using details::Op; } namespace black::details { \

  declare_operator(negation,     unary)
  declare_operator(tomorrow,     unary)
  declare_operator(yesterday,    unary)
  declare_operator(always,       unary)
  declare_operator(eventually,   unary)
  declare_operator(past,         unary)
  declare_operator(historically, unary)
  declare_operator(conjunction, binary)
  declare_operator(disjunction, binary)
  declare_operator(then,        binary)
  declare_operator(iff,         binary)
  declare_operator(until,       binary)
  declare_operator(release,     binary)
  declare_operator(since,       binary)
  declare_operator(triggered,   binary)

  #undef declare_operator

  /*
   * Pattern matching on formulas
   */
  template<typename ...Cases>
  auto formula::match(Cases&& ...cases) const
  {
    black_assert(_formula);
    using formula_type = formula_base::formula_type;
    using unary_type = unary_t::operator_type;
    using binary_type = binary_t::operator_type;

    #define match_case(Enum, H)                                       \
      case Enum::H: {                                                 \
        black_assert(is<H>());                                        \
        return apply_first(std::make_tuple(*to<H>()), FWD(cases)...); \
      }

    switch(_formula->type) {
      match_case(formula_type, boolean)
      match_case(formula_type, atom)
      case formula_type::unary:
        switch(formula_cast<unary_t*>(_formula)->op_type) {
          match_case(unary_type, negation)
          match_case(unary_type, tomorrow)
          match_case(unary_type, yesterday)
          match_case(unary_type, always)
          match_case(unary_type, eventually)
          match_case(unary_type, past)
          match_case(unary_type, historically)
        }
      case formula_type::binary:
        switch(formula_cast<binary_t*>(_formula)->op_type) {
          match_case(binary_type, conjunction)
          match_case(binary_type, disjunction)
          match_case(binary_type, then)
          match_case(binary_type, iff)
          match_case(binary_type, until)
          match_case(binary_type, release)
          match_case(binary_type, since)
          match_case(binary_type, triggered)
        }
    }
  }
  #undef match_case

  /*
   * Operators
   */
   template<typename F, REQUIRES(is_formula<F>)>
   auto operator !(F&& f) {
     return negation(FWD(f));
   }

   template<
    typename F1, REQUIRES(is_formula<F1>),
    typename F2, REQUIRES(is_formula<F2>)
    >
   auto operator &&(F1&& f1, F2&& f2) {
     return conjunction(FWD(f1), FWD(f2));
   }

   template<
    typename F1, REQUIRES(is_formula<F1>),
    typename F2, REQUIRES(is_formula<F2>)
    >
   auto operator ||(F1&& f1, F2&& f2) {
     return disjunction(FWD(f1), FWD(f2));
   }

   //
   // Helper functions akin to operators.
   // Note: these are found by ADL, and are *not* exported by the `black`
   //       namespace. This means the worringly generic names do not risk to
   //       cause name clashes with user names
   //
   #define declare_unary_helper(Op, Type) \
     template<typename F, REQUIRES(is_formula<F>)> \
     Type Op(F&& f) { return Type(FWD(f)); }

   declare_unary_helper(X, tomorrow)
   declare_unary_helper(Y, yesterday)
   declare_unary_helper(F, eventually)
   declare_unary_helper(G, always)
   declare_unary_helper(P, past)
   declare_unary_helper(H, historically)

   #undef declare_unary_helper

   #define declare_binary_helper(Op, Type) \
     template<typename F1, typename F2, \
              REQUIRES(is_formula<F1>, is_formula<F2>)> \
     Type Op(F1&& f1, F2&& f2) { return Type(FWD(f1), FWD(f2)); }

   declare_binary_helper(U, until)
   declare_binary_helper(S, since)
   declare_binary_helper(R, release)
   declare_binary_helper(T, triggered)

   #undef declare_binary_helper

   #define combine_unary_helpers(Op1, Op2) \
     template<typename F, REQUIRES(is_formula<F>)> \
     auto Op1##Op2(F&& f) { return Op1(Op2(FWD(f))); }

   combine_unary_helpers(X,F)
   combine_unary_helpers(X,G)
   combine_unary_helpers(Y,P)
   combine_unary_helpers(Y,H)
   combine_unary_helpers(G,F)

   #undef combine_unary_helpers

} // namespace black::details

// Names exported to the user
namespace black {
  using details::boolean;
  using details::atom;
  using details::unary;
  using details::binary;
  using details::formula;
  using details::otherwise;
}

#endif // BLACK_LOGIC_FORMULA_HPP_
