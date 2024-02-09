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

#ifndef BLACK_SMT_BACKENDS_CVC5_HPP
#define BLACK_SMT_BACKENDS_CVC5_HPP

#include <black/logic>

#include <memory>

namespace black::smt {

  class cvc5 : public backend 
  {
  public:
    cvc5();

    cvc5(cvc5 const&) = delete;
    cvc5(cvc5 &&) = delete;
    
    virtual ~cvc5() override;

    cvc5 &operator=(cvc5 const&) = delete;
    cvc5 &operator=(cvc5 &&) = delete;

    virtual result<support::tribool> check(logic::module const *mod) override;

    virtual std::optional<logic::scope const *> model() const override;

  private:
    struct _impl_t;
    std::unique_ptr<_impl_t> _impl;
  };

}

#endif // BLACK_SMT_BACKENDS_CVC5_HPP