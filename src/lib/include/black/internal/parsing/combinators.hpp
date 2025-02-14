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
  parser<T> pass(T v) { 
    return [v = std::move(v)] -> parsed<T> {
      co_return v; 
    };
  }

  inline parser<void> reject() {
    return [] -> parsed<void> {
      co_await internal::primitives::reject{ };
    };
  }

  inline parser<char> peek() { 
    return [] -> parsed<char> {
      co_return co_await internal::primitives::peek{ };
    };
  }

  inline parser<void> advance() { 
    return [] -> parsed<void> {
      co_return co_await internal::primitives::advance{ };
    };
  }

  template<typename T>
  parser<T> choice(parser<T> p1, parser<T> p2) {
    return [=] -> parsed<T> {
      co_return co_await internal::primitives::choice{ p1, p2 };
    };
  }

  template<typename T>
  parser<T[]> choice(parser<T[]> p1, parser<T[]> p2) {
    return [=] -> parsed<T[]> {
      for(auto x : co_await internal::primitives::choice{ p1, p2 })
        co_yield std::move(x);
    };
  }

  template<typename T>
  parser<T> try_(parser<T> p) {
    return [=] -> parsed<T> {
      co_return co_await internal::primitives::try_{ p };
    };
  }

  //
  // Derived combinators
  //
  parser<char> peek(predicate_for<char> auto pred) {
    return [=] -> parsed<char> {
      auto t = co_await peek();
      if(!pred(t))
        co_await reject();
      co_return t;
    };
  }

  inline parser<char> peek(char v) { 
    return peek([=](char c) { return c == v; });
  }


  template<typename T, predicate_for<T> F>
  parser<T> require(parser<T> p, F f) {
    return [=] -> parsed<T> {
      if(auto v = co_await p; f(v))
        co_return std::move(v);
      co_await reject();
      black_unreachable();
    };
  }

  parser<char> chr(predicate_for<char> auto pred) {
    return [=] -> parsed<char> {
      char c = co_await require(peek(), pred);
      
      co_await advance();
      
      co_return c;
    };
  }

  inline parser<char> chr() {
    return chr([](char){ return true; });
  }

  inline parser<char> chr(char c) {
    return chr([=](char v) { return c == v; });
  }

  template<typename T, parser_of<T> ...Ps>
  parser<T> choice(parser<T> p, Ps ...ps) {
    return choice(p, choice(ps...));
  }

  template<typename T>
  parser<T> operator|(parser<T> p1, parser<T> p2) {
    return choice(p1, p2);
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

  template<typename T1, typename T2>
  parser<T2[]> then(parser<T1> p1, parser<T2[]> p2) {
    return [=] -> parsed<T2[]> {
      co_await p1;
      for(auto v : co_await p2)
        co_yield std::move(v);
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

  template<typename T, typename ...Args>
    requires std::is_constructible_v<T, Args...>
  parser<T> construct(parser<Args> ...args) {
    return [=] -> parsed<T> {
      co_return T{ co_await args... };
    };
  }

  template<typename T>
  parser<std::optional<T>> optional(parser<T> p) { 
    return choice(construct<std::optional<T>>(p), pass(std::optional<T>{}));
  }

  template<typename T, typename R = std::optional<std::vector<T>>>
  parser<R> optional(parser<T[]> p) { 
    return choice(construct<R>(p), pass(R{}));
  }
  
  inline parser<bool> optional(parser<void> p) { 
    return choice(p + pass(true), pass(false));
  }

  // inline parser<void> eof() { 
  //   return [] -> parsed<void> {
  //     co_return co_await internal::primitives::eof{ };
  //   };
  // }

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

  template<typename T>
  parser<void> operator not(parser<T> p) {
    return [=] -> parsed<T> {
      if(co_await optional(try_(p)))
        co_await reject();
    };
  }

}

#endif // BLACK_PARSING_COMBINATORS_HPP
