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

#include <black/bdd/bdd.hpp>

#include <black/bdd/cudd/cuddObj.hh>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

#include <cstdlib>
#include <random>
#include <iostream>
#include <random>

namespace black::bdd {

  using cudd_ptr = std::unique_ptr<DdNode, std::function<void(DdNode *)>>;

  //
  // manager
  //
  struct manager::impl_t {
    impl_t(alphabet *_sigma) : sigma{_sigma} { }

    alphabet *sigma;
    Cudd mgr;

    tsl::hopscotch_map<proposition, DdNode *> prop_to_var;
    tsl::hopscotch_map<DdNode *, proposition> var_to_prop;
    tsl::hopscotch_map<DdNode *, int> var_to_index;
    std::vector<cudd_ptr> index_to_var;

    int next_var = 0;
    tsl::hopscotch_map<
      black::logic::formula<black::logic::QBF>, 
      node
    > to_node_cache;
  };

  manager::manager(alphabet *sigma) 
    : _impl{std::make_unique<impl_t>(sigma)} 
  { 
    // Best so far:
    // - CUDD_REORDER_WINDOW2
    // - CUDD_REORDER_WINDOW3
    // - CUDD_REORDER_WINDOW2_CONV
    _impl->mgr.AutodynEnable(CUDD_REORDER_WINDOW2_CONV);
  }

  manager::manager(manager &&) = default;
  manager &manager::operator=(manager &&) = default;
  manager::~manager() = default;

  size_t manager::varcount() {
    return _impl->index_to_var.size();
  }

  variable manager::variable(proposition name) {
    if(_impl->prop_to_var.contains(name))
      return bdd::variable{this, name, _impl->prop_to_var.at(name)};

    int index = _impl->next_var++;
    DdNode * var = _impl->mgr.bddVar(index).getNode();
    Cudd_Ref(var);
    DdManager *manager = handle()->getManager();
    cudd_ptr var_ptr = cudd_ptr{var, [=](DdNode *n) {
      Cudd_RecursiveDeref(manager, n);
    }};
    
    _impl->prop_to_var.insert({name, var});
    _impl->var_to_prop.insert({var, name});
    _impl->var_to_index.insert({var, index});
    _impl->index_to_var.push_back(std::move(var_ptr));

    return bdd::variable{this, name, var};
  }

  node manager::top() {
    return node{this, _impl->mgr.bddOne().getNode()};
  }

  node manager::bottom() {
    return node{this, _impl->mgr.bddZero().getNode()};
  }

  void manager::minimize() {
    _impl->mgr.ReduceHeap(CUDD_REORDER_WINDOW2, 3000);
  }

  std::vector<class variable> manager::variables() {
    std::vector<class variable> result;
    for(auto &ddnode : _impl->index_to_var)
      result.push_back({this, ddnode.get()});
    return result;
  }

  alphabet *manager::sigma() const {
    return _impl->sigma;
  }

  Cudd *manager::handle() const {
    return &_impl->mgr;
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
        return bdd::node(variable(p));
      },
      [&](logic::negation<QBF>, auto arg) {
        return !to_node(arg);
      },
      [&](logic::conjunction<QBF> c) {
        bdd::node acc = top();
        for(auto op : c.operands())
          acc = acc && to_node(op);
        return acc;
      },
      [&](logic::disjunction<QBF> c) {
        bdd::node acc = bottom();
        for(auto op : c.operands())
          acc = acc || to_node(op);
        return acc;
      },
      [&](logic::implication<QBF>, auto left, auto right) {
        return bdd::implies(to_node(left), to_node(right));
      },
      [&](logic::iff<QBF>, auto left, auto right) {
        return bdd::iff(to_node(left), to_node(right));
      },
      [&](logic::qbf<QBF> q, auto qvars, auto matrix) {
        bdd::node bddmatrix = to_node(matrix);
        if(qvars.empty())
          return bddmatrix;
        
        std::vector<bdd::variable> vars;
        for(auto qvar : qvars)
          vars.push_back(variable(qvar));

        return q.node_type().match(
          [&](logic::qbf<QBF>::type::thereis) {
            return bdd::exists(vars, bddmatrix);
          },
          [&](logic::qbf<QBF>::type::foreach) {
            return bdd::forall(vars, bddmatrix);
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
    BDD _var;
  };

  variable::variable(class manager *mgr, proposition name, DdNode * var) 
      : _impl{std::make_unique<impl_t>(mgr, name, BDD(*mgr->handle(), var))} { }

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
    return _impl->_var;
  }

  node variable::operator!() const {
    return node{ 
      manager(), 
      (!handle()).getNode()
    };
  }

  //
  // node
  //
  struct node::impl_t {
    class manager *_mgr;
    BDD _node;
  };

  node::node(class manager *mgr, DdNode * n) 
    : _impl{std::make_unique<impl_t>(mgr, BDD(*mgr->handle(), n))} { }  

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
    return _impl->_node; 
  }

  size_t node::hash() const {
    return std::hash<DdNode *>{}(handle().getNode());
  }

  size_t node::count() const {
    return size_t(manager()->handle()->nodeCount({handle()}));
  }

  std::vector<variable> node::variables() const {
    tsl::hopscotch_set<variable> vars;
    std::vector<unsigned int> support = handle().SupportIndices();
    for(auto i : support)
      vars.insert(variable{
        manager(),
        manager()->_impl->index_to_var.at(i).get()
      });
    
    return std::vector<variable>(begin(vars), end(vars));
  }

  bool node::is_one() const {
    return handle().IsOne();
  }

  bool node::is_zero() const {
    return handle().IsZero();
  }

  node node::condition(variable var, bool sign) const {
    BDD literal = sign ? var.handle() : (!var).handle();
    return node{
      manager(),
      handle().Cofactor(literal).getNode()
    };
  }

  node node::condition(std::vector<class variable> const& v, bool sign) const 
  {
    node result = *this;
    for(auto var : v)
      result = result.condition(var, sign);
    
    return result;
  }

  node node::condition(
    std::vector<black::proposition> const& props, bool sign
  ) const {
    std::vector<variable> vars;
    for(auto p : props)
      vars.push_back(manager()->variable(p));
    return condition(vars, sign);
  }

  node node::swapvars(std::function<black::proposition(black::proposition)> map)
  {
    std::vector<BDD> vars;
    std::vector<BDD> mapped;

    for(auto v : variables()) {
      vars.push_back(v.handle());
      mapped.push_back(manager()->variable(map(v.name())).handle());
    }

    return node {
      manager(),
      handle().SwapVariables(vars, mapped).getNode()
    };
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
    std::vector<bdd::variable> bddvars;
    for(auto v : vars)
      bddvars.push_back(n.manager()->variable(v));
    return exists(bddvars, n);
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
    std::vector<bdd::variable> bddvars;
    for(auto v : vars)
      bddvars.push_back(n.manager()->variable(v));
    return forall(bddvars, n);
  }

  node implies(node n1, node n2) {
    return !n1 || n2;
  }

  node iff(node n1, node n2) {
    return implies(n1, n2) && implies(n2, n1);
  }  

}