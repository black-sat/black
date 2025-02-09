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

  inline eof_t eof() { return { }; }

  inline peek_t peek() { return { }; }

  inline consume_t consume() { return { }; }

  template<typename Out, typename In>
  parser<std::optional<Out>, In> optional(parser<Out, In> p) { 
    co_return co_await optional_t{ std::move(p) }; 
  }

  template<typename Out, typename In>
  parser<Out, In> lookahead(parser<Out, In> p) { 
    co_return co_await lookahead_t{ std::move(p) }; 
  }

  //
  // Derived combinators
  //
  template<typename In>
  parser<In, In> consume(In v) {
    In in = co_await consume();
    if(v != in)
      co_return failure();
    
    co_return v;
  }

  template<typename Out, typename In>
  parser<Out, In> either(parser<Out, In> p) {
    return std::move(p);
  }

  template<typename Out, typename In, parser_of<Out, In> ...Ps>
  parser<Out, In> either(parser<Out, In> p, Ps ...ps) {
    auto v1 = co_await optional(std::move(p));
    if(v1)
      co_return *v1;
    
    co_return co_await either(std::move(ps)...);
  }

}

#endif // BLACK_PARSING_COMBINATORS_HPP
