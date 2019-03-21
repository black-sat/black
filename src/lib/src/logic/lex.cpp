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
#include <iostream>

namespace black::details
{
  namespace {

    std::optional<token> symbol(std::istream &s)
    {
      char ch = char(s.peek());

      switch (ch) {
        case '(':
          s.get();
          return token{token::left_paren};
        case ')':
          s.get();
          return token{token::right_paren};
        case '!':
        case '~':
          s.get();
          return token{token::negation};
        // '&' or '&&'
        case '&':
          s.get();
          if (s.peek() == '&')
            s.get();
          return token{token::conjunction};

        // '|' or '||'
        case '|':
          s.get();
          if (s.peek() == '|')
            s.get();
          return token{token::disjunction};

        // '->'
        case '-':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{token::then};
          }
          return std::nullopt;

        // '=' or '=>'
        case '=':
          s.get();
          if (s.peek() == '>') {
            s.get();
            return token{token::then};
          }
          else {
            return token{token::iff};
          }

        // '<->' or '<=>' or '<>'
        case '<':
          s.get();
          if (s.peek() == '-' || s.peek() == '=')
            ch = char(s.get());

          if (s.peek() == '>') {
            s.get();
            return ch != '<' ? token{token::iff}
                             : token{token::eventually};
          }
          return std::nullopt;

        case '[':
          s.get();
          if (s.peek() == ']') {
            s.get();
            return token{token::always};
          }
          return std::nullopt;
      }

      return std::nullopt;
    }

    std::optional<token> keyword(std::istream &s)
    {
      static const std::map<std::string, token::token_type> keywords = {
        {"NOT", token::negation},    {"AND",  token::conjunction},
        {"OR",  token::disjunction}, {"THEN", token::then},
        {"IFF", token::iff},         {"X",    token::tomorrow},
        {"U",   token::until},       {"R",    token::release},
        {"V",   token::release},     {"G",    token::always},
        {"F",   token::eventually},  {"Y",    token::yesterday},
        {"S",   token::since},       {"T",    token::triggered},
        {"P",   token::past},        {"H",    token::historically}
      };

      std::string kw;

      if (!s.good())
        return std::nullopt;

      if (isalpha(s.peek())) {
        kw += char(s.peek());
        s.get();

        if (auto it = keywords.find(kw); it != keywords.end())
          return token{it->second};
      } else
        return std::nullopt;

      while (s.good() && isalnum(s.peek())) {
        kw += char(s.peek());
        s.get();

        if (auto it = keywords.find(kw); it != keywords.end())
          return token{it->second};
      }
      if (!s.good() && !s.eof())
        return std::nullopt;

      return token{kw};
    }

  }  // namespace

  std::optional<token> lexer::_lex()
  {
    while (_stream.good() && isspace(_stream.peek())) {
      _stream.get();
    }

    if (!_stream.good())
      return std::nullopt;

    if(std::optional<token> t = symbol(_stream); t)
      return t;

    return keyword(_stream);
  }

}  // namespace black::details
