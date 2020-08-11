/*
 Copyright (c) 2014, Nicola Gigante
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * The names of its contributors may not be used to endorse or promote
 products derived from this software without specific prior written
 permission.
 */


#include "black/logic/lex.hpp"

#include <map>
#include <algorithm>
#include <iostream>

namespace black::internal
{
  namespace {

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
        case '!':
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

        // '->'
        case '-':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{binary::type::implication};
          }
          return std::nullopt;

        // '=' or '=>'
        case '=':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{binary::type::implication};
          }
          else {
            return token{binary::type::iff};
          }

        // '<->' or '<=>' or '<>'
        case '<':
          s.get();
          if (s.peek() == '-' || s.peek() == '=')
            ch = char(s.get());

          if (s.peek() == '>') {
            s.get();
            return ch != '<' ? token{binary::type::iff}
                             : token{unary::type::eventually};
          }
          return std::nullopt;

        case '[':
          s.get();
          if (s.peek() == ']') {
            s.get();
            return token{unary::type::always};
          }
          return std::nullopt;
      }

      // TODO: garantire che se restituiamo nullopt, lo stream non è avanzato
      return std::nullopt;
    }
  }  // namespace

  std::optional<token> lexer::_identifier()
  {
    static constexpr std::pair<std::string_view, token> operators[] = {
      {"True",  token{true}},
      {"False", token{false}},
      {"NOT",   token{unary::type::negation}},
      {"X",     token{unary::type::tomorrow}},
      {"Y",     token{unary::type::yesterday}},
      {"F",     token{unary::type::eventually}},
      {"G",     token{unary::type::always}},
      {"P",     token{unary::type::once}},
      {"H",     token{unary::type::historically}},
      {"AND",   token{binary::type::conjunction}},
      {"OR",    token{binary::type::disjunction}},
      {"THEN",  token{binary::type::implication}},
      {"IFF",   token{binary::type::iff}},
      {"U",     token{binary::type::until}},
      {"R",     token{binary::type::release}},
      {"V",     token{binary::type::release}},
      {"S",     token{binary::type::since}},
      {"T",     token{binary::type::triggered}}
    };

    if (!_stream.good() || !isalpha(_stream.peek()))
      return std::nullopt;

    std::string id;
    while (_stream.good() && isalnum(_stream.peek())) {
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

    _lexed_identifiers.push_back(std::move(id));
    return token{std::string_view{_lexed_identifiers.back()}};
  }

  std::optional<token> lexer::_lex()
  {
    while (_stream.good() && isspace(_stream.peek())) {
      _stream.get();
    }

    if (!_stream.good())
      return std::nullopt;

    if(std::optional<token> t = symbol(_stream); t)
      return t;

    return _identifier();
  }

}  // namespace black::internal
