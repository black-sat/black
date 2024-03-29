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

#ifndef BLACK_PIPES_PIPES_HPP
#define BLACK_PIPES_PIPES_HPP

namespace black::pipes {

  class id_t : public transform::base
  {
  public:
    id_t(class consumer *next) : _next{next} { }
      
    virtual class consumer *consumer() override { return _next; }

  private:
    class consumer *_next;
  };

  inline constexpr auto id = make_transform<id_t>{};


  class composed_t : public transform::base
  {
  public:
    composed_t(
      class consumer *next, 
      transform::pipeline first, transform::pipeline second
    ) : _second{second(next)}, _first{first(_second->consumer())} { }
      
    virtual class consumer *consumer() override {
      return _first->consumer();
    }
  
  private:
    transform::instance _second;
    transform::instance _first;
  };

  inline constexpr auto composed = make_transform<composed_t>{};

  inline transform::pipeline 
  operator|(transform::pipeline first, transform::pipeline second) {
    return composed(std::move(first), std::move(second));
  }

}

#endif // BLACK_PIPES_PIPES_HPP
