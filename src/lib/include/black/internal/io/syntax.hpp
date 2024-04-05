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

#ifndef BLACK_INTERNAL_IO_SYNTAX_HPP
#define BLACK_INTERNAL_IO_SYNTAX_HPP

#include <black/support>
#include <black/ast/core>

#include <variant>
#include <string_view>
#include <array>
#include <ostream>
#include <format>

namespace black::io {

  class token 
  {
  public:
    struct null        { std::monostate   value; };
    struct eof         { std::monostate   value; };
    struct invalid     { std::string_view value; };
    struct identifier  { std::string_view value; };
    struct keyword     { std::string_view value; };
    struct integer     { uint64_t         value; };
    struct real        { double           value; };
    struct punctuation { std::string_view value; };

    using token_t = std::variant<
      null, eof, invalid, identifier, keyword,
      integer, real, punctuation
    >;

    template<typename T>
      requires support::is_in_variant_v<T, token::token_t>
    friend bool operator==(T t1, T t2) { return t1.value == t2.value; }

    token() = default;

    template<typename T>
      requires std::is_constructible_v<token_t, T>
    token(T&& v) : _data{std::forward<T>(v)} { }
 
    token(token const&) = default;
    token(token &&) = default;
    
    token &operator=(token const&) = default;
    token &operator=(token &&) = default;

    bool operator==(token const&) const = default;

    template<typename T>
    bool is() const {
      if constexpr(support::is_in_variant_v<T, token_t>)
        return std::holds_alternative<T>(_data);
      return false;
    }

    template<typename T>
    std::optional<T> get() const {
      if constexpr(support::is_in_variant_v<T, token_t>)
        if(is<T>())
          return std::get<T>(_data);
      return {};
    }

    explicit operator bool() const { 
      return !is<null>() && !is<invalid>() && !is<eof>();
    }

  private:
    token_t _data;
  };

}

namespace black::support {

  template<>
  struct match_cases<black::io::token> 
    : support::variant_alternatives<black::io::token::token_t> { };

  template<typename T>
    requires support::is_in_variant_v<T, black::io::token::token_t>
  struct match_downcast<black::io::token, T> {
    static std::optional<T> downcast(black::io::token tok) {
      return tok.get<T>();
    }
  };
}

namespace black::io {
  
  inline std::ostream &operator<<(std::ostream &out, token tok) {
    return out << support::match(tok)(
      [&](token::null) { 
        return "token::null{}"; 
      },
      [&](token::eof) { 
        return "token::eof{}"; 
      },
      [&](token::invalid t) { 
        return std::format("token::invalid{{{:?}}}", t.value);
      },
      [&](token::identifier t) { 
        return std::format("token::identifier{{{:?}}}", t.value);
      },
      [&](token::keyword t) { 
        return std::format("token::keyword{{{:?}}}", t.value);
      },
      [&](token::integer t) { 
        return std::format("token::integer{{{}}}", t.value);
      },
      [&](token::real t) { 
        return std::format("token::real{{{}}}", t.value);
      },
      [&](token::punctuation t) { 
        return std::format("token::punctuation{{{:?}}}", t.value);
      }
    );
  }

  template<typename P>
  concept parselet = requires (P p, std::string_view str) {
    // lexing
    { &P::is_symbol } -> std::convertible_to<bool (*)(std::string_view)>;
    { &P::is_keyword } -> std::convertible_to<bool (*)(std::string_view)>;
  };

}

#endif // BLACK_INTERNAL_IO_SYNTAX_HPP
