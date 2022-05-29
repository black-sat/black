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

#ifndef BLACK_LOGIC_TERM_IMPL_HPP
#define BLACK_LOGIC_TERM_IMPL_HPP

#ifndef BLACK_TERM_HPP
  #error "This header file cannot be included alone, "\
         "please include <black/logic/term.hpp> instad."
#endif

#include <charconv>
#include <string_view>

namespace black::internal
{
  /*
   * Out-of-line definitions for class `term`
   */
  inline term::type term::term_type() const {
    return _term->type;
  }

  inline alphabet *term::sigma() const {
    return _alphabet;
  }

  template<typename H>
  std::optional<H> term::to() const {
    black_assert(_term != nullptr);

    return H::cast(_alphabet, _term);
  }

  template<typename H>
  bool term::is() const {
    black_assert(_term != nullptr);
    return H::cast(_alphabet, _term).has_value();
  }

  inline size_t term::hash() const {
    return std::hash<term_base *>{}(_term);
  }

  template<size_t I, REQUIRES(I == 0)>
  term get(constructor c) {
    return c.argument();
  }
} // namespace black::internal

namespace std {
  template<>
  struct tuple_size<::black::internal::constant>
    : std::integral_constant<int, 0> { };

  template<>
  struct tuple_size<::black::internal::variable>
    : std::integral_constant<int, 0> { };

  template<>
  struct tuple_size<::black::internal::constructor>
    : std::integral_constant<int, 1> { };
  
  template<size_t I>
  struct tuple_element<I, ::black::internal::constructor> {
    using type = ::black::internal::term;
  };

  template<>
    struct tuple_size<::black::internal::application>
      : std::integral_constant<int, 0> { };
}

namespace black::internal {
  /*
   * Opaque id for formulas
   */
  enum class term_id : uintptr_t;

  inline term_id term::unique_id() const {
    return static_cast<term_id>(reinterpret_cast<uintptr_t>(_term));
  }

  inline std::string to_string(term_id id) {
    return std::to_string(static_cast<uintptr_t>(id));
  }

  inline std::optional<term_id> term_id_from_string(std::string_view str)
  {
    try {
      uintptr_t p = 0;
      auto [last, err] =
        std::from_chars(str.data(), str.data() + str.size(), p);

      if(*last == '\0')
        return {static_cast<term_id>(p)};
    }catch(...) {}

    return std::nullopt;
  }

  /*
   * Out-of-line definitions for handles
   */ 

  // struct term_handle_base
  template<typename H, typename T>
  term_handle_base<H, T>::operator term() const {
    return term{this->_alphabet, this->_term};
  }

  template<typename H, typename T>
  alphabet *term_handle_base<H, T>::sigma() const {
    return _alphabet;
  }

  template<typename H, typename T>
  term_id term_handle_base<H, T>::unique_id() const {
    return term{*this}.unique_id();
  }

  // struct variable
  inline identifier variable::label() const {
    return _term->label;
  }

  template<typename T>
  std::optional<T> variable::label() const {
    return _term->label.to<T>();
  }

  // class function
  inline function::function(type f) : _data{f} { }
  inline function::function(identifier const&name) : _data{name} {}


  inline bool operator==(function const&f1, function const&f2) {
    return f1._data == f2._data;
  }

  inline bool operator!=(function const&f1, function const&f2) {
    return f1._data != f2._data;
  }

  template<typename... T>
  inline application function::operator()(T ...args) {
    std::array<term, sizeof...(args)> _args = {args...};
    std::vector<term> argsv;
    std::copy(_args.begin(), _args.end(), std::back_inserter(argsv));

    return application(*this, argsv);
  }

  inline std::optional<function::type> function::known_type() const {
    if(std::holds_alternative<type>(_data))
      return std::get<type>(_data);
    return std::nullopt;
  }

  inline identifier function::name() const {
    using namespace std::literals;

    if(std::holds_alternative<identifier>(_data))
      return std::get<identifier>(_data);
    
    black_assert(std::holds_alternative<type>(_data));
    type func = std::get<type>(_data);
    switch(func) {
      case type::negation:
      case type::subtraction:
        return identifier{"-"sv};
      case type::addition:
        return identifier{"+"sv};
      case type::multiplication:
        return identifier{"*"sv};
      case type::division:
        return identifier{"/"sv};
    }
    black_unreachable(); // LCOV_EXCL_LINE
  }

  // struct application
  inline application::application(
    function const&func, std::vector<term> const&args
  ) : term_handle_base<application, application_t>{
      allocate_application(func, args)
    } { 
      black_assert(!args.empty());
    }

  inline function application::func() const {
    return _term->f;
  }

  inline std::vector<term> application::arguments() const {
    std::vector<term> result;
    for(term_base *t : _term->args) {
      result.push_back(term{_alphabet, t});
    }

    return result;
  }

  // struct constructor
  inline constructor::constructor(type t, term arg)
    : term_handle_base<constructor, constructor_t>{allocate_constructor(t, arg)}
    { }

  inline constructor::type constructor::term_type() const {
    return static_cast<constructor::type>(_term->type);
  }

  inline term constructor::argument() const {
    return term{_alphabet, _term->arg};
  }

  //
  // operators on terms are defined in internal/formula/alphabet.hpp
  // to have a complete alphabet type
  //

  template<typename H, typename F, auto OT>
  struct term_operator_base : term_handle_base<H, F>
  {
    friend class term;
    friend class alphabet;

    using base_t = term_handle_base<H, F>;
    using base_t::base_t;

  protected:
    static std::optional<H> cast(alphabet *sigma, term_base *t) {
      auto ptr = term_cast<F *>(t);
      if(ptr && ptr->type == static_cast<term_type>(OT))
        return std::optional<H>{H{sigma, ptr}};
      return std::nullopt;
    }
  };

  template<typename H, constructor::type OT>
  struct term_ctor_operator : term_operator_base<H, constructor_t, OT>
  {
    using base_t = term_operator_base<H, constructor_t, OT>;
    using base_t::base_t;

    explicit term_ctor_operator(term t) 
      : base_t{this->allocate_constructor(OT, t)}
    {
      black_assert(is_constructor_type(this->_term->type));
      black_assert(this->_term->type == static_cast<term_type>(OT));
    }

    operator constructor() const { 
      return constructor{this->_alphabet, this->_term}; 
    }

    term argument() const {
      return term{this->_alphabet, this->_term->arg};
    }
  };

  #define declare_ctor_operator(Op)                                          \
    struct Op : term_ctor_operator<Op, constructor::type::Op> {              \
      using base_t = term_ctor_operator<Op, constructor::type::Op>;          \
      using base_t::base_t;                                                  \
      friend term_operator_base<Op, constructor_t, constructor::type::Op>;   \
    };                                                                       \
  } namespace black { using internal::Op; } namespace std {                  \
    template<>                                                               \
    struct tuple_size<::black::internal::Op>                                 \
      : std::integral_constant<int, 1> { };                                  \
                                                                             \
    template<size_t I>                                                       \
    struct tuple_element<I, ::black::internal::Op> {                         \
      using type = ::black::internal::term;                                  \
    };                                                                       \
  } namespace black::internal {

  declare_ctor_operator(next)
  declare_ctor_operator(wnext)
  declare_ctor_operator(prev)
  declare_ctor_operator(wprev)

  /*
   * Matching facility
   */
  using term_syntax = syntax<
    constant,
    variable,
    application,
    next,
    wnext,
    prev,
    wprev
  >;

  template<typename ...Handlers>
  auto term::match(Handlers&& ...handlers) const {
    return matcher<term, term_syntax>::match(*this, FWD(handlers)...);
  }
}

namespace std {
  template<>
  struct hash<::black::internal::term_id> {
    size_t operator()(black::internal::term_id id) const {
      return hash<uintptr_t>{}(static_cast<uintptr_t>(id));
    }
  };

  #define declare_term_hash(Type)                              \
    template<>                                                 \
    struct hash<black::internal::Type> {                       \
      size_t operator()(black::internal::Type const&t) const { \
        return black::internal::term{t}.hash();                \
      }                                                        \
    };

  declare_term_hash(term)
  declare_term_hash(constant)
  declare_term_hash(variable)
  declare_term_hash(application)
  declare_term_hash(next)
  declare_term_hash(wnext)

  #undef declare_term_hash
}


#endif
