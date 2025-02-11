//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2025 Nicola Gigante
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

#ifndef BLACK_PARSING_COMBINATORS_HPP
#define BLACK_PARSING_COMBINATORS_HPP

namespace black::parsing {

  template<typename Pred, typename T>
  concept predicate_for = requires (Pred p, T c) {
    { p(c) } -> std::convertible_to<bool>;
  };

  //
  // Primitives
  //
  template<typename T>
  parser<T> value(T v) { 
    return [v = std::move(v)] -> parsed<T> {
      co_return v; 
    };
  }

  inline parser<void> fail() {
    return [] -> parsed<void> {
      co_return co_await fail_t{ };
    };
  }

  inline parser<void> eof() { 
    return [] -> parsed<void> {
      co_return co_await eof_t{ };
    };
  }

  inline parser<char> peek() { 
    return [] -> parsed<char> {
      co_return co_await peek_t{ };
    };
  }

  inline parser<void> advance() { 
    return [] -> parsed<void> {
      co_return co_await advance_t{ };
    };
  }

  template<typename T>
  parser<std::optional<T>> optional(parser<T> p) { 
    return [=] -> parsed<std::optional<T>> {
      co_return co_await optional_t{ p };
    };
  }

  template<typename T>
  parser<std::optional<std::vector<T>>> optional(parser<T[]> p) { 
    return [=] -> parsed<std::optional<std::vector<T>>> {
      co_return co_await optional_t{ p };
    };
  }
  
  inline parser<std::optional<std::monostate>> optional(parser<void> p) { 
    return [=] -> parsed<std::optional<std::monostate>> {
      co_return co_await optional_t{ p };
    };
  }

  template<typename T>
  parser<T> try_(parser<T> p) { 
    return [=] -> parsed<T> {
      co_return co_await try_t{ p };
    };
  }

  //
  // Derived combinators
  //
  parser<char> peek(predicate_for<char> auto pred) {
    return [=] -> parsed<char> {
      auto t = co_await peek();
      if(!pred(t))
        co_await fail();
      co_return t;
    };
  }

  inline parser<char> peek(char v) { 
    return peek([=](char c) { return c == v; });
  }

  inline parser<char> chr() {
    return [] -> parsed<char> {
      char c = co_await peek();
      
      co_await advance();
      
      co_return c;
    };
  }

  parser<char> expect(predicate_for<char> auto pred) {
    return [=] -> parsed<char> {
      auto t = co_await chr();
      if(!pred(t))
        co_await fail();
      co_return t;
    };
  }

  inline parser<char> expect(char v) { 
    return expect([=](char c) { return c == v; });
  }

  parser<char> chr(predicate_for<char> auto pred) {
    return try_(expect(pred));
  }

  inline parser<char> chr(char c) {
    return try_(expect(c));
  }

  template<typename T>
  parser<T> either(parser<T> p1, parser<T> p2) {
    return [=] -> parsed<T> {
      if(auto v = co_await optional(p1); v)
        co_return *v;
      
      co_return co_await p2;
    };
  }

  template<typename T>
  parser<T[]> either(parser<T[]> p1, parser<T[]> p2) {
    return [=] -> parsed<T[]> {
      if(auto v = co_await optional(p1); v)
        for(auto x : *v)
          co_yield x;
      
      for(auto x : co_await p2)
          co_yield x;
    };
  }

  template<typename T, parser_of<T> ...Ps>
  parser<T> either(parser<T> p, Ps ...ps) {
    return either(p, either(ps...));
  }

  template<typename T>
  parser<T> operator|(parser<T> p1, parser<T> p2) {
    return either(p1, p2);
  }

  template<typename T1, typename T2>
  parser<T2> then(parser<T1> p1, parser<T2> p2) {
    return [=] -> parsed<T2> {
      co_await p1;
      co_return co_await p2;
    };
  }

  template<typename T>
  parser<void> then(parser<T> p1, parser<void> p2) {
    return [=] -> parsed<void> {
      co_await p1;
      co_await p2;
    };
  }

  template<typename T, typename ...Ts>
  auto then(parser<T> p, parser<Ts> ...ps) {
    return then(p, then(ps...));
  }

  template<typename T1, typename T2>
  auto operator+(parser<T1> p1, parser<T2> p2) {
    return then(p1, p2);
  }

  template<typename T>
  parser<T[]> many(parser<T> p) {
    return [=] -> parsed<T[]> {
      std::optional<T> element;
      while((element = co_await optional(p))) {
        co_yield *element;
      }
    };
  }

  inline parser<void> many(parser<void> p) {
    return [=] -> parsed<void> {
      while(co_await optional(p)) { }
    };
  }

  template<typename T>
  parser<T[]> some(parser<T> p) {
    return [=] -> parsed<T[]> 
    {
      co_yield co_await p;

      for(auto v : co_await many(p))
        co_yield v;
    };
  }

  inline parser<void> some(parser<void> p) {
    return then(p, many(p));
  }
  
  template<typename T>
  parser<void> skip(parser<T> p) {
    return [=] -> parsed<void> {
      co_await p;
    };
  }

  template<typename T>
  parser<void> skip_many(parser<T> p) {
    return many(skip(p));
  }
  
  template<typename T>
  parser<void> skip_some(parser<T> p) {
    return some(skip(p));
  }

}

#endif // BLACK_PARSING_COMBINATORS_HPP
