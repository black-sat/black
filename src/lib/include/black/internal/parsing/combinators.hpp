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
  parser<Out> succeed(Out v) {
    co_return v;
  }

  inline eof_t eof() { return { }; }

  inline peek_t peek() { return { }; }

  inline consume_t consume() { return { }; }

  template<typename Out, typename In>
  optional_t<Out, In> optional(parser<Out, In> p) { return { std::move(p) }; }

  //
  // Sugar on top of primitives
  //
  template<typename In>
  parser<In, In> consume(In v) {
    In in = co_await consume();
    if(v != in)
      co_return failure();
    
    co_return v;
  }


}

#endif // BLACK_PARSING_COMBINATORS_HPP
