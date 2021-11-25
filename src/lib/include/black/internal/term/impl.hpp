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

  /*
   * Matching facility
   */
  using term_syntax = syntax<
    constant,
    variable,
    application,
    next
  >;

  template<typename ...Handlers>
  auto term::match(Handlers&& ...handlers) const {
    return matcher<term, term_syntax>::match(*this, FWD(handlers)...);
  }

  inline size_t term::hash() const {
    return std::hash<term_base *>{}(_term);
  }

  template<size_t I, REQUIRES(I == 0)>
  term get(next n) {
    return n.argument();
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
    struct tuple_size<::black::internal::next>
      : std::integral_constant<int, 1> { };

  template<size_t I>
    struct tuple_element<I, ::black::internal::next> {
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

  // struct function
  inline function::function(type f) : _data{f} { }
  inline function::function(std::string const&name) : _data{name} { }

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

  inline std::string function::name() const {
    if(std::holds_alternative<std::string>(_data))
      return std::get<std::string>(_data);
    
    black_assert(std::holds_alternative<type>(_data));
    type func = std::get<type>(_data);
    switch(func) {
      case negation:
      case subtraction:
        return "-";
      case addition:
        return "+";
      case multiplication:
        return "*";
      case division:
        return "/";
      case modulo:
        return "mod";
      default:
        black_unreachable();
    }
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

  // struct next
  inline next::next(term arg)
    : term_handle_base<next, next_t>{allocate_next(arg)} { }

  inline term next::argument() const {
    return term{_alphabet, _term->arg};
  }

  //
  // operators on terms defined in internal/formula/alphabet.hpp
  // to have a complete alphabet type
  //
}

namespace std {
  template<>
  struct hash<::black::internal::function> {
    size_t operator()(black::internal::function const&f) const {
      if(auto k = f.known_type(); k)
        return hash<uint8_t>{}(static_cast<uint8_t>(*k));

      return hash<std::string>{}(f.name());
    }
  };

  template<>
  struct hash<::black::internal::term_id> {
    size_t operator()(black::internal::term_id id) const {
      return hash<uintptr_t>{}(static_cast<uintptr_t>(id));
    }
  };
}


#endif