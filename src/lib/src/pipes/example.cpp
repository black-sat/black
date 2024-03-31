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
  using namespace support;
  using namespace logic;

  struct example_t::impl_t : public consumer
  {
    impl_t(class consumer *next) : _next{next} { }

    class consumer *_next;
    persistent::map<entity const *, entity const *> _replacements;

    decltype(auto) to_ints(auto const& v) { return v; }
    term to_ints(term);
    std::vector<term> to_ints(std::vector<term>);

    virtual void import(logic::module) override;
    virtual void adopt(std::shared_ptr<logic::root const>) override;
    virtual void require(logic::term) override;
    virtual void push() override;
    virtual void pop(size_t) override;
  };

  example_t::example_t(class consumer *next)
    : _impl{std::make_unique<impl_t>(next)} { }

  example_t::~example_t() = default;

  consumer *example_t::consumer() { return _impl.get(); }
  
  term example_t::impl_t::to_ints(term t) {
    return match(t)(
      [](real, double v) { return integer(uint64_t(v)); },
      [&](object o, auto e) {
        if(auto ptr = _replacements.find(e); ptr)
          return object(*ptr);
        return o;
      },
      [&]<typename T>(T, auto ...args) { return T(to_ints(args)...); }
    );
  }

  std::vector<term> example_t::impl_t::to_ints(std::vector<term> ts) {
    for(term &t : ts)
      t = to_ints(t);
    return ts;
  }

  void example_t::impl_t::import(logic::module m) {
    _next->import(std::move(m));
  }

  void example_t::impl_t::adopt(std::shared_ptr<logic::root const> r) {
    using namespace logic;

    module m;

    bool flag = false;
    for(auto const& e : r->entities) {
      types::type type = e->type == types::real() ? types::integer() : e->type;
      std::optional<term> value = e->value;
      if(value)
        value = to_ints(*value);

      if(type != e->type || value != e->value) {
        object obj = e->value ? 
          m.define(e->name, type, *value, resolution::delayed) 
        : m.declare(e->name, type, resolution::delayed);
        
        _replacements.set(e.get(), obj.entity());
        flag = true;
      }
    }

    if(flag)
      _next->adopt(m.resolve(r->mode));
    else
      _next->adopt(r);
  }

  void example_t::impl_t::require(logic::term t) {
    _next->require(to_ints(t));
  }

  void example_t::impl_t::push() {
    _replacements.push();
    _next->push();
  }

  void example_t::impl_t::pop(size_t n) {
    _replacements.pop(n);
    _next->pop(n);
  }



}