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


  //
  // Primitives
  //
  template<typename Out>
  parser<Out> succeed(Out v) { co_return v; }

  inline parser<void> fail() {
    co_return co_await fail_t{ }; 
  }

  inline parser<void> eof() { 
    co_return co_await eof_t{ }; 
  }

  inline parser<char> peek() { 
    co_return co_await peek_t{ }; 
  }

  inline parser<void> advance() { 
    co_return co_await advance_t{ }; 
  }

  template<typename Out>
  parser<std::optional<Out>> optional(parser<Out> p) { 
    co_return co_await optional_t{ std::move(p) }; 
  }

  template<typename Out>
  parser<Out> try_(parser<Out> p) { 
    co_return co_await try_t{ std::move(p) }; 
  }

  //
  // Derived combinators
  //
  parser<char> peek(predicate auto pred) {
    auto t = co_await peek();
    if(!pred(t))
      co_await fail();
    co_return t;
  }

  inline parser<char> peek(char v) { 
    return peek([=](char c) { return c == v; });
  }

  inline parser<char> consume() {
    char c = co_await peek();
    
    co_await advance();
    
    co_return c;
  }

  parser<char> expect(predicate auto pred) {
    auto t = co_await consume();
    if(!pred(t))
      co_await fail();
    co_return t;
  }

  inline parser<char> expect(char v) { 
    return expect([=](char c) { return c == v; });
  }

  parser<char> ask(predicate auto pred) {
    return try_(expect(pred));
  }

  inline parser<char> ask(char c) {
    return try_(expect(c));
  }

  template<typename Out>
  parser<Out> either(parser<Out> p) {
    return std::move(p);
  }

  template<typename Out, parser_of<Out> ...Ps>
  parser<Out> either(parser<Out> p, Ps ...ps) {
    auto v = co_await optional(std::move(p));
    if(v)
      co_return *v;
    
    co_return co_await either(std::move(ps)...);
  }

  template<typename Out>
  parser<Out> operator||(parser<Out> p1, parser<Out> p2) {
    return either(std::move(p1), std::move(p2));
  }

}

#endif // BLACK_PARSING_COMBINATORS_HPP
