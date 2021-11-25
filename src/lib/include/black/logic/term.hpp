//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#ifndef BLACK_TERM_HPP
#define BLACK_TERM_HPP

#include <black/support/meta.hpp>
#include <cstdint>
#include <string>
#include <optional>
#include <variant>

namespace black::internal {
  
  enum class term_type : uint8_t {
    constant,
    variable,
    application,
    next
  };

  struct application;

  struct function 
  {
    enum type : uint8_t {
      negation,
      subtraction,
      addition,
      multiplication,
      division,
      modulo
    };

    function() = delete;
    function(type);
    function(std::string const&name);

    function(function const&) = default;
    function(function &&) = default;

    friend bool operator==(function const&f1, function const&f2);
    friend bool operator!=(function const&f1, function const&f2);

    function &operator=(function const&) = default;
    function &operator=(function &&) = default;

    template<typename... T>
    application operator()(T ...args);

    std::optional<type> known_type() const;
    std::string name() const;

  private:
    std::variant<type, std::string> _data;
  };
}

#include <black/internal/term/base.hpp>

namespace black::internal {

  class term 
  {
    public:
      term() = delete;
      term(term const&) = default;
      term(term &&) = default;

      term &operator=(term const&) = default;
      term &operator=(term &&) = default;

      using type = black::internal::term_type;

      type term_type() const;

      alphabet *sigma() const;

      template<typename H>
      std::optional<H> to() const;

      template<typename H>
      bool is() const;

      template<typename ...Cases>
      auto match(Cases &&...) const;

      size_t hash() const;

      term_id unique_id() const;

    private:
      class alphabet *_alphabet;
      term_base *_term;

      friend struct term_base;
      
      template<typename, typename>
      friend struct term_handle_base;

      template<typename, typename>
      friend struct handle_base;

    public:
      explicit term(class alphabet *sigma, term_base *t)
        : _alphabet{sigma}, _term{t} { black_assert(t != nullptr); }
  };

  struct constant : term_handle_base<constant, constant_t>
  {
    using term_handle_base<constant, constant_t>::term_handle_base;

    int value() const { return _term->value; }
  };

  struct variable : term_handle_base<variable, variable_t>
  {
    using term_handle_base<variable, variable_t>::term_handle_base;

    identifier label() const;

    template<typename T>
    std::optional<T> label() const;
  };

  struct application : term_handle_base<application, application_t>
  {
    using term_handle_base<application, application_t>::term_handle_base;

    application(function const&func, std::vector<term> const&args);

    function func() const;

    std::vector<term> arguments() const;
  };

  struct next : term_handle_base<next, next_t>
  {
    using term_handle_base<next, next_t>::term_handle_base;

    next(term arg);

    term argument() const;
  };

  // Syntactic sugar for known functions
  application operator-(term);
  application operator-(term, term);
  application operator-(term, int);
  application operator-(int, term);
  application operator+(term, term);
  application operator+(term, int);
  application operator+(int, term);
  application operator*(term, term);
  application operator*(term, int);
  application operator*(int, term);
  application operator/(term, term);
  application operator/(term, int);
  application operator/(int, term);
}

// Names exported from the `black` namespace
namespace black {
  using internal::term;
  using internal::term_id;
  using internal::constant;
  using internal::variable;
  using internal::application;
  using internal::next;
  using internal::function;
}

#include <black/internal/term/impl.hpp>

#endif