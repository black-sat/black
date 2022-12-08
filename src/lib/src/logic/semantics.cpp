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

namespace black_internal::logic {
  
  scope::scope(alphabet &sigma)
    : _sigma{&sigma}, _frame{std::nullopt} { }


  void scope::push(std::vector<var_decl> decls, rigid_t r) {
    std::vector<declaration> v;
    for(auto d : decls)
      v.push_back(d);

    _frame = _sigma->frame(_frame, r, v);
  }

  void scope::push(std::vector<rel_decl> decls, rigid_t r) {
    std::vector<declaration> v;
    for(auto d : decls)
      v.push_back(d);

    _frame = _sigma->frame(_frame, r, v);
  }

  void scope::push(std::vector<fun_decl> decls, rigid_t r) {
    std::vector<declaration> v;
    for(auto d : decls)
      v.push_back(d);

    _frame = _sigma->frame(_frame, r, v);
  }

  void scope::push(std::vector<sort_decl> decls) {
    std::vector<declaration> v;
    for(auto d : decls)
      v.push_back(d);

    _frame = _sigma->frame(_frame, rigid_t::non_rigid, v);
  }

  void scope::push(variable var, class sort s, rigid_t r) {
    push({_sigma->var_decl(var, s)}, r);
  }

  void scope::push(relation rel, std::vector<class sort> sorts, rigid_t r) {
    push({_sigma->rel_decl(rel, sorts)}, r);
  }
  
  void scope::push(relation rel, std::vector<var_decl> decls, rigid_t r) {
    std::vector<class sort> sorts;
    for(auto d : decls)
      sorts.push_back(d.sort());
    push({_sigma->rel_decl(rel, sorts)}, r);
  }

  void scope::push(
    function fun, class sort s, std::vector<class sort> sorts, rigid_t r
  ) {
    push({_sigma->fun_decl(fun, s, sorts)}, r);
  }
  
  void scope::push(
    function fun, class sort s, std::vector<var_decl> decls, rigid_t r
  ) {
    std::vector<class sort> sorts;
    for(auto d : decls)
      sorts.push_back(d.sort());
    push({_sigma->fun_decl(fun, s, sorts)}, r);
  }

  void scope::push(named_sort s, domain_ref domain) {
    std::vector<var_decl> decls;
    for(variable x : domain->elements())
      decls.push_back(_sigma->var_decl(x, s));
    
    push(decls, rigid_t::rigid);
    push({_sigma->sort_decl(s, domain)});
  }

  void scope::pop() {
    if(_frame)
      _frame = _frame->next();
  }

  std::optional<sort> scope::sort(variable x) const {
    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<var_decl>(); d.has_value()) {
          if(d->variable() == x)
            return d->sort();
        }
      }
      current = current->next();
    }

    if(x.subscript())
      return sort(_sigma->variable(x.name()));

    return std::nullopt;
  }

  std::optional<sort> scope::sort(function fun) const {
    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<fun_decl>(); d.has_value()) {
          if(d->function() == fun)
            return d->sort();
        }
      }
      current = current->next();
    }

    if(fun.subscript())
      return sort(_sigma->function(fun.name()));

    return std::nullopt;
  }

  std::optional<std::vector<sort>> scope::signature(function fun) const {
    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<fun_decl>(); d.has_value()) {
          if(d->function() == fun)
            return d->signature();
        }
      }
      current = current->next();
    }

    if(fun.subscript())
      return signature(_sigma->function(fun.name()));
    return {};
  }

  std::optional<std::vector<sort>> scope::signature(relation rel) const {
    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<rel_decl>(); d.has_value()) {
          if(d->relation() == rel)
            return d->signature();
        }
      }
      current = current->next();
    }

    if(rel.subscript())
      return signature(_sigma->relation(rel.name()));
    return {};
  }

  domain const*scope::domain(class sort s) const {
    if(!s.is<named_sort>())
      return nullptr;
    
    named_sort n = *s.to<named_sort>();

    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<sort_decl>(); d.has_value()) {
          if(d->sort() == n)
            return d->domain().get();
        }
      }
      current = current->next();
    }

    return nullptr;
  }

  bool scope::is_rigid(variable x) const {
    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<var_decl>(); d.has_value()) {
          if(d->variable() == x)
            return current->rigid() == rigid_t::rigid;
        }
      }
      current = current->next();
    }

    if(x.subscript())
      return is_rigid(_sigma->variable(x.name()));
    return false;
  }

  bool scope::is_rigid(relation rel) const {
    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<rel_decl>(); d.has_value()) {
          if(d->relation() == rel)
            return current->rigid() == rigid_t::rigid;
        }
      }
      current = current->next();
    }

    if(rel.subscript())
      return is_rigid(_sigma->relation(rel.name()));
    return false;
  }

  bool scope::is_rigid(function fun) const {
    std::optional<class frame> current = _frame;

    while(current) {
      for(auto decl : current->decls()) {
        if(auto d = decl.to<fun_decl>(); d.has_value()) {
          if(d->function() == fun)
            return current->rigid() == rigid_t::rigid;
        }
      }
      current = current->next();
    }

    if(fun.subscript())
      return is_rigid(_sigma->function(fun.name()));
    return false;
  }

  struct type_checker 
  {
    template<typename F>
    type_checker(scope &_xi, std::optional<sort> _sort, F _err) 
      : global{_xi}, xi{_xi}, default_sort{_sort}, err{_err} { }

    bool type_check_rel_func(hierarchy auto h, auto terms);

    std::optional<sort> type_check(term<LTLPFO> t);
    bool type_check(formula<LTLPFO> f);

    scope &global;
    scope xi;
    std::optional<sort> default_sort;
    std::function<void(std::string)> err;
  };

  std::optional<class sort>
  scope::type_check(
    term<LTLPFO> t, std::optional<class sort> default_sort, 
    std::function<void(std::string)> err
  ) {
    type_checker checker{*this, default_sort, err};

    return checker.type_check(t);
  }

  bool scope::type_check(
    formula<LTLPFO> f, std::optional<class sort> default_sort, 
    std::function<void(std::string)> err
  ) {
    type_checker checker{*this, default_sort, err};

    return checker.type_check(f);
  }

  static void push_fun_rel(
    scope &xi, function fun, sort s, std::vector<sort> sorts
  ) {
    xi.push(fun, s, sorts);
  }
  
  static void push_fun_rel(
    scope &xi, relation rel, sort, std::vector<sort> sorts
  ) {
    xi.push(rel, sorts);
  }

  bool type_checker::type_check_rel_func(hierarchy auto h, auto terms) {
    std::vector<sort> sorts;
    for(auto t : terms) {
      auto s = type_check(t);
      if(!s.has_value())
        return false;
      
      sorts.push_back(*s);
    }

    auto signature = global.signature(h);
    if(!signature.has_value()) {
      if(!default_sort.has_value()) {
        err("Use of undeclared function/relation '" + to_string(h) + "'");
        return false;
      }
      push_fun_rel(global, h, *default_sort, sorts);
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
    using S = std::optional<class sort>;

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

        if(default_sort) {
          global.push(x, *default_sort);
          xi.push(x, *default_sort);
          return default_sort;
        }
        
        err("Use of undeclared variable '" + to_string(x) + "'");
        return {};
      },
      [&](application<LTLPFO>, auto func, auto terms) -> S {
        if(!type_check_rel_func(func, terms))
          return {};

        return xi.sort(func);
      },
      [&](to_integer<LTLPFO>, auto arg) -> S {
        std::optional<class sort> argsort = type_check(arg);
        if(!argsort)
          return {};
        if(!argsort->is<arithmetic_sort>()) {
          err("to_int() can only be applied to terms of arithmetic sort");
          return {};
        }
          
        return t.sigma()->integer_sort();
      },
      [&](to_real<LTLPFO>, auto arg) -> S {
        std::optional<class sort> argsort = type_check(arg);
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
        std::optional<class sort> leftsort = type_check(left);
        if(!leftsort.has_value())
          return {};

        std::optional<class sort> rightsort = type_check(right);
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
        std::optional<class sort> leftsort = type_check(left);
        if(!leftsort.has_value())
          return {};

        std::optional<class sort> rightsort = type_check(right);
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
          if(!ts)
            return false;
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
        std::optional<class sort> leftsort = type_check(left);
        std::optional<class sort> rightsort = type_check(right);

        if(!leftsort || !rightsort)
          return false;

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
      [&](quantifier<LTLPFO> q) {
        nest_scope_t nest{xi};

        xi.push(q.variables(), scope::rigid);
        
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
