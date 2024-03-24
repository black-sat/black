//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_LOGIC_TERMS_HPP
#define BLACK_LOGIC_TERMS_HPP

namespace black::logic {
  struct term;
  struct atom;
  struct integer;
  struct real;
  struct variable;
  struct boolean;
  struct entity;

  namespace internal {
    
    template<typename Node>
    struct term_custom_members {
      template<std::convertible_to<term> ...Ts>
      atom operator()(Ts ...terms) const;
    };

    struct term_custom_init {
      static term init(std::integral auto);
      static term init(std::floating_point auto);
      static term init(bool);
    };
    
    struct integer_custom_init {
      static integer init(std::integral auto);
    };
    
    struct real_custom_init {
      static real init(std::floating_point auto);
    };
    
    struct boolean_custom_init {
      static boolean init(bool);
    };
    
    struct variable_custom_init {
      static variable init(std::string s);
    };
  }
}

namespace black::ast::core {
  template<>
  struct ast_custom_members<logic::term> 
    : logic::internal::term_custom_members<logic::term> { };
  
  template<ast_node_of<logic::term> Node>
  struct ast_node_custom_members<Node> 
    : logic::internal::term_custom_members<Node> { };
  
  template<>
  struct ast_custom_init<logic::term> 
    : logic::internal::term_custom_init { };
  
  template<>
  struct ast_node_custom_init<logic::integer> 
    : logic::internal::integer_custom_init { };
  
  template<>
  struct ast_node_custom_init<logic::real> 
    : logic::internal::real_custom_init { };
  
  template<>
  struct ast_node_custom_init<logic::boolean> 
    : logic::internal::boolean_custom_init { };
  
  template<>
  struct ast_node_custom_init<logic::variable> 
    : logic::internal::variable_custom_init { };
}

  //
  // Equality/inequality
  //

namespace black::logic {
  inline bool term_equal(term t1, term t2);

  template<bool EQ>
  struct eq_wrapper_t;

  inline eq_wrapper_t<true> operator==(term t1, term t2);

  inline eq_wrapper_t<false> operator!=(term t1, term t2);
}


#define BLACK_AST_REFLECT_DEFS_FILE <black/internal/logic/terms-defs.hpp>
#include <black/ast/reflect>
#undef BLACK_AST_REFLECT_DEFS_FILE


namespace black::logic 
{
  template<bool EQ>
  struct eq_wrapper_t {
    term t1;
    term t2;

    operator bool() const requires (EQ) {
      return term_equal(t1, t2);
    }

    operator bool() const requires (!EQ) {
      return !term_equal(t1, t2);
    }
    
    eq_wrapper_t<!EQ> operator!() const { 
      return {t1, t2};
    }

    operator term() const requires (EQ) {
      return equal({t1, t2});
    }

    operator equal() const requires (EQ) {
      return equal({t1, t2});
    }

    operator term() const requires (!EQ) {
      return distinct({t1, t2});
    }

    operator distinct() const requires (!EQ) {
      return distinct({t1, t2});
    }
  };

  inline eq_wrapper_t<true> operator==(term t1, term t2) {
    return {t1, t2};
  }
  
  inline eq_wrapper_t<false> operator!=(term t1, term t2) {
    return {t1, t2};
  }

  inline bool term_equal(term t1, term t2) {
    if(t1.unique_id() == t2.unique_id())
      return true;

    if(t1.hash() != t2.hash())
      return false;

    return support::match(t1)(
      [&]<typename Node>(Node, auto const& ...args1) {
        return support::match(t2)(
          [&](Node, auto const& ...args2) {
            return (bool(args1 == args2) && ...);
          },
          [](auto) { return false; }
        );
      }
    );
  }
  
  namespace internal {

    template<typename Node>
    template<std::convertible_to<term> ...Ts>
    atom term_custom_members<Node>::operator()(Ts ...ts) const 
    {
      return 
        atom(static_cast<Node const&>(*this), std::vector<term>{ts...});
    };

    inline term term_custom_init::init(std::integral auto v) {
      return integer(v);
    }

    inline term term_custom_init::init(std::floating_point auto v) {
      return real(v);
    }
    
    inline term term_custom_init::init(bool v) {
      return boolean(v);
    }

    inline integer integer_custom_init::init(std::integral auto v) {
      return integer(int64_t(v));
    }

    inline real real_custom_init::init(std::floating_point auto v) {
      return real(v);
    }

    inline boolean boolean_custom_init::init(bool v) {
      return boolean(v);
    }

    inline variable variable_custom_init::init(std::string s) {
      return variable(ast::core::label{s});
    }

  }
}

namespace black::logic {

  template<typename T, typename ...Ts>
  concept any_of = (std::same_as<T, Ts> || ...);

  template<typename T>
  concept connective = 
    any_of<T, negation, conjunction, disjunction, implication>;
  
  template<typename T>
  concept quantifier = any_of<T, exists, forall>;

  template<typename T>
  concept future =
    any_of<T, tomorrow, w_tomorrow, eventually, always, until, release>;

  template<typename T>
  concept past =
    any_of<T, yesterday, w_yesterday, once, historically, since, triggered>;

  template<typename T>
  concept temporal = future<T> || past<T>;

  template<typename T>
  concept arithmetic = any_of<T, minus, sum, product, difference, division>;
  
  template<typename T>
  concept relational = 
    any_of<T, less_than, less_than_eq, greater_than, greater_than_eq>;

}

#endif // BLACK_LOGIC_TERMS_HPP
