//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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
//

#include <black/sdd/sdd.hpp>
#include <black/sdd/sdd.h>

#include <tsl/hopscotch_map.h>

#include <cstdlib>
#include <random>
#include <iostream>
#include <random>

namespace black::sdd {

  template<typename T>
  auto free_later(T *ptr) {
    return std::unique_ptr<T[], void(*)(void*)>(ptr, &free);
  }

  //
  // manager
  //
  struct manager::impl_t {
    impl_t(alphabet *_sigma, size_t nvars) 
      : sigma{_sigma}, mgr{sdd_manager_create(SddLiteral(nvars), 0)}
      { 
        black_assert(nvars > 0);
      }

    tsl::hopscotch_map<proposition, sdd::variable> map;
    std::vector<sdd::variable> vars;
    tsl::hopscotch_map<node, logic::formula<logic::propositional>> formulas;
    
    alphabet *sigma;
    SddManager *mgr;
    SddLiteral next_var = 1;
    tsl::hopscotch_map<
      black::logic::formula<black::logic::QBF>, 
      node
    > to_node_cache;
  };

  manager::manager(alphabet *sigma, size_t vars) 
    : _impl{std::make_unique<impl_t>(sigma, vars)} { }

  manager::manager(manager &&) = default;
  manager &manager::operator=(manager &&) = default;
  manager::~manager() = default;

  void manager::minimize() const {
    sdd_manager_minimize(handle());
  }

  variable manager::variable(proposition name) {
    if(_impl->map.contains(name))
      return _impl->map.at(name);

    while(_impl->next_var > sdd_manager_var_count(_impl->mgr))
      sdd_manager_add_var_after_last(_impl->mgr);

    SddLiteral var = _impl->next_var++;
    
    sdd::variable result{this, name, unsigned(var)};
    _impl->vars.push_back(result);
    _impl->map.insert({name, result});

    return result;
  }

  std::vector<class variable> 
  manager::variables(std::vector<proposition> props) {
    std::vector<class variable> result;
    for(auto prop : props)
      result.push_back(variable(prop));
    return result;
  }

  node manager::top() {
    return node{this, sdd_manager_true(handle())};
  }

  node manager::bottom() {
    return node{this, sdd_manager_false(handle())};
  }

  alphabet *manager::sigma() const {
    return _impl->sigma;
  }

  SddManager *manager::handle() const {
    return _impl->mgr;
  }

  node manager::to_node(black::logic::formula<black::logic::QBF> f) {
    using logic::QBF;

    if(_impl->to_node_cache.contains(f))
      return _impl->to_node_cache.at(f);

    auto result = f.match(
      [&](boolean, bool value) {
        return value ? top() : bottom();
      },
      [&](proposition p) {
        return sdd::node(variable(p));
      },
      [&](logic::negation<QBF>, auto arg) {
        return !to_node(arg);
      },
      [&](logic::conjunction<QBF> c) {
        sdd::node acc = top();
        for(auto op : c.operands())
          acc = acc && to_node(op);
        return acc;
      },
      [&](logic::disjunction<QBF> c) {
        sdd::node acc = bottom();
        for(auto op : c.operands())
          acc = acc || to_node(op);
        return acc;
      },
      [&](logic::implication<QBF>, auto left, auto right) {
        return sdd::implies(to_node(left), to_node(right));
      },
      [&](logic::iff<QBF>, auto left, auto right) {
        return sdd::iff(to_node(left), to_node(right));
      },
      [&](logic::qbf<QBF> q, auto qvars, auto matrix) {
        sdd::node sddmatrix = to_node(matrix);
        if(qvars.empty())
          return sddmatrix;
        
        std::vector<sdd::variable> vars;
        for(auto qvar : qvars)
          vars.push_back(variable(qvar));

        return q.node_type().match(
          [&](logic::qbf<QBF>::type::thereis) {
            return sdd::exists(vars, sddmatrix);
          },
          [&](logic::qbf<QBF>::type::foreach) {
            return sdd::forall(vars, sddmatrix);
          }
        );
      }
    );

    _impl->to_node_cache.insert({f, result});
    return result;
  }

  logic::formula<logic::propositional> manager::to_formula(node n) {
    if(_impl->formulas.contains(n))
      return _impl->formulas.at(n);
    
    auto result = [&]() -> logic::formula<logic::propositional> {
      if(n.is_valid())
        return _impl->sigma->top();
      else if(n.is_unsat())
        return _impl->sigma->bottom();
      else if(n.is_literal()) {
        sdd::literal lit = n.literal().value();
        auto prop = _impl->vars[lit.variable().handle() - 1].name();
        if(lit.sign())
          return prop;
        return !prop;
      } else {
        black_assert(n.is_decision());

        auto elements = n.elements();
        return big_or(*_impl->sigma, elements, [&](auto elem) {
          return to_formula(elem.prime) && to_formula(elem.sub);
        });
      }
    }();
    
    _impl->formulas.insert({n, result});
    return result;
  }

  //
  // variable and literal
  //
  variable::variable(class manager *mgr, unsigned var) 
    : _mgr{mgr}, _name{_mgr->_impl->vars[var - 1].name()}, _var{var} { }

  literal::literal(class manager *mgr, SddLiteral lit)
    : _var{mgr, unsigned(std::abs(lit))}, _sign{lit > 0} { }

  //
  // node
  //
  // node::node(class manager *mgr, SddNode *n) 
  //   : _mgr{mgr}, _node{
  //     std::shared_ptr<SddNode>{
  //       sdd_ref(n, mgr->handle()), [=](SddNode *_n) {
  //         sdd_deref(_n, mgr->handle());
  //       }
  //     }
  //   } { }
  node::node(class manager *mgr, SddNode *n) 
    : _mgr{mgr}, _node{sdd_ref(n, mgr->handle())} { }  

  node::node(class literal lit) : _mgr{lit.manager()}, _node{
    sdd_manager_literal(lit.handle(), _mgr->handle())
  } { }

  node::node(variable var) : node{(class literal)(var)} { }

  sdd::node node::minimize() const {
    Vtree *tree = sdd_vtree_of(handle());
    if(!tree)
      return *this;
    
    sdd_vtree_minimize(tree, manager()->handle());

    return *this;
  }

  std::vector<variable> node::variables() const {
    auto array = free_later(sdd_variables(handle(), manager()->handle()));
    
    std::vector<variable> result;
    for(unsigned i = 1; i <= sdd_manager_var_count(manager()->handle()); ++i) {
      if(array[i])
        result.push_back(manager()->_impl->vars[i - 1]);
    }
    return result;
  }

  size_t node::count() const {
    return sdd_count(handle());
  }

  bool node::is_valid() const {
    return sdd_node_is_true(handle());
  }

  bool node::is_unsat() const {
    return sdd_node_is_false(handle());
  }

  bool node::is_sat() const {
    return !is_unsat();
  }

  std::optional<std::vector<literal>> node::model() const 
  {
    std::vector<sdd::literal> model;

    auto vars = variables();
    
    node n = *this;
    size_t i = 0;
    while(true) { // vars.size() iterations max anyway
      if(n.is_unsat())
        return {};
      if(n.is_valid())
        return model;
      
      class literal lit = vars.at(i);
      node choice = n.condition(lit);
      
      if(choice.is_unsat()) {
        lit = !lit;
        choice = n.condition(lit);
      }

      // arbitrary choice now
      model.push_back(lit);
      n = choice;
      ++i;
    }
  }

  bool node::is_literal() const {
    return sdd_node_is_literal(handle());
  }

  bool node::is_decision() const {
    return sdd_node_is_decision(handle());
  }

  std::optional<literal> node::literal() const {
    if(!is_literal())
      return {};
    
    return sdd::literal{manager(), sdd_node_literal(handle())};
  }

  std::vector<element> node::elements() const {
    if(!is_decision())
      return {};

    SddNode **elems = sdd_node_elements(handle());
    size_t nelems = sdd_node_size(handle());

    std::vector<element> result;
    for(size_t i = 0; i < nelems; i++) {
      result.push_back(element{
        node{manager(), elems[2 * i]},
        node{manager(), elems[2 * i + 1]}
      });
    }

    return result;
  }

  node node::condition(class literal lit) const {
    return node{
      manager(), sdd_condition(lit.handle(), handle(), manager()->handle())
    };
  }

  node node::condition(std::vector<class literal> const& lits) const {
    node n = *this;
    for(auto lit : lits) {
      n = n.condition(lit);
      if(n.is_valid() || n.is_unsat())
        break;
    }
    return n;
  }

  node node::condition(std::vector<class variable> const& v, bool sign) const 
  {
    node n = *this;
    for(auto var : v) {
      n = n.condition(sign ? var : !var);
      if(n.is_valid() || n.is_unsat())
        break;
    }
    return n;
  }

  node node::condition(
    std::vector<black::proposition> const& props, bool sign
  ) const {
    std::vector<variable> vars;
    for(auto p : props)
      vars.push_back(manager()->variable(p));
    return condition(vars, sign);
  }

  node node::rename(std::function<black::proposition(black::proposition)> map) 
  {
    auto f = manager()->to_formula(*this);
    auto renamed = black_internal::rename(f, map);
    return manager()->to_node(renamed);
  }

  node node::change(std::function<black::proposition(black::proposition)> map)
  {
    sdd::node changes = manager()->top();
    std::vector<sdd::variable> old;
    
    for(auto v : variables()) {
      auto p = map(v.name());
      if(p != v.name()) {
        old.push_back(v);
        changes = changes && iff(v, manager()->variable(p));
      }
    }

    return exists(old, changes && *this);
  }

  std::ostream &operator<<(std::ostream &str, literal const& lit) {
    if(lit.sign())
      str << black::to_string(lit.variable().name());
    else
      str << "!" << black::to_string(lit.variable().name());
    return str;
  }

  std::ostream &operator<<(std::ostream &str, std::vector<literal> const&lits) {
    if(lits.empty()) {
      str << "{ }";
      return str;
    }
      
    str << "{ ";
    for(size_t i = 0; i < lits.size() - 1; i++) {
      str << lits[i] << ", ";
    }
    str << lits.back();
    str << " }";

    return str;
  }

  node operator!(node n) {
    return node{
      n.manager(),
      sdd_negate(n.handle(), n.manager()->handle())
    };
  }

  node operator&&(node n1, node n2) {
    return node{
      n1.manager(),
      sdd_conjoin(n1.handle(), n2.handle(), n1.manager()->handle())
    };
  }

  node operator||(node n1, node n2) {
    return node{
      n1.manager(),
      sdd_disjoin(n1.handle(), n2.handle(), n1.manager()->handle())
    };
  }

  node exists(variable var, node n) {
    return node{
      n.manager(), 
      sdd_exists(literal{var}.handle(), n.handle(), n.manager()->handle())
    };
  }
   
  node exists(std::vector<variable> const& vars, node n) {
    std::vector<int> map(
      size_t(sdd_manager_var_count(n.manager()->handle()) + 1), 0
    );

    for(auto var : vars)
      map[var.handle()] = 1;
    
    return node{
      n.manager(), 
      sdd_exists_multiple(map.data(), n.handle(), n.manager()->handle())
    };
  }

  node exists(std::vector<black::proposition> const& vars, node n) {
    std::vector<sdd::variable> sddvars;
    for(auto v : vars)
      sddvars.push_back(n.manager()->variable(v));
    return exists(sddvars, n);
  }

  node forall(variable var, node n) {
    return node{
      n.manager(), 
      sdd_forall(literal{var}.handle(), n.handle(), n.manager()->handle())
    };
  }

  node forall(std::vector<variable> const& vars, node n) {
    for(auto v : vars)
      n = forall(v, n);
    return n;
  }

  node forall(std::vector<black::proposition> const& vars, node n) {
    std::vector<sdd::variable> sddvars;
    for(auto v : vars)
      sddvars.push_back(n.manager()->variable(v));
    return forall(sddvars, n);
  }

  node implies(node n1, node n2) {
    return !n1 || n2;
  }

  node iff(node n1, node n2) {
    return implies(n1, n2) && implies(n2, n1);
  }  

}