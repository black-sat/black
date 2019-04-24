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

#ifndef BLACK_LOGIC_FORMULA_IMPL_HPP_
#define BLACK_LOGIC_FORMULA_IMPL_HPP_

#ifndef BLACK_LOGIC_FORMULA_HPP_
  #error "This header file cannot included alone, "\
         "please include <black/logic/formula.hpp> instead"
#endif

#include <charconv>

namespace black::details
{
  /*
   * Out-of-line definitions for class `formula_base`
   */
  inline formula_base::formula_base(formula_type t) : type{t} { }

  /*
   * Out-of-line definitions for class `formula`
   */
  inline bool operator==(formula f1, formula f2) {
    return f1._formula == f2._formula;
  }

  inline bool operator!=(formula f1, formula f2) {
    return !(f1 == f2);
  }

  inline formula::type formula::formula_type() const {
    return _formula->type;
  }

  inline alphabet *formula::alphabet() const {
    return _alphabet;
  }

  template<typename H>
  optional<H> formula::to() const {
    black_assert(_formula != nullptr);

    return H::cast(_alphabet, _formula);
  }

  template<typename H>
  bool formula::is() const {
    black_assert(_formula != nullptr);
    return H::cast(_alphabet, _formula).has_value();
  }

  inline size_t formula::hash() const {
    return std::hash<formula_base*>{}(_formula);
  }

  // Implementation of formula::to_sat() under alphabet's in alphabet_impl.hpp
  // Implementation of formula::match() below, after handles

  /*
   * Opaque id for formulas
   */
  enum class formula_id : uintptr_t;

  inline formula_id formula::unique_id() const {
    return static_cast<formula_id>(reinterpret_cast<uintptr_t>(_formula));
  }

  inline std::string to_string(formula_id id) {
    return std::to_string(static_cast<uintptr_t>(id));
  }

  inline std::optional<formula_id> id_from_string(std::string_view str)
  {
    try {
      uintptr_t p = 0;
      auto [last, err] =
        std::from_chars(str.data(), str.data() + str.size(), p);

      if(*last == '\0')
        return {static_cast<formula_id>(p)};
    }catch(...) {}

    return std::nullopt;
  }

  //
  // std::hash specialization for class `formula`
  //
} namespace std {

  template<>
  struct hash<black::details::formula> {
    size_t operator()(black::details::formula const&f) const {
      return f.hash();
    }
  };

} namespace black::details {

  /*
   * Out-of-line definitions for handles
   */

  // struct handle_base
  template<typename H, typename F>
  handle_base<H,F>::operator formula() const {
    return formula{this->_alphabet, this->_formula};
  }

  template<typename H, typename F>
  alphabet *handle_base<H,F>::alphabet() const {
    return _alphabet;
  }

  template<typename H, typename F>
  formula_id handle_base<H,F>::unique_id() const {
    return formula{*this}.unique_id();
  }

  // struct atom
  inline std::any atom::label() const {
    return _formula->label.any();
  }

  template<typename T>
  std::optional<T> atom::label() const {
    return _formula->label.to<T>();
  }

  // struct unary
  inline unary::unary(type t, formula f)
    : handle_base<unary, unary_t>{allocate_unary(t, f)} { }

  inline unary::type unary::formula_type() const {
    black_assert(is_unary_type(_formula->type));
    return static_cast<unary::type>(_formula->type);
  }

  inline formula unary::operand() const {
    return formula{_alphabet, _formula->operand};
  }

  // struct binary
  inline binary::binary(type t, formula f1, formula f2)
    : handle_base<binary, binary_t>{allocate_binary(t, f1, f2)} { }

  inline binary::type binary::formula_type() const {
    black_assert(is_binary_type(_formula->type));
    return static_cast<binary::type>(_formula->type);
  }

  inline formula binary::left() const {
    return formula{_alphabet, _formula->left};
  }

  inline formula binary::right() const {
    return formula{_alphabet, _formula->right};
  }

  /*
   * Type-specific handles, similar to unary and binary, but with the operator
   * known at compile-type
   */
  template<typename H, typename F, auto OT>
  struct operator_base : handle_base<H, F>
  {
    friend class formula;
    friend class black::alphabet;

    using base_t = handle_base<H, F>;
    using base_t::base_t;

  protected:
    static optional<H> cast(alphabet *sigma, formula_base *f) {
      auto ptr = formula_cast<F *>(f);
      if( ptr && ptr->type == static_cast<formula_type>(OT))
        return optional<H>{H{sigma, ptr}};
      return nullopt;
    }
  };

  template<typename H, unary::type OT>
  struct unary_operator : operator_base<H, unary_t, OT>
  {
    using base_t = operator_base<H, unary_t, OT>;
    using base_t::base_t;

    explicit unary_operator(formula f) : base_t{this->allocate_unary(OT, f)}
    {
      black_assert(is_unary_type(this->_formula->type));
      black_assert(this->_formula->type == static_cast<formula_type>(OT));
    }

    operator unary() const { return unary{this->_alphabet, this->_formula}; }

    formula operand() const {
      return formula{this->_alphabet, this->_formula->operand};
    }
  };

  template<typename H, binary::type OT>
  struct binary_operator : operator_base<H, binary_t, OT>
  {
    using base_t = operator_base<H, binary_t, OT>;
    using base_t::base_t;

    binary_operator(formula f1, formula f2)
      : base_t{this->allocate_binary(OT, f1, f2)}
    {
      black_assert(is_binary_type(this->_formula->type));
      black_assert(this->_formula->type == static_cast<formula_type>(OT));
    }

    operator binary() const { return binary{this->_alphabet, this->_formula}; }

    formula left() const {
      return formula{this->_alphabet, this->_formula->left};
    }

    formula right() const {
      return formula{this->_alphabet, this->_formula->right};
    }
  };

  #define declare_operator(Op, Arity)                                 \
    struct Op : Arity##_operator<Op, Arity::type::Op> {                 \
      using base_t = Arity##_operator<Op, Arity::type::Op>;             \
      using base_t::base_t;                                           \
      friend operator_base<Op, Arity##_t, Arity::type::Op>;             \
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

  //
  // Implementation of the matching function
  //
  template<typename ...Cases>
  auto formula::match(Cases&& ...cases) const
  {
    black_assert(_formula);

    #define match_case(H)                                             \
      case formula_type::H: {                                         \
        black_assert(is<H>());                                        \
        return apply_first(std::make_tuple(*to<H>()), FWD(cases)...); \
      }

    switch(_formula->type) {
      match_case(boolean)
      match_case(atom)
      match_case(negation)
      match_case(tomorrow)
      match_case(yesterday)
      match_case(always)
      match_case(eventually)
      match_case(past)
      match_case(historically)
      match_case(conjunction)
      match_case(disjunction)
      match_case(then)
      match_case(iff)
      match_case(until)
      match_case(release)
      match_case(since)
      match_case(triggered)
    }
    #undef match_case
    black_unreachable();
  }

  //
  // Implementation of operators
  //
  inline auto operator !(formula f) { return negation(f); }

  inline auto operator &&(formula f1, formula f2) {
    return conjunction(f1, f2);
  }

  inline auto operator ||(formula f1, formula f2) {
    return disjunction(f1, f2);
  }

  // Helper functions akin to operators.
  // Note: these are found by ADL, and are *not* exported by the `black`
  //       namespace. This means the worringly generic names do not risk to
  //       cause name clashes with user names
  inline auto X(formula f) { return tomorrow(f);     }
  inline auto Y(formula f) { return yesterday(f);    }
  inline auto F(formula f) { return eventually(f);   }
  inline auto G(formula f) { return always(f);       }
  inline auto P(formula f) { return past(f);         }
  inline auto H(formula f) { return historically(f); }

  inline auto U(formula f1, formula f2) { return until(f1,f2);     }
  inline auto R(formula f1, formula f2) { return release(f1,f2);   }
  inline auto S(formula f1, formula f2) { return since(f1,f2);     }
  inline auto T(formula f1, formula f2) { return triggered(f1,f2); }

  inline auto XF(formula f) { return X(F(f)); }
  inline auto XG(formula f) { return X(G(f)); }
  inline auto GF(formula f) { return G(F(f)); }
  inline auto YP(formula f) { return Y(P(f)); }
  inline auto YH(formula f) { return Y(H(f)); }
}

#endif // BLACK_LOGIC_FORMULA_IMPL_HPP_
