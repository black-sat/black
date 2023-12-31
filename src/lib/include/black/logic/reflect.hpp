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

#ifndef BLACK_LOGIC_REFLECT_HPP
#define BLACK_LOGIC_REFLECT_HPP

#include <string_view>

namespace black::logic::reflect {

  template<typename Term>
  struct terms_enum { };

  template<typename Term>
  using terms_enum_t = typename terms_enum<Term>::type;

  template<typename Term>
  struct terms_list { };

  template<typename Term>
  using terms_list_t = typename terms_list<Term>::type;

  template<typename Term, terms_enum_t<Term> Case>
  struct term_type { };
  
  template<typename Term, terms_enum_t<Term> Case>
  using term_type_t = typename term_type<Term, Case>::type;

  template<typename Term, terms_enum_t<Term> Case>
  struct term_fields_enum { };

  template<typename Term, terms_enum_t<Term> Case>
  using term_fields_enum_t = typename term_fields_enum<Term, Case>::type;

  template<typename Term, terms_enum_t<Term> Case>
  struct term_fields_list { };

  template<typename Term, terms_enum_t<Term> Case>
  using term_fields_list_t = typename term_fields_list<Term, Case>::type;

  template<
    typename Term, terms_enum_t<Term> Case, 
    term_fields_enum_t<Term, Case> Field
  >
  struct term_field_name { };
  
  template<
    typename Term, terms_enum_t<Term> Case, 
    term_fields_enum_t<Term, Case> Field
  >
  inline constexpr auto term_field_name_v = 
    term_field_name<Term, Case, Field>::value;
  
  template<
    typename Term, terms_enum_t<Term> Case, 
    term_fields_enum_t<Term, Case> Field
  >
  struct term_field_type { };
  
  template<
    typename Term, terms_enum_t<Term> Case, 
    term_fields_enum_t<Term, Case> Field
  >
  using term_field_type_t = typename term_field_type<Term, Case, Field>::type;

  template<
    typename Derived,
    typename Term, terms_enum_t<Term> Case, 
    term_fields_enum_t<Term, Case> Field,
    auto Member
  >
  struct term_field_member_base { };

}

//
// x-pattern expansions to implement the traits above
//
namespace black::logic {
  struct term;

  #define declare_term_type(Term) \
    struct Term;
  #include <black/logic/defs.hpp>
}

namespace black::logic::reflect {

  template<int Dummy, typename ...Args>
  struct tuple_cpp : std::type_identity<std::tuple<Args...>> { };
  
  template<int Dummy, typename ...Args>
  using tuple_cpp_t = typename tuple_cpp<Dummy, Args...>::type;

  template<>
  struct terms_enum<term> {
    enum class type {
      #define declare_term_type(Term) Term, 
      #include <black/logic/defs.hpp>
    };
  };

  template<>
  struct terms_list<term> : tuple_cpp<0
    #define declare_term_type(Term) \
      , std::integral_constant< \
          terms_enum_t<term>, terms_enum_t<term>::Term \
        >
    #include <black/logic/defs.hpp>
  > { };

  #define declare_term_type(Term) \
    template<> \
    struct term_type<term, terms_enum_t<term>::Term> \
      : std::type_identity<Term> { };
  #include <black/logic/defs.hpp>

  #define declare_term_type(Term) \
    template<> \
    struct term_fields_enum<term, terms_enum_t<term>::Term> { \
      enum class type { \
  
  #define declare_field(Term, Field, Type) \
        Field, 

  #define end_term_type(Term) \
      }; \
    };

  #include <black/logic/defs.hpp>

  #define declare_term_type(Term) \
    template<> \
    struct term_fields_list<term, terms_enum_t<term>::Term> : tuple_cpp<0

  #define declare_field(Term, Field, Type) \
    , std::integral_constant< \
        term_fields_enum_t<term, terms_enum_t<term>::Term>, \
        term_fields_enum_t<term, terms_enum_t<term>::Term>::Field \
      >
  
  #define end_term_type(Term) \
  > { };

  #include <black/logic/defs.hpp>

  #define declare_field(Term, Field, Type) \
    template<> \
    struct term_field_name< \
      term, \
      terms_enum_t<term>::Term, \
      term_fields_enum_t<term, terms_enum_t<term>::Term>::Field \
    > { \
      static constexpr std::string_view value = #Field; \
    };

  #include <black/logic/defs.hpp>

  #define declare_field(Term, Field, Type) \
    template<> \
    struct term_field_type< \
      term, \
      terms_enum_t<term>::Term, \
      term_fields_enum_t<term, terms_enum_t<term>::Term>::Field \
    > : std::type_identity<Type> { };

  #include <black/logic/defs.hpp>

  #define declare_field(Term, Field, Type) \
  template< \
    typename Derived, \
    typename Base, typename R, typename ...Args, \
    R (Base::*Ptr)(Args...) \
  > \
  struct term_field_member_base< \
    Derived, \
    term, terms_enum_t<term>::Term, \
    term_fields_enum_t<term, terms_enum_t<term>::Term>::Field, \
    Ptr \
  > { \
    R Field(Args... args) { \
      return (static_cast<Derived *>(this)->*Ptr)(std::move(args...)); \
    } \
  }; \
  \
  template< \
    typename Derived, \
    typename Base, typename R, typename ...Args, \
    R (Base::*Ptr)(Args...) const \
  > \
  struct term_field_member_base< \
    Derived, \
    term, terms_enum_t<term>::Term, \
    term_fields_enum_t<term, terms_enum_t<term>::Term>::Field, \
    Ptr \
  > { \
    R Field(Args... args) const { \
      return (static_cast<Derived const*>(this)->*Ptr)(std::move(args...)); \
    } \
  }; 

  #include <black/logic/defs.hpp>

}

#endif // BLACK_LOGIC_REFLECT_HPP
