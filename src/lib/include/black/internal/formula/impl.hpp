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
  #error "This header file cannot be included alone, "\
         "please include <black/logic/formula.hpp> instead."
#endif

#include <charconv>

namespace black::internal
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

  inline alphabet *formula::sigma() const {
    return _alphabet;
  }

  template<typename H>
  std::optional<H> formula::to() const {
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

  // Implementation of formula::match() in match.hpp, included below

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

  /*
   * Out-of-line definitions for handles
   */

  // struct handle_base
  template<typename H, typename F>
  handle_base<H,F>::operator formula() const {
    return formula{this->_alphabet, this->_formula};
  }

  template<typename H, typename F>
  alphabet *handle_base<H,F>::sigma() const {
    return _alphabet;
  }

  template<typename H, typename F>
  formula_id handle_base<H,F>::unique_id() const {
    return formula{*this}.unique_id();
  }

  // struct proposition
  inline std::any proposition::label() const {
    return _formula->label.any();
  }

  template<typename T>
  std::optional<T> proposition::label() const {
    return _formula->label.to<T>();
  }

  // struct atom
  inline atom::atom(relation const&r, std::vector<term> const&terms)
    : handle_base<atom, atom_t>{allocate_atom(r, terms)} { }

  inline relation atom::rel() const {
    return _formula->r;
  }

  inline std::vector<term> atom::terms() const {
    std::vector<term> result;
    for(term_base *t : _formula->terms)
      result.push_back(term{_alphabet, t});
    
    return result;
  }

  // struct relation
  inline relation::relation(known_relation r) : _data{r} { }
  inline relation::relation(std::string const& name) : _data{name}{ }

  inline bool operator==(relation const&r1, relation const&r2) {
    return r1._data == r2._data;
  }

  inline bool operator!=(relation const&r1, relation const&r2) {
    return r1._data != r2._data;
  }

  template<typename... T>
  inline atom relation::operator()(T ...args) {
    std::array<term, sizeof...(args)> _args = {args...};
    std::vector<term> argsv;
    std::copy(_args.begin(), _args.end(), std::back_inserter(argsv));

    return atom(*this, argsv);
  }

  inline std::optional<relation::known_relation> relation::known() const {
    if(std::holds_alternative<known_relation>(_data))
      return {std::get<known_relation>(_data)};
    return std::nullopt;
  }

  inline std::string relation::name() const {
    if(std::holds_alternative<std::string>(_data))
      return std::get<std::string>(_data);

    black_assert(std::holds_alternative<known_relation>(_data));
    known_relation r = std::get<known_relation>(_data);
    switch(r) {
      case equal:
        return "==";
      case not_equal:
        return "!=";
      case less_than:
        return "<";
      case less_than_equal:
        return "<=";
      case greater_than:
        return ">";
      case greater_than_equal:
        return ">=";
      default:
        black_unreachable();
    }

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
    friend class alphabet;

    using base_t = handle_base<H, F>;
    using base_t::base_t;

  protected:
    static std::optional<H> cast(alphabet *sigma, formula_base *f) {
      auto ptr = formula_cast<F *>(f);
      if( ptr && ptr->type == static_cast<formula_type>(OT))
        return std::optional<H>{H{sigma, ptr}};
      return std::nullopt;
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

  //
  // std::hash specialization for `formula` and others
  //
  #define declare_formula_hash(Type)                              \
  } namespace std {                                               \
    template<>                                                    \
    struct hash<black::internal::Type> {                          \
      size_t operator()(black::internal::Type const&f) const {    \
        return black::internal::formula{f}.hash();                \
      }                                                           \
    };                                                            \
  } namespace black::internal {

  #define declare_operator(Op, Arity)                                   \
    struct Op : Arity##_operator<Op, Arity::type::Op> {                 \
      using base_t = Arity##_operator<Op, Arity::type::Op>;             \
      using base_t::base_t;                                             \
      friend operator_base<Op, Arity##_t, Arity::type::Op>;             \
    };                                                                  \
    declare_formula_hash(Op)                                            \
  } namespace black { using internal::Op; } namespace black::internal {      
  
  
  declare_operator(negation,     unary)
  declare_operator(tomorrow,     unary)
  declare_operator(w_tomorrow,   unary)
  declare_operator(yesterday,    unary)
  declare_operator(w_yesterday,  unary)
  declare_operator(always,       unary)
  declare_operator(eventually,   unary)
  declare_operator(once,         unary)
  declare_operator(historically, unary)
  declare_operator(conjunction, binary)
  declare_operator(disjunction, binary)
  declare_operator(implication, binary)
  declare_operator(iff,         binary)
  declare_operator(until,       binary)
  declare_operator(release,     binary)
  declare_operator(w_until,     binary)
  declare_operator(s_release,   binary)
  declare_operator(since,       binary)
  declare_operator(triggered,   binary)

  declare_formula_hash(formula)
  declare_formula_hash(boolean)
  declare_formula_hash(proposition)
  declare_formula_hash(unary)
  declare_formula_hash(binary)

  #undef declare_operator

  //
  // Implementation of most of the operators. 
  // The ones taking boolean args need alphabet hence are implemented in 
  // alphabet_impl.hpp
  //
  inline negation operator !(formula f) { return negation(f); }

  inline conjunction operator &&(formula f1, formula f2) {
    return conjunction(f1, f2);
  }

  inline disjunction operator ||(formula f1, formula f2) {
    return disjunction(f1, f2);
  }

  inline implication implies(formula f1, formula f2) {
    return implication(f1, f2);
  }

  // Helper functions akin to operators.
  // Note: these are found by ADL, and are *not* exported by the `black`
  //       namespace. This means the worringly generic names do not risk to
  //       cause name clashes with user names
  inline tomorrow     X(formula f) { return tomorrow(f);     }
  inline w_tomorrow  wX(formula f) { return w_tomorrow(f);   }
  inline yesterday    Y(formula f) { return yesterday(f);    }
  inline w_yesterday  Z(formula f) { return w_yesterday(f);  }
  inline eventually   F(formula f) { return eventually(f);   }
  inline always       G(formula f) { return always(f);       }
  inline once         O(formula f) { return once(f);         }
  inline historically H(formula f) { return historically(f); }

  inline until     U(formula f1, formula f2) { return until(f1,f2);      }
  inline release   R(formula f1, formula f2) { return release(f1,f2);    }
  inline w_until   W(formula f1, formula f2) { return w_until(f1, f2);   }
  inline s_release M(formula f1, formula f2) { return s_release(f1, f2); }
  inline since     S(formula f1, formula f2) { return since(f1,f2);      }
  inline triggered T(formula f1, formula f2) { return triggered(f1,f2);  }

  inline tomorrow   XF(formula f) { return X(F(f)); }
  inline tomorrow   XG(formula f) { return X(G(f)); }
  inline eventually FG(formula f) { return F(G(f)); }
  inline always     GF(formula f) { return G(F(f)); }
  inline yesterday  YO(formula f) { return Y(O(f)); }
  inline yesterday  YH(formula f) { return Y(H(f)); }
}

namespace std {
  template<>
  struct hash<::black::internal::relation> {
    size_t operator()(black::internal::relation const&r) const {
      if(auto k = r.known(); k)
        return hash<uint8_t>{}(static_cast<uint8_t>(*k));

      return hash<std::string>{}(r.name());
    }
  };
}

#include <black/internal/formula/match.hpp>

#endif // BLACK_LOGIC_FORMULA_IMPL_HPP_
