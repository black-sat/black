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
#include <black/logic/prettyprint.hpp>

#include <tsl/hopscotch_map.h>

#include <any>

#include <iostream>

namespace black_internal::logic {
  
  struct scope::impl_t {
    
    struct var_record_t {
      struct sort sort;
      rigid_t rigid;
    };

    struct rel_record_t {
      std::vector<struct sort> signature;
      rigid_t rigid;
    };

    struct func_record_t {
      struct sort result;
      std::vector<struct sort> signature;
      rigid_t rigid;
    };

    struct frame_t 
    {  
      std::vector<domain_ref> domains;

      tsl::hopscotch_map<variable, var_record_t> vars;
      tsl::hopscotch_map<relation, rel_record_t> rels;
      tsl::hopscotch_map<function, func_record_t> funcs;
      tsl::hopscotch_map<struct sort, size_t> sorts;
      
      tsl::hopscotch_map<variable, std::any> vars_data;
      tsl::hopscotch_map<relation, std::any> rels_data;
      tsl::hopscotch_map<function, std::any> funcs_data;
      tsl::hopscotch_map<struct sort, std::any> sorts_data;

      std::optional<struct sort> default_sort;
      std::shared_ptr<const frame_t> next;

      frame_t(
        std::optional<struct sort> d,
        std::shared_ptr<const frame_t> n
      ) : default_sort{d}, next{std::move(n)} { }

      frame_t(frame_t const&) = delete;
      frame_t &operator=(frame_t const&) = delete;
    };

    impl_t(alphabet &a, std::optional<struct sort> default_sort)
      : sigma{a},
        frame{std::make_shared<frame_t>(default_sort, nullptr)} { }
    
    impl_t(scope const&s)
      : sigma{s._impl->sigma},
        frame{std::make_shared<frame_t>(s.default_sort(), s._impl->frame)} { }

    alphabet &sigma;
    std::shared_ptr<frame_t> frame;
  };

  scope::scope(alphabet &sigma, std::optional<struct sort> def) 
    : _impl{std::make_unique<impl_t>(sigma, def)} { }
  
  scope::scope(chain_t c) : _impl{std::make_unique<impl_t>(c.s)} { }

  scope::~scope() = default;

  scope::scope(scope &&) = default;
  scope &scope::operator=(scope &&) = default;

  void scope::set_default_sort(std::optional<struct sort> s) {
    _impl->frame->default_sort = s;
  }

  std::optional<sort> scope::default_sort() const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(current->default_sort.has_value())
        return current->default_sort;
      current = current->next;
    }

    return {};
  }

  void scope::declare(variable x, struct sort s, rigid_t r) {
    _impl->frame->vars.insert({x, {s,r}});
  }

  void scope::declare(
    function f, struct sort s, std::vector<struct sort> args, rigid_t r
  ) {
    _impl->frame->funcs.insert({f, {s, std::move(args), r}});
  }
  
  void scope::declare(
    function f, std::vector<struct sort> args, rigid_t r
  ) {
    black_assert(default_sort().has_value());
    _impl->frame->funcs.insert({f, {*default_sort(), std::move(args), r}});
  }
  
  void scope::declare(
    relation r, std::vector<struct sort> args, rigid_t rigid
  ) {
    _impl->frame->rels.insert({r, {std::move(args), rigid}});
  }

  void scope::declare(struct sort s, domain_ref domain) {
    _impl->frame->domains.push_back(std::move(domain));
    _impl->frame->sorts.insert({s, _impl->frame->domains.size() - 1});
    for(variable x : domain->elements())
      declare(x, s, scope::rigid);
  }

  std::optional<sort> scope::sort(variable x) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->vars.find(x); it != current->vars.end())
        return it->second.sort;
      current = current->next;
    }

    return default_sort();
  }

  std::optional<sort> scope::sort(function f) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->funcs.find(f); it != current->funcs.end())
        return it->second.result;
      current = current->next;
    }

    return default_sort();
  }

  std::optional<std::vector<sort>> scope::signature(function f) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->funcs.find(f); it != current->funcs.end())
        return it->second.signature;
      current = current->next;
    }

    return {};
  }

  std::optional<std::vector<sort>> scope::signature(relation r) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->rels.find(r); it != current->rels.end())
        return it->second.signature;
      current = current->next;
    }

    return {};
  }

  domain const*scope::domain(struct sort s) const {
    if(auto it = _impl->frame->sorts.find(s); it != _impl->frame->sorts.end()) {
      black_assert(it->second < _impl->frame->domains.size());
      return _impl->frame->domains[it->second].get();
    }

    return nullptr;
  }

  bool scope::is_rigid(variable x) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->vars.find(x); it != current->vars.end())
        return it->second.rigid == rigid_t::rigid;
      current = current->next;
    }

    return false;
  }

  bool scope::is_rigid(relation r) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->rels.find(r); it != current->rels.end())
        return it->second.rigid == rigid_t::rigid;
      current = current->next;
    }

    return false;
  }

  bool scope::is_rigid(function f) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->funcs.find(f); it != current->funcs.end())
        return it->second.rigid == rigid_t::rigid;
      current = current->next;
    }

    return false;
  }

  void scope::set_data_inner(variable x, std::any data) {
    _impl->frame->vars_data.insert({x, std::move(data)});
  }

  void scope::set_data_inner(relation r, std::any data) {
    _impl->frame->rels_data.insert({r, std::move(data)});
  }

  void scope::set_data_inner(function f, std::any data) {
    _impl->frame->funcs_data.insert({f, std::move(data)});
  }

  void scope::set_data_inner(struct sort s, std::any data) {
    _impl->frame->sorts_data.insert({s, std::move(data)});
  }

  std::any scope::data_inner(variable x) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->vars_data.find(x); it != current->vars_data.end())
        return it->second;
      current = current->next;
    }

    return {};
  }

  std::any scope::data_inner(relation r) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->rels_data.find(r); it != current->rels_data.end())
        return it->second;
      current = current->next;
    }

    return {};
  }

  std::any scope::data_inner(function f) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->funcs_data.find(f); it != current->funcs_data.end())
        return it->second;
      current = current->next;
    }

    return {};
  }

  std::any scope::data_inner(struct sort s) const {
    std::shared_ptr<const impl_t::frame_t> current = _impl->frame;

    while(current) {
      if(auto it = current->sorts_data.find(s); it != current->sorts_data.end())
        return it->second;
      current = current->next;
    }

    return {};
  }

  struct type_checker 
  {
    template<typename F>
    type_checker(scope &_xi, F _err) 
      : global{_xi}, xi{chain(_xi)}, err{_err} { }

    bool type_check_rel_func(hierarchy auto h, auto terms);

    std::optional<sort> type_check(term<LTLPFO> t);
    bool type_check(formula<LTLPFO> f);

    scope &global;
    scope xi;
    std::function<void(std::string)> err;
  };

  std::optional<struct sort>
  scope::type_check(term<LTLPFO> t, std::function<void(std::string)> err) {
    type_checker checker{*this, err};

    return checker.type_check(t);
  }

  bool 
  scope::type_check(formula<LTLPFO> f, std::function<void(std::string)> err) {
    type_checker checker{*this, err};

    return checker.type_check(f);
  }

  bool type_checker::type_check_rel_func(hierarchy auto h, auto terms) {
    std::vector<sort> sorts;
    for(auto t : terms) {
      auto s = type_check(t);
      if(!s.has_value())
        return false;
      
      sorts.push_back(*s);
    }

    auto signature = xi.signature(h);
    if(!signature.has_value()) {
      if(!global.default_sort().has_value()) {
        err("Use of undeclared function/relation '" + to_string(h) + "'");
        return false;
      }
      global.declare(h, sorts);
      signature = sorts;
    }

    if(signature->size() != sorts.size()) {
      err(
        "Function/relation '" + to_string(h) + "' expects " + 
        std::to_string(signature->size()) + " arguments, " + 
        std::to_string(sorts.size()) + " given"
      );
      return false;
    }

    for(size_t i = 0; i < signature->size(); ++i) {
      if(sorts[i] != signature->at(i)) {
        err(
          "In call of function/relation '" + to_string(h) + 
          "', argument " + std::to_string(i) + " has wrong sort (expected '" +
          to_string(signature->at(i)) + "', given '" + 
          to_string(sorts[i]) + "')"
        );
        return false;
      }
    }

    return true;
  }

  std::optional<sort> 
  type_checker::type_check(term<LTLPFO> t) {
    using S = std::optional<struct sort>;

    return t.match(
      [&](constant<LTLPFO>, auto value) -> S {
        return value.match(
          [&](integer) { return t.sigma()->integer_sort(); },
          [&](real)    { return t.sigma()->real_sort(); }
        );
      },
      [&](variable x) -> S { 
        if(auto s = xi.sort(x); s.has_value())
          return *s;
        
        err("Use of undeclared variable '" + to_string(x) + "'");
        return {};
      },
      [&](application<LTLPFO>, auto func, auto terms) -> S {
        if(!type_check_rel_func(func, terms))
          return {};

        return xi.sort(func);
      },
      [&](to_integer<LTLPFO>, auto arg) -> S {
        std::optional<struct sort> argsort = type_check(arg);
        if(!argsort)
          return {};
        if(!argsort->is<arithmetic_sort>()) {
          err("to_int() can only be applied to terms of arithmetic sort");
          return {};
        }
          
        return t.sigma()->integer_sort();
      },
      [&](to_real<LTLPFO>, auto arg) -> S {
        std::optional<struct sort> argsort = type_check(arg);
        if(!argsort)
          return {};
        if(!argsort->is<arithmetic_sort>()) {
          err("to_real() can only be applied to terms of arithmetic sort");
          return {};
        }
          
        return t.sigma()->real_sort();
      },
      [&](unary_term<LTLPFO>, auto arg) { 
        return type_check(arg);
      },
      [&](int_division<LTLPFO>, auto left, auto right) -> S {
        auto leftsort = type_check(left);
        if(!leftsort.has_value())
          return {};

        auto rightsort = type_check(right);
        if(!rightsort.has_value())
          return {};

        if(leftsort != t.sigma()->integer_sort() ||
           rightsort != t.sigma()->integer_sort())
        {
          err("Integer division operator must applied to terms of 'Int' sort");
          return {};
        }

        return t.sigma()->integer_sort();
      },
      [&](division<LTLPFO>, auto left, auto right) -> S {
        std::optional<struct sort> leftsort = type_check(left);
        if(!leftsort.has_value())
          return {};

        std::optional<struct sort> rightsort = type_check(right);
        if(!rightsort.has_value())
          return {};

        if(!leftsort->is<arithmetic_sort>() ||
           !rightsort->is<arithmetic_sort>())
        {
          err("Division operator applied to terms of non-arithmetic sort");
          return {};
        }

        return t.sigma()->real_sort();
      },
      [&](binary_term<LTLPFO>, auto left, auto right) -> S {
        std::optional<struct sort> leftsort = type_check(left);
        if(!leftsort.has_value())
          return {};

        std::optional<struct sort> rightsort = type_check(right);
        if(!rightsort.has_value())
          return {};

        if(!leftsort->is<arithmetic_sort>() ||
           !rightsort->is<arithmetic_sort>())
        {
          err("Arithmetic operator applied to terms of non-arithmetic sort");
          return {};
        }

        if(leftsort != rightsort)
          return t.sigma()->real_sort();
        return *leftsort;
      }
    );
  }

  bool type_checker::type_check(formula<LTLPFO> f) {
    return f.match(
      [](boolean) { return true; },
      [](proposition) { return true; },
      [&](atom<LTLPFO>, auto rel, auto terms) {
        return type_check_rel_func(rel, terms);
      },
      [&](equality<LTLPFO>, auto terms) {
        std::vector<sort> sorts;
        for(auto t : terms) {
          auto ts = type_check(t);
          if(!ts) {
            std::cerr << "Here 2\n";
            return false;
          }
          sorts.push_back(*ts);
        }
        if(
          std::adjacent_find(
            sorts.begin(), sorts.end(), std::not_equal_to<>{}
          ) != sorts.end()
        ) {
          err(
            "Equal/distinct predicate must be applied to terms of equal sort"
          );
          return false;
        }
        return true;
      },
      [&](comparison<LTLPFO>, auto left, auto right) {
        std::optional<struct sort> leftsort = type_check(left);
        std::optional<struct sort> rightsort = type_check(right);

        if(!leftsort || !rightsort) {
          std::cerr << "Here 3\n";
          return false;
        }

        if(!leftsort->is<arithmetic_sort>() ||
           !rightsort->is<arithmetic_sort>()) 
        {
          err(
            "Arithmetic comparison predicates applied to terms "
            "of non-arithmetic sorts"
          );
          return false;
        }
        
        if(leftsort != rightsort) {
          err(
            "Arithmetic comparison operators applied to terms of different "
            "sorts"
          );
          return false;
        }
        return true;
      },
      [&](quantifier_block<LTLPFO> q) {
        nest_scope_t nest{xi};

        for(auto d : q.variables())
          xi.declare(d, scope::rigid);
        
        return type_check(q.matrix());
      },
      [&](unary<LTLPFO>, auto arg) {
        return type_check(arg);
      },
      [&](binary<LTLPFO>, auto left, auto right) {
        return type_check(left) && type_check(right);
      }
    );
  }

}
