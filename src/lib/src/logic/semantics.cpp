//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#include <black/logic/logic.hpp>

#include <tsl/hopscotch_map.h>

namespace black_internal::logic {
  
  struct scope::impl_t {
    
    struct frame_t 
    {  
      tsl::hopscotch_map<variable, struct sort> vars;
      tsl::hopscotch_map<relation, std::vector<struct sort>> rels;
      tsl::hopscotch_map<
        function, std::pair<struct sort, std::vector<struct sort>>
      > funcs;

      std::optional<struct sort> default_sort;
      std::shared_ptr<const frame_t> next;

      frame_t(
        std::optional<struct sort> d,
        std::shared_ptr<const frame_t> n
      ) : default_sort{d}, next{std::move(n)} { }

      frame_t(frame_t const&) = default;
      frame_t &operator=(frame_t const&) = default;
    };

    impl_t(alphabet &a, std::optional<struct sort> default_sort)
      : sigma{a},
        frame{std::make_shared<frame_t>(default_sort, nullptr)} { }
    
    impl_t(scope const&s)
      : sigma{s._impl->sigma},
        frame{std::make_shared<frame_t>(s.default_sort(), s._impl->frame)} { }

    impl_t(impl_t const& i)
      : sigma{i.sigma},
        frame{std::make_shared<frame_t>(*i.frame)} { }

    alphabet &sigma;
    std::shared_ptr<frame_t> frame;
    mutable tsl::hopscotch_map<term<LTLPFO>, struct sort> cache;
  };

  scope::scope(alphabet &sigma, std::optional<struct sort> def) 
    : _impl{std::make_unique<impl_t>(sigma, def)} { }
  
  scope::scope(chain_t c) : _impl{std::make_unique<impl_t>(c.s)} { }

  scope::~scope() = default;

  scope::scope(scope &&) = default;
  scope &scope::operator=(scope &&) = default;

  scope::scope(scope const& s) 
    : _impl{std::make_unique<impl_t>(*s._impl)} { }

  scope &scope::operator=(scope const& s) {
    _impl = std::make_unique<impl_t>(*s._impl);

    return *this;
  }

  void scope::set_default_sort(std::optional<struct sort> s) {
    _impl->frame->default_sort = s;
  }

  std::optional<sort> scope::default_sort() const {
    return _impl->frame->default_sort;
  }

  void scope::declare_variable(variable x, struct sort s) {
    _impl->frame->vars.insert({x,s});
  }

  void scope::declare_function(
    function f, struct sort s, std::vector<struct sort> args
  ) {
    _impl->frame->funcs.insert({f, {s, std::move(args)}});
  }
  
  void scope::declare_relation(relation r, std::vector<struct sort> args) {
    _impl->frame->rels.insert({r, std::move(args)});
  }

  std::optional<sort> scope::sort(variable x) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->vars.find(x); it != current->vars.end())
        return it->second;
      current = current->next;
    }

    return _impl->frame->default_sort;
  }

  std::optional<sort> scope::sort(function f) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->funcs.find(f); it != current->funcs.end())
        return it->second.first;
      current = current->next;
    }

    return _impl->frame->default_sort;
  }

  std::optional<std::vector<sort>> scope::signature(function f) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->funcs.find(f); it != current->funcs.end())
        return it->second.second;
      current = current->next;
    }

    return {};
  }

  std::optional<std::vector<sort>> scope::signature(relation r) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->rels.find(r); it != current->rels.end())
        return it->second;
      current = current->next;
    }

    return {};
  }

  std::optional<sort> scope::sort(term<LTLPFO> t) const {
    using S = std::optional<struct sort>;

    if(auto it = _impl->cache.find(t); it != _impl->cache.end())
      return it->second;

    S result = t.match(
      [&](constant<LTLPFO>, auto value) -> S {
        return value.match(
          [&](integer) { return _impl->sigma.integer_sort(); },
          [&](real)    { return _impl->sigma.real_sort(); }
        );
      },
      [&](variable x) {
        return sort(x);
      },
      [&](application<LTLPFO> app) {
        return sort(app.func());
      },
      [&](to_integer<LTLPFO>) -> S {
        return _impl->sigma.integer_sort();
      },
      [&](to_real<LTLPFO>) -> S {
        return _impl->sigma.real_sort();
      },
      [&](unary_term<LTLPFO>, auto arg) { 
        return sort(arg);
      },
      [&](binary_term<LTLPFO>, auto left, [[maybe_unused]] auto right) -> S {
        black_assert(sort(left) == sort(right));
        
        return sort(left);
      }
    );

    if(result)
      _impl->cache.insert({t, *result});

    return result;
  }

}
