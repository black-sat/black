//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante, Alex Della Schiava
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

#ifndef BLACK_PIPES_ENCODING_HPP
#define BLACK_PIPES_ENCODING_HPP

#include <black/support>
#include <black/logic>

namespace black::pipes::internal {

class automaton_t : public transform::base
  {
  public:
    automaton_t(class consumer *next);

    virtual ~automaton_t() override;
      
    virtual class consumer *consumer() override;

    virtual std::optional<logic::object> translate(logic::object x) override;

    virtual logic::term undo(logic::term x) override;

  private:
    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };
}

namespace black::pipes {
  inline constexpr auto automaton = make_transform<internal::automaton_t>;
}

#endif // BLACK_PIPES_ENCODING_HPP