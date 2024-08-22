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

#include "pipeline.hpp"
#include "utils.hpp"
#include <black/support>
#include <black/logic>
#include <black/ast/algorithms>
#include <black/pipes>
#include <iostream>

namespace black::pipes::internal {

  using namespace support;
  using namespace logic;
  using namespace ast;

  struct automaton_t::impl_t : public consumer
  {
    impl_t(class consumer *next) : _next{next}{ }

    class consumer *_next;

    std::optional<object> translate(object x);
    term undo(term x);

    virtual void import(logic::module) override;
    virtual void adopt(std::shared_ptr<logic::root const>) override;
    virtual void state(logic::term t, logic::statement s) override;
    virtual void push() override;
    virtual void pop(size_t) override;
  };

  automaton_t::automaton_t( class consumer *next ) : _impl{std::make_unique<impl_t>(next)} {
    std::cout << "costruttore" << std::endl;
  }

  automaton_t::~automaton_t() = default;

  consumer *automaton_t::consumer() { return _impl.get(); }

  std::optional<object> automaton_t::translate(object x) { 
    return _impl->translate(x); 
  }
  
  term automaton_t::undo(term x) { return _impl->undo(x); }

  void automaton_t::impl_t::import(logic::module m) {
    std::cout << "import";
    _next->import(std::move(m));
  }

  void automaton_t::impl_t::adopt(std::shared_ptr<logic::root const>) {
    std::cout << "adopt" << std::endl;
  }

  void automaton_t::impl_t::state(logic::term t, logic::statement s) {
    if(s == logic::statement::requirement) {
      std::cout << "state -> requirement" << std::endl;
      module mod;
      object surrogate = mod.declare(decl("s", types::function({}, types::boolean())), resolution::delayed);
    
      _next->adopt(mod.resolve());

      _next->state(t, statement::init);
      //mod.require(implication(surrogate, t) && implication(t, surrogate));
    }
    if(s == logic::statement::init) {
      std::cout << "state -> init" << std::endl;
    }
    if(s == logic::statement::transition) {
      std::cout << "state -> transition" << std::endl;
    }
    if(s == logic::statement::final) {
      std::cout << "state -> final" << std::endl;
    }

    std::cout << term_to_string(t) << std::endl;
  }

  void automaton_t::impl_t::push() {
    std::cout << "push" << std::endl;
  }

  void automaton_t::impl_t::pop(size_t n) {
    std::cout << "pop" << std::endl;
    _next->pop(n);
  }

  std::optional<object> automaton_t::impl_t::translate(object) {
    
    return {};
  }

  term automaton_t::impl_t::undo(term t) {
    return t;
  }
}

using namespace black;
using namespace black::logic;

int main () {
  // phi è la formula FOLTL
  module phi;

  object p = phi.declare("p", types::boolean(), role::rigid, resolution::delayed); // no adopt
  object q = phi.declare("q", types::boolean(), role::rigid, resolution::immediate); // adopt

  phi.resolve(recursion::forbidden); // adopt

  // ...
  // Definisco la mia formula in FO_LTL
  // ...

  phi.require(p && q); // state
  
  
  pipes::transform encoding = pipes::automaton();
  module A = encoding(phi);
  object r = A.declare("r", types::boolean());

  return 0;
}