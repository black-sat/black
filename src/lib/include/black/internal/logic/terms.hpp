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
  struct exists;
  struct forall;
  struct entity;
  struct decl;
  struct def;

  namespace types {
    struct type;
  }
  
  namespace sl {
    struct standpoint;
    struct sp;
    struct star;
  }

  using types::type;

  template<typename T>
  concept term_source = std::constructible_from<term, T>;

  namespace internal {
    
    template<typename Node>
    struct term_custom_members {
      template<term_source ...Ts>
      atom operator()(Ts ...terms) const;
    };

    struct term_custom_ctor {
      static term init(decl const&);
      static term init(def const&);
      static term convert(std::integral auto);
      static term convert(std::floating_point auto);
      static term convert(bool);
    };
    
    struct integer_custom_ctor {
      static integer convert(std::integral auto);
    };
    
    struct real_custom_ctor {
      static real convert(std::floating_point auto);
    };
    
    struct boolean_custom_ctor {
      static boolean convert(bool);
    };
    
    struct variable_custom_ctor {
      static variable convert(std::string s);
    };

    struct exists_custom_ctor {
      static exists init(decl, term);
    };
    
    struct forall_custom_ctor {
      static forall init(decl, term);
    };
  }

  namespace sl::internal {
    struct sp_custom_ctor {
      static sp convert(std::string);
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
  struct ast_custom_ctor<logic::term> 
    : logic::internal::term_custom_ctor { };
  
  template<>
  struct ast_node_custom_ctor<logic::integer> 
    : logic::internal::integer_custom_ctor { };
  
  template<>
  struct ast_node_custom_ctor<logic::real> 
    : logic::internal::real_custom_ctor { };
  
  template<>
  struct ast_node_custom_ctor<logic::boolean> 
    : logic::internal::boolean_custom_ctor { };
  
  template<>
  struct ast_node_custom_ctor<logic::variable> 
    : logic::internal::variable_custom_ctor { };
  
  template<>
  struct ast_node_custom_ctor<logic::exists> 
    : logic::internal::exists_custom_ctor { };
  
  template<>
  struct ast_node_custom_ctor<logic::forall> 
    : logic::internal::forall_custom_ctor { };

  template<>
    struct ast_node_custom_ctor<logic::sl::sp> 
    : logic::sl::internal::sp_custom_ctor { };
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

  namespace types {
    inline bool operator==(type t1, type t2);
  }
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

    // work-around for a difference between Clang and G++ that causes an
    // incompatibility with Catch2 macros.

    //
    // This code would be the simple thing to do...
    //
    // explicit operator bool() const requires (EQ) {
    //   return term_equal(t1, t2);
    // }

    // explicit operator bool() const requires (!EQ) {
    //   return !term_equal(t1, t2);
    // }

    //
    // Instead we have to do the following...
    //

    inline static constexpr bool True = true;
    inline static constexpr bool False = false;

    explicit operator const bool&() const requires (EQ) {
      if(term_equal(t1, t2))
        return True;
      else
        return False;
    }

    explicit operator const bool&() const requires (!EQ) {
      if(!term_equal(t1, t2))
        return True;
      else
        return False;
    }

    // END work-around
    
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
  
  inline eq_wrapper_t<true> operator==(
    std::convertible_to<term> auto t1, 
    std::convertible_to<term> auto t2
  ) {
    return {term{t1}, term{t2}};
  }
  
  inline eq_wrapper_t<false> operator!=(
    std::convertible_to<term> auto t1, 
    std::convertible_to<term> auto t2
  ) {
    return {term{t1}, term{t2}};
  }

  inline bool operator==(std::optional<term> t1, std::optional<term> t2) {
    if(t1.has_value() && t2.has_value())
      return bool(*t1 == *t2);
    return t1.has_value() == t2.has_value();
  }

  inline bool operator!=(std::optional<term> t1, std::optional<term> t2) {
    return !(t1 == t2);
  }

  inline bool _test_equal(auto v1, auto v2) {
    return bool(v1 == v2);
  }
  
  inline bool 
  _test_equal(std::vector<term> const&v1, std::vector<term> const&v2) {
    return std::equal(begin(v1), end(v1), begin(v2), [](term t1, term t2) {
      return bool(t1 == t2);
    });
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
            return (_test_equal(args1, args2) && ...);
          },
          [](auto) { return false; }
        );
      }
    );
  }

  template<typename T = void>
  struct term_equal_to {
    term_equal_to() = default;
    term_equal_to(term_equal_to const&) = default;
    term_equal_to(term_equal_to &&) = default;
    
    term_equal_to &operator=(term_equal_to const&) = default;
    term_equal_to &operator=(term_equal_to &&) = default;

    bool operator()(T v1, T v2) const {
      return bool(v1 == v2);
    }

  };
  
  template<>
  struct term_equal_to<void> {
    term_equal_to() = default;
    term_equal_to(term_equal_to const&) = default;
    term_equal_to(term_equal_to &&) = default;
    
    term_equal_to &operator=(term_equal_to const&) = default;
    term_equal_to &operator=(term_equal_to &&) = default;

    bool operator()(auto v1, auto v2) const {
      return bool(v1 == v2);
    }

  };
  
  namespace internal {

    template<typename Node>
    template<term_source ...Ts>
    atom term_custom_members<Node>::operator()(Ts ...ts) const 
    {
      return 
        atom(static_cast<Node const&>(*this), std::vector<term>{term{ts}...});
    };

    inline term term_custom_ctor::init(decl const &d) {
      return term{d.name};
    }
    
    inline term term_custom_ctor::init(def const &d) {
      return term{d.name};
    }

    inline term term_custom_ctor::convert(std::integral auto v) {
      return integer(v);
    }

    inline term term_custom_ctor::convert(std::floating_point auto v) {
      return real(v);
    }
    
    inline term term_custom_ctor::convert(bool v) {
      return boolean(v);
    }

    inline integer integer_custom_ctor::convert(std::integral auto v) {
      return integer(int64_t(v));
    }

    inline real real_custom_ctor::convert(std::floating_point auto v) {
      return real(v);
    }

    inline boolean boolean_custom_ctor::convert(bool v) {
      return boolean(v);
    }

    inline variable variable_custom_ctor::convert(std::string s) {
      return variable(ast::core::label{s});
    }

    inline exists exists_custom_ctor::init(decl d, term body) {
      return exists({d}, body);
    }
    
    inline forall forall_custom_ctor::init(decl d, term body) {
      return forall({d}, body);
    }

  }

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

namespace black::logic::types {

  inline bool type_equal(type t1, type t2) {
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

  inline bool operator==(type t1, type t2) {
    return type_equal(t1, t2);
  }

}

namespace black::logic::sl {

  namespace internal {
    inline logic::sl::sp sp_custom_ctor::convert(std::string name) {
      return logic::sl::sp(ast::core::label{name});
    }
  }

  inline sharper operator<=(standpoint s1, standpoint s2) {
    return sharper(s1, s2);
  }

  inline bool operator==(standpoint s1, standpoint s2) {
    return support::match(s1)(
      [&](star) {
        return support::match(s2)(
          [](star) { return true; },
          [](sp) { return false; }
        );
      },
      [&](sp, auto name1) {
        return support::match(s2)(
          [](star) { return false; },
          [&](sp, auto name2) {
            return name1 == name2;
          }
        );
      }
    );
  }

}

#endif // BLACK_LOGIC_TERMS_HPP
