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

namespace black::details
{
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
    if(auto *unary = formula_cast<unary_t const*>(_formula); unary)
      return op_to_formula_type(unary->op_type);
    if(auto *binary = formula_cast<binary_t const*>(_formula); binary)
      return op_to_formula_type(binary->op_type);

    return type{to_underlying(_formula->type)};
  }

  template<typename H>
  optional<H> formula::to() const {
    black_assert(_formula != nullptr);

    return H::cast(_formula);
  }

  template<typename H>
  bool formula::is() const {
    black_assert(_formula != nullptr);
    return H::cast(_formula).has_value();
  }

  inline size_t formula::hash() const {
    return std::hash<formula_base const*>{}(_formula);
  }

  // Implementation of formula::match() below, after handles

  //
  // std::hash specialization for class `formula`
  //
} namespace std {

  template<>
  struct hash<black::details::formula> {
    size_t operator()(black::details::formula const&f) {
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
    return formula{this->_formula};
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
  inline unary::unary(operator_type t, formula f)
    : handle_base<unary, unary_t>{allocate_formula(t, f)} { }

  inline unary_t::operator_type unary::type() const {
    return _formula->op_type;
  }

  inline formula unary::operand() const {
    return formula{_formula->operand};
  }

  // struct binary
  inline binary::binary(operator_type t, formula f1, formula f2)
    : handle_base<binary, binary_t>{allocate_formula(t, f1, f2)} { }

  inline binary_t::operator_type binary::type() const {
    return _formula->op_type;
  }

  inline formula binary::left() const {
    return formula{_formula->left};
  }

  inline formula binary::right() const {
    return formula{_formula->right};
  }

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

    explicit unary_operator(formula f) : base_t{this->allocate_formula(OT, f)}
    {
      black_assert(this->_formula->type == formula_base::formula_type::unary);
      black_assert(this->_formula->op_type == OT);
    }

    operator unary() const { return unary{this->_formula}; }

    formula operand() const { return formula{this->_formula->operand}; }
  };

  template<typename H, binary_t::operator_type OT>
  struct binary_operator : operator_base<H, binary_t, OT>
  {
    using base_t = operator_base<H, binary_t, OT>;
    using base_t::base_t;

    binary_operator(formula f1, formula f2)
      : base_t{this->allocate_formula(OT, f1, f2)}
    {
      black_assert(this->_formula->type == formula_base::formula_type::binary);
      black_assert(this->_formula->op_type == OT);
    }

    operator binary() const { return binary{this->_formula}; }

    formula left() const { return formula{this->_formula->left}; }

    formula right() const { return formula{this->_formula->right}; }
  };

  #define declare_operator(Op, Arity)                                 \
    struct Op : Arity##_operator<Op, Arity##_t::Op> {                 \
      using base_t = Arity##_operator<Op, Arity##_t::Op>;             \
      using base_t::base_t;                                           \
      friend operator_base<Op, Arity##_t, Arity##_t::Op>;             \
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
    #undef match_case
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
