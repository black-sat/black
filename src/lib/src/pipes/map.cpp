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
#include <black/ast/algorithms>
#include <black/pipes>

namespace black::pipes::internal {

  namespace persistent = support::persistent;
  using namespace support;
  using namespace logic;
  using namespace ast;

  struct map_t::impl_t : public consumer
  {
    impl_t(
      class consumer *next, 
      type_mapping_t ty_map, term_mapping_t te_map, term_mapping_t back_map
    )
      : _next{next}, _ty_map{ty_map}, _te_map{te_map}, _back_map{back_map} { }

    class consumer *_next;
    type_mapping_t _ty_map;
    term_mapping_t _te_map;
    term_mapping_t _back_map;
    persistent::map<entity const *, entity const *> _replacements;

    std::optional<object> translate(object x);
    term undo(term x);

    virtual void import(logic::module) override;
    virtual void adopt(std::shared_ptr<logic::root const>) override;
    virtual void state(logic::term t, logic::statement s) override;
    virtual void push() override;
    virtual void pop(size_t) override;
  };

  map_t::map_t(
    class consumer *next, 
    type_mapping_t ty_map, term_mapping_t te_map, term_mapping_t back_map
  )
    : _impl{std::make_unique<impl_t>(next, ty_map, te_map, back_map)} { }

  map_t::~map_t() = default;

  consumer *map_t::consumer() { return _impl.get(); }

  std::optional<object> map_t::translate(object x) { 
    return _impl->translate(x); 
  }
  
  term map_t::undo(term x) { return _impl->undo(x); }

  void map_t::impl_t::import(logic::module m) {
    _next->import(std::move(m));
  }

  void map_t::impl_t::adopt(std::shared_ptr<logic::root const> r) {
    using namespace logic;

    module m;

    bool flag = false;
    for(auto const& e : r->entities) {
      if(!e->value) {
        types::type type = _ty_map(e->type);
        if(type != e->type) {
          object obj = m.declare(e->name, type, *e->role, resolution::delayed);
          _replacements.set(e.get(), obj.entity());
          flag = true;
        }
      } else {
        term value = _te_map(*e->value);
        
        if(value != e->value) {
          types::type type = r->mode == recursion::allowed ? 
            _ty_map(e->type) : types::inferred();

          object obj = m.define(e->name, type, value, resolution::delayed);
          _replacements.set(e.get(), obj.entity());
          flag = true;
        }        
      }
    }

    if(flag)
      _next->adopt(m.resolve(r->mode));
    else
      _next->adopt(r);
  }

  void map_t::impl_t::state(logic::term t, logic::statement s) {
    term res = ast::map(_te_map(t))(
      [&](object x) { 
        if(auto y = translate(x); y)
          return *y;
        return x;
      }
    );
    _next->state(res, s);
  }

  void map_t::impl_t::push() {
    _replacements.push();
    _next->push();
  }

  void map_t::impl_t::pop(size_t n) {
    _replacements.pop(n);
    _next->pop(n);
  }

  std::optional<object> map_t::impl_t::translate(object x) {
    if(auto ptr = _replacements.find(x.entity()); ptr)
      return object(*ptr);
    return {};
  }

  term map_t::impl_t::undo(term t) {
    return _back_map(t);
  }



}