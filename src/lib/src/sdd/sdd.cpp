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

#include <black/cudd/cuddObj.hh>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

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
    impl_t(alphabet *_sigma) : sigma{_sigma}, mgr{new Cudd} { }

    tsl::hopscotch_map<proposition, DdNode *> prop_to_var;
    tsl::hopscotch_map<DdNode *, proposition> var_to_prop;
    tsl::hopscotch_map<size_t, DdNode *> index_to_var;
    tsl::hopscotch_map<node, logic::formula<logic::propositional>> formulas;
    
    alphabet *sigma;
    Cudd *mgr;
    int next_var = 0;
    tsl::hopscotch_map<
      black::logic::formula<black::logic::QBF>, 
      node
    > to_node_cache;
  };

  manager::manager(alphabet *sigma) 
    : _impl{std::make_unique<impl_t>(sigma)} { }

  manager::manager(manager &&) = default;
  manager &manager::operator=(manager &&) = default;
  manager::~manager() = default;

  variable manager::variable(proposition name) {
    if(_impl->prop_to_var.contains(name))
      return sdd::variable{this, name, _impl->prop_to_var.at(name)};

    int index = _impl->next_var++;
    DdNode * var = _impl->mgr->bddVar(index).getNode();
    
    _impl->prop_to_var.insert({name, var});
    _impl->var_to_prop.insert({var, name});
    _impl->index_to_var.insert({index, var});

    return sdd::variable{this, name, var};
  }

  node manager::top() {
    return node{this, _impl->mgr->bddOne().getNode()};
  }

  node manager::bottom() {
    return node{this, _impl->mgr->bddZero().getNode()};
  }

  alphabet *manager::sigma() const {
    return _impl->sigma;
  }

  Cudd *manager::handle() const {
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

  //
  // variable and literal
  //
  struct variable::impl_t {
    class manager *_mgr;
    proposition _name;
    DdNode * _var;
  };

  variable::variable(class manager *mgr, proposition name, DdNode * var) 
      : _impl{std::make_unique<impl_t>(mgr, name, var)} { }

  variable::variable(class manager *mgr, DdNode * var) 
    : variable{mgr, mgr->_impl->var_to_prop.at(var), var} { }

  variable::variable(variable const& other) 
    : _impl{std::make_unique<impl_t>(*other._impl)} { }
    
  variable &variable::operator=(variable const& other) {
    _impl = std::make_unique<impl_t>(*other._impl);
    return *this;
  }

  variable::~variable() = default;
  
  variable::variable(variable &&) = default;
  variable &variable::operator=(variable &&) = default;

  proposition variable::name() const { return _impl->_name; }
  class manager *variable::manager() const { return _impl->_mgr; }
  BDD variable::handle() const { 
    return BDD(*manager()->handle(), _impl->_var); 
  }

  node variable::operator!() const {
    return node{ manager(), _impl->_var };
  }

  //
  // node
  //
  struct node::impl_t {
    class manager *_mgr;
    DdNode * _node;
  };

  node::node(class manager *mgr, DdNode * n) 
    : _impl{std::make_unique<impl_t>(mgr, n)} { }  

  node::node(variable var) : node{var.manager(), var.handle().getNode()} { }

  node::~node() = default; 

  node::node(node const& other) 
    : _impl{std::make_unique<impl_t>(*other._impl)} { }

  node &node::operator=(node const& other) {
    _impl = std::make_unique<impl_t>(*other._impl);
    return *this;
  }

  
  node::node(node &&) = default;
  node &node::operator=(node &&) = default;

  class manager *node::manager() const { return _impl->_mgr; }
  BDD node::handle() const { 
    return BDD(*manager()->handle(), _impl->_node); 
  }

  size_t node::hash() const {
    return std::hash<DdNode *>{}(handle().getNode());
  }

  std::vector<variable> node::variables() const {
    tsl::hopscotch_set<variable> vars;
    std::vector<unsigned int> support = handle().SupportIndices();
    for(auto i : support)
      vars.insert(variable{
        manager(),
        manager()->_impl->index_to_var.at(i)
      });
    
    return std::vector<variable>(begin(vars), end(vars));
  }

  bool node::is_valid() const {
    return !(*this).handle().IsZero();
  }

  bool node::is_unsat() const {
    return handle().IsZero();
  }

  bool node::is_sat() const {
    return !is_unsat();
  }

  node node::condition(variable var, bool sign) const {
    BDD literal = sign ? var.handle() : (!var).handle();
    return node{
      manager(),
      handle().Constrain(literal).getNode()
    };
  }

  node node::condition(std::vector<class variable> const& v, bool sign) const 
  {
    node cube = manager()->top();
    for(auto var : v)
      cube = cube && (sign ? var : !var);

    return node{
      manager(),
      handle().Constrain(cube.handle()).getNode()
    };
  }

  node node::condition(
    std::vector<black::proposition> const& props, bool sign
  ) const {
    std::vector<variable> vars;
    for(auto p : props)
      vars.push_back(manager()->variable(p));
    return condition(vars, sign);
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

  node operator!(node n) {
    return node{ n.manager(), (!n.handle()).getNode() };
  }

  node operator&&(node n1, node n2) {
    return node{ n1.manager(), (n1.handle() & n2.handle()).getNode() };
  }

  node operator||(node n1, node n2) {
    return node{ n1.manager(), (n1.handle() | n2.handle()).getNode() };
  }

  node exists(variable var, node n) {
    return node{
      n.manager(), 
      n.handle().ExistAbstract(var.handle()).getNode()
    };
  }
   
  node exists(std::vector<variable> const& vars, node n) {
    node cube = n.manager()->top();
    for(auto v : vars)
      cube = cube && v;
    
    return node{
      n.manager(),
      n.handle().ExistAbstract(cube.handle()).getNode()
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
      n.handle().UnivAbstract(var.handle()).getNode()
    };
  }
   
  node forall(std::vector<variable> const& vars, node n) {
    node cube = n.manager()->top();
    for(auto v : vars)
      cube = cube && v;
    
    return node{
      n.manager(),
      n.handle().UnivAbstract(cube.handle()).getNode()
    };
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
    return node{
      n1.manager(),
      n1.handle().Xnor(n2.handle()).getNode()
    };
  }  

}