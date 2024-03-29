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

#include <black/support>
#include <black/support/private>
#include <black/logic>
#include <black/pipes>

namespace black::pipes {

  namespace persistent = support::persistent;

  struct map_t::impl_t : consumer {
    impl_t(class consumer *next, std::function<logic::term(logic::term)> f)
      : _next(next), _map{std::move(f)} { }

    class consumer *_next;
    std::function<logic::term(logic::term)> _map;

    persistent::map<logic::entity const *, logic::entity const *> _entities;

    virtual void import(logic::module) override;
    
    virtual void adopt(std::shared_ptr<logic::root const>) override;
    
    virtual void require(logic::term) override;
    
    virtual void push() override;
    
    virtual void pop(size_t) override;

  };

  map_t::map_t(class consumer *next, std::function<logic::term(logic::term)> f) 
    : _impl{std::make_unique<impl_t>(next, std::move(f))} { }

  map_t::~map_t() = default;

  consumer *map_t::consumer() { return _impl.get(); }

  void map_t::impl_t::import(logic::module) {
    // ...
  }
    
  void map_t::impl_t::adopt(std::shared_ptr<logic::root const> r) {
    using namespace logic;
    module m;
    
    for(auto const& e : r->entities) {
      object obj = e->value ?
        m.define(e->name, _map(e->type), _map(*e->value), resolution::delayed)
      : m.declare(e->name, _map(e->type), resolution::delayed);

      _entities.set(e.get(), obj.entity());
    }
    
    _next->adopt(m.resolve(r->mode));
  }
  
  void map_t::impl_t::require(logic::term t) {
    
  }
  
  void map_t::impl_t::push() {
    _entities.push();
    _next->push();
  }
  
  void map_t::impl_t::pop(size_t n) {
    _entities.pop(n);
    _next->pop(n);
  }


}