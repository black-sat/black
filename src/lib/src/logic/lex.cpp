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


#include "black/logic/lex.hpp"

#include <map>
#include <algorithm>
#include <istream>
#include <charconv>

namespace black::internal
{
  namespace {

    std::optional<token> digits(std::istream &s, lexer::error_handler error)
    {
      std::string data;

      // integer part
      while(isdigit(s.peek())) {
        data += char(s.peek());
        s.get();
      }
      if(data.empty())
        return {};

      if(s.peek() == '.') {
        data += '.';
        s.get();
      } else {
        int val = 0;
        auto [ptr, err] = 
          std::from_chars(data.data(), data.data() + data.size(), val);

        if(err == std::errc())
          return token{val};

        error(
          "Integer constant '" + data + "' is too big. " +
          "Admitted range: [" + 
          std::to_string(std::numeric_limits<int>::min()) + ", " +
          std::to_string(std::numeric_limits<int>::max()) + "]"
        );
        return std::nullopt;
      }

      // fractional part
      while(isdigit(s.peek())) {
        data += char(s.peek());
        s.get();
      }
      if(data.empty())
        return {};
      
      try {
        return token{stod(data)};
      } catch (std::out_of_range const&) {
        error(
          "Fractional constant '" + data + "' cannot be represented as a "
          "64-bit floating-point value"
        );
        return std::nullopt;
      }
    }

    std::optional<token> symbol(std::istream &s)
    {
      char ch = char(s.peek());

      switch (ch) {
        case '(':
          s.get();
          return token{token::punctuation::left_paren};
        case ')':
          s.get();
          return token{token::punctuation::right_paren};
        case ',':
          s.get();
          return token{token::punctuation::comma};
        case '.':
          s.get();
          return token{token::punctuation::dot};
        case '!':
          s.get();
          if(s.peek() == '=') {
            s.get();
            return token{relation::type::not_equal};
          }
          return token{unary::type::negation};
        case '~':
          s.get();
          return token{unary::type::negation};
        // '&' or '&&'
        case '&':
          s.get();
          if (s.peek() == '&')
            s.get();
          return token{binary::type::conjunction};

        // '|' or '||'
        case '|':
          s.get();
          if (s.peek() == '|')
            s.get();
          return token{binary::type::disjunction};

        // '->' and '=>'
        case '-':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{binary::type::implication};
          }
          return token{function::type::subtraction};
        case '=':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{binary::type::implication};
          }
          return token{relation::type::equal};
        
        case '>':
          s.get();
          if(s.peek() == '=') {
            s.get();
            return token{relation::type::greater_than_equal};
          }
          return token{relation::type::greater_than};

        // '<->' or '<=>' or '<>'
        case '<':
          s.get();
          if (s.peek() == '-' || s.peek() == '=')
            s.get();
          else
            return token{relation::type::less_than};

          if (s.peek() == '>') {
            s.get();
            return token{binary::type::iff};
          }
          return token{relation::type::less_than_equal};

        case '+':
          s.get();
          return token{function::type::addition};
        case '*':
          s.get();
          return token{function::type::multiplication};
        case '/':
          s.get();
          return token{function::type::division};
      }

      // TODO: garantire che se restituiamo nullopt, lo stream non è avanzato
      return std::nullopt;
    }
  }  // namespace

  bool lexer::_is_identifier_char(int c) {
    return isalnum(c) || c == '_';
  }

  bool lexer::_is_initial_identifier_char(int c) {
    return isalpha(c) || c == '_';
  }

  std::optional<token> lexer::_identifier()
  {
    static std::pair<std::string_view, token> operators[] = {
      {"True",   token{true}},
      {"False",  token{false}},
      {"next",   token{token::keyword::next}},
      {"wnext",  token{token::keyword::wnext}},
      {"exists", token{token::keyword::exists}},
      {"forall", token{token::keyword::forall}},
      {"NOT",    token{unary::type::negation}},
      {"X",      token{unary::type::tomorrow}},
      {"wX",     token{unary::type::w_tomorrow}},
      {"Y",      token{unary::type::yesterday}},
      {"Z",      token{unary::type::w_yesterday}},
      {"F",      token{unary::type::eventually}},
      {"G",      token{unary::type::always}},
      {"O",      token{unary::type::once}},
      {"H",      token{unary::type::historically}},
      {"AND",    token{binary::type::conjunction}},
      {"OR",     token{binary::type::disjunction}},
      {"THEN",   token{binary::type::implication}},
      {"IFF",    token{binary::type::iff}},
      {"U",      token{binary::type::until}},
      {"R",      token{binary::type::release}},
      {"V",      token{binary::type::release}},
      {"W",      token{binary::type::w_until}},
      {"M",      token{binary::type::s_release}},
      {"S",      token{binary::type::since}},
      {"T",      token{binary::type::triggered}}
    };

    if (!_stream.good() || !_is_initial_identifier_char(_stream.peek()))
      return std::nullopt;

    std::string id;
    while (_stream.good() && _is_identifier_char(_stream.peek())) 
    {
      id += char(_stream.peek());
      _stream.get();
    }
    black_assert(!id.empty());

    auto it =
      std::find_if(std::begin(operators), std::end(operators), [&](auto p) {
        return p.first == id;
      });

    if(it != std::end(operators))
      return {it->second};

    return token{std::move(id)};
  }

  std::optional<token> lexer::_lex()
  {
    char c = (char)_stream.peek();
    while (_stream.good() && isspace(c)) {
      _stream.get();
      c = (char)_stream.peek();
    }

    if (!_stream.good())
      return std::nullopt;

    if(std::optional<token> t = digits(_stream, _error); t)
      return t;

    if(std::optional<token> t = symbol(_stream); t)
      return t;

    return _identifier();
  }

}  // namespace black::internal
