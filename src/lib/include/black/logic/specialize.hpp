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

#ifndef BLACK_LOGIC_SPECIALIZE_HPP
#define BLACK_LOGIC_SPECIALIZE_HPP

#include <vector>
#include <tuple>

namespace black::logic::internal {

  template<int Dummy, typename ...Types>
  struct cpp_tuple : std::type_identity<std::tuple<Types...>> { };

  template<int Dummy, typename ...Types>
  using cpp_tuple_t = std::tuple<Types...>;

  #define declare_term_type(Term) Term,

  enum class term::type : uint8_t {
    #include <black/logic/defs.hpp>
  };

  template<typename>
  struct term_types : cpp_tuple<0
    #define declare_term_type(Term) \
      , std::integral_constant<term::type, term::type::Term>

    #include <black/logic/defs.hpp>
  > { };

  #define declare_term_type(Term) \
    template<> \
    struct node_data<term::type::Term> : cpp_tuple<0

  #define declare_field(Term, Field, Type) , Type
  #define declare_fields(Term, Field, Type) , std::vector<Type>

  #define end_term_type(Term) \
    > { };

  #include <black/logic/defs.hpp>

  template<term::type Type>
  struct field_to_index;

  #define declare_term_type(Term) \
    template<> \
    struct field_to_index<term::type::Term> { \
      enum class index_of : uint8_t {

  #define declare_field(Term, Field, Type) Field, 
  #define declare_fields(Term, Field, Type) Field, 

  #define end_term_type(Term) \
      }; \
    };

  #include <black/logic/defs.hpp>


  #define declare_term_type(Term) \
    template<typename Derived> \
    struct term_handle_fields<term::type::Term, Derived> {

  #define declare_fields declare_field
  #define declare_field(Term, Field, Type) \
    decltype(auto) Field() const { \
      constexpr auto index = \
        std::to_underlying(field_to_index<term::type::Term>::index_of::Field); \
      Derived const& self = static_cast<Derived const&>(*this); \
      auto node = \
        std::static_pointer_cast<node_t<term::type::Term> const>(self._node); \
      \
      return std::get<index>(node->data); \
    }

  #define end_term_type(Term) \
    };

  #include <black/logic/defs.hpp>

  #define declare_term_type(Term) \
    struct Term : term_handle_base<term::type::Term> { \
      using term_handle_base<term::type::Term>::term_handle_base; \
    };

  #include <black/logic/defs.hpp>

  #define declare_term_type(Term) \
    template<> \
    struct type_of_handle<Term> { \
      static constexpr auto value = term::type::Term; \
    };

  #include <black/logic/defs.hpp>


  #define declare_term_type(Term) \
    template<typename ...Args> \
    struct alphabet_named_factory_base<term::type::Term, std::tuple<Args...>> \
      : alphabet_factory_base<term::type::Term> \
    { \
      internal::Term Term(Args ...args) { \
        return internal::Term{ \
          alphabet_factory_base<term::type::Term>::construct( \
            std::move(args)... \
          ) \
        }; \
      } \
    };

  #include <black/logic/defs.hpp>
}

namespace black::support {

  template<>
  struct match_trait<logic::internal::term> {

    using cases = logic::internal::cpp_tuple_t<0

    #define declare_term_type(Term) , logic::internal::Term

    #include <black/logic/defs.hpp>

    >;

    template<typename U>
    static std::optional<U> downcast(black::logic::term t) {
      return t.to<U>();
    }

  };

}


  

namespace black::logic {

  #define declare_term_type(Term) using internal::Term;

  #include <black/logic/defs.hpp>

}

#endif // BLACK_LOGIC_SPECIALIZE_HPP
