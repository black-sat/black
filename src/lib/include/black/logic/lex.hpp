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

#ifndef BLACK_LEX_H_
#define BLACK_LEX_H_

#include <black/logic/formula.hpp>

#include <iostream>
#include <cassert>
#include <cctype>

#include <optional>
#include <variant>
#include <vector>

namespace black::details
{
  #define declare_token(Name) Name = formula::type,

  // Type representing tokens generated from the lexer.
  // WARNING: tokens must not outlive their originating lexer object.
  struct token
  {
    enum class type : uint8_t {
      boolean = 0,
      atom,
      unary_operator,
      binary_operator,
      punctuation
    };

    // Type of non-logical tokens. Only parens, for now.
    enum class punctuation : uint8_t {
      // non-logical tokens
      left_paren,
      right_paren
    };

    constexpr token(bool b)               : _data{b} { }
    constexpr token(std::string_view s)   : _data{s} { }
    constexpr token(unary::type t)        : _data{t} { }
    constexpr token(binary::type t)       : _data{t} { }
    constexpr token(punctuation s) : _data{s} { }

    template<typename T>
    constexpr bool is() const {
      return std::holds_alternative<T>(_data);
    }

    template<typename T>
    constexpr std::optional<T> data() const {
      if(auto p = std::get_if<T>(&_data); p)
        return {*p};
      return std::nullopt;
    }

    type token_type() const { return static_cast<type>(_data.index()); }

    friend constexpr std::string_view to_string(token const &tok);

  private:
    // data related to recognized tokens
    std::variant<
      bool,             // booleans
      std::string_view, // atoms
      unary::type,      // unary operator
      binary::type,     // binary operator
      punctuation       // any non-logical token
    > _data;
  };

  constexpr std::string_view to_string(unary::type const& t)
  {
    constexpr std::string_view toks[] = {
      "!", // negation
      "X", // tomorrow
      "Y", // yesterday
      "G", // always
      "F", // eventually
      "P", // past
      "H", // historically
    };

    return toks[to_underlying(t) - to_underlying(unary::type::negation)];
  }

  constexpr std::string_view to_string(binary::type const& t) {
    constexpr std::string_view toks[] = {
      "&&",  // conjunction
      "||",  // disjunction
      "->",  // then
      "<->", // iff
      "U",   // until
      "R",   // release
      "S",   // since
      "T",   // triggered
    };

    return toks[to_underlying(t) - to_underlying(binary::type::conjunction)];
  }

  constexpr std::string_view to_string(token const &tok)
  {
    using namespace std::literals;

    return std::visit( overloaded {
      [](bool b)             { return b ? "true"sv : "false"sv; },
      [](std::string_view s) { return s; },
      [](unary::type t)      { return to_string(t); },
      [](binary::type t)     { return to_string(t); },
      [](token::punctuation s) {
        switch(s) {
          case token::punctuation::left_paren:
            return "("sv;
          case token::punctuation::right_paren:
            return ")"sv;
        }
      }
    }, tok._data);
  }

  class lexer
  {
  public:
    explicit lexer(std::istream &stream) : _stream(stream) {}
    std::optional<token> get() { return _token = _lex(); }
    std::optional<token> peek() const { return _token; }

  private:
    std::optional<token> _lex();
    std::optional<token> _keyword(std::istream &stream);

    std::optional<token> _token = std::nullopt;
    std::istream &_stream;

    std::vector<std::string> _lexed_keywords;
  };

  std::ostream &operator<<(std::ostream &s, token const &t);
}

#endif // LEX_H_
