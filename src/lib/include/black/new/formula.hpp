//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_LOGIC_FORMULA_HPP
#define BLACK_LOGIC_FORMULA_HPP

#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#include <variant>

namespace black::internal::new_api {
  
  enum class quantifier_type : uint8_t {
    exists,
    forall
  };

  class function 
  {
  public:
    enum type : uint8_t {
      negation,
      subtraction,
      addition,
      multiplication,
      division
    };

    function() = delete;
    function(type t) : _data{t} { }
    function(identifier const&name) : _data{name} { }

    function(function const&) = default;
    function(function &&) = default;

    friend bool operator==(function const&f1, function const&f2) {
      return f1._data == f2._data;
    }

    friend bool operator!=(function const&f1, function const&f2) {
      return f1._data != f2._data;
    }

    function &operator=(function const&) = default;
    function &operator=(function &&) = default;

    template<typename... T>
    class application operator()(T ...args);

    std::optional<function::type> known_type() const {
      if(std::holds_alternative<type>(_data))
        return std::get<type>(_data);
      return std::nullopt;
    }

    identifier name() const {
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

  private:
    std::variant<type, identifier> _data;
  };

  inline std::string to_string(function f) {
    return to_string(f.name());
  }
}

namespace std {
  template<>
  struct hash<::black::internal::new_api::function> {
    size_t operator()(black::internal::new_api::function const&f) const {
      if(auto k = f.known_type(); k)
        return hash<uint8_t>{}(static_cast<uint8_t>(*k));

      return hash<::black::internal::identifier>{}(f.name());
    }
  };

  template<typename T>
  struct hash<std::vector<T>>
  {
    size_t operator()(std::vector<T> const&v) const {
      hash<T> h;
      size_t result = 0;
      for(size_t i = 0; i < v.size(); ++i)
        result = ::black::internal::hash_combine(result, h(v[i]));

      return result;
    }
  };
}

#include <black/new/hierarchy_impl.hpp>

namespace black::internal::new_api {
  template<typename... T>
  application function::operator()(T ...args) {
    std::vector<term> argsv = {args...};

    return application(*this, std::move(argsv));
  }
}

#endif // BLACK_LOGIC_FORMULA_HPP
