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

#ifndef BLACK_BDD_HPP
#define BLACK_BDD_HPP

#include <black/logic/logic.hpp>
#include <black/logic/renamings.hpp>

#include <ostream>

class BDD;
class Cudd;
struct DdNode;

namespace black::bdd {

  class variable;
  class node;

  class manager {
  public:
    manager(alphabet *sigma);
    manager(manager const&) = delete;
    manager(manager &&);
    ~manager();
    
    manager &operator=(manager const&) = delete;
    manager &operator=(manager &&);

    size_t varcount();

    class variable variable(proposition p);
    node top();
    node bottom();

    std::vector<class variable> variables();

    void minimize();

    alphabet *sigma() const;
    Cudd *handle() const;

    node to_node(black::logic::formula<black::logic::QBF>);

  private:
    friend class variable;
    friend class node;

    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

  class variable 
  {
  public:
    variable(class manager *, DdNode *);
    variable(variable const&);
    variable(variable &&);
    ~variable();
    
    variable &operator=(variable const&);
    variable &operator=(variable &&);
    
    bool operator==(variable const&) const = default;

    proposition name() const;
    class manager *manager() const;
    BDD handle() const;

    node operator!() const;

  private:
    friend class manager;
    variable(class manager *mgr, proposition name, DdNode * var);

    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

  struct element;

  class node {
  public:
    node(node const&);
    node(node &&);
    node(class manager *, DdNode *);
    ~node();

    node(variable lit);
    
    node &operator=(node const&);
    node &operator=(node &&);

    class manager *manager() const;
    BDD handle() const;

    size_t hash() const;

    size_t count() const;

    std::vector<variable> variables() const;

    bool operator==(node const&other) const = default;

    bool is_one() const;
    bool is_zero() const;
    
    node condition(variable lit, bool sign) const;
    node condition(std::vector<class variable> const& lits, bool sign) const;
    node condition(
      std::vector<black::proposition> const& props, bool sign
    ) const;

    node swapvars(std::function<black::proposition(black::proposition)> map);
    node operator[](std::function<black::proposition(black::proposition)> map) {
      return swapvars(map);
    }

  private:
    friend class manager;
    
    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

  node operator!(node n);
  node operator&&(node n1, node n2);
  node operator||(node n1, node n2);
  
  node exists(variable var, node n);
  node forall(variable var, node n);
  node exists(std::vector<variable> const& vars, node n);
  node forall(std::vector<variable> const& vars, node n);
  node exists(std::vector<black::proposition> const& vars, node n);
  node forall(std::vector<black::proposition> const& vars, node n);

  template<black_internal::filter M>
  node exists(M const& m, node n) {
    auto node_vars = n.variables();
    auto it = std::remove_if(begin(node_vars), end(node_vars), [&](auto v) {
      return !m.filter(v.name());
    });
    node_vars.erase(it, end(node_vars));
    
    return exists(node_vars, n);
  }

  template<black_internal::filter M>
  node forall(M const& m, node n) {
    auto node_vars = n.variables();
    auto it = std::remove_if(begin(node_vars), end(node_vars), [&](auto v) {
      return !m.filter(v.name());
    });
    node_vars.erase(it, end(node_vars));
    
    return forall(node_vars, n);
  }

  node implies(node n1, node n2);
  node iff(node n1, node n2);

  template<typename T>
  concept node_like = requires(T v) {
    { to_node(v) } -> std::convertible_to<node>;
  };

  template<node_like T1, node_like T2>
  node operator&&(T1 v1, T2 v2) {
    return to_node(v1) && to_node(v2);
  }
  
  template<node_like T1, node_like T2>
  node operator||(T1 v1, T2 v2) {
    return to_node(v1) || to_node(v2);
  }

  template<node_like T1, node_like T2>
  node implies(T1 v1, T2 v2) {
    return implies(to_node(v1), to_node(v2));
  }
  
  template<node_like T1, node_like T2>
  node iff(T1 v1, T2 v2) {
    return iff(to_node(v1), to_node(v2));
  }

  inline bdd::variable prime(bdd::variable var, size_t n) {
    return var.manager()->variable(black_internal::prime(var.name(), n));
  }

  inline bdd::variable prime(bdd::variable var) {
    return var.manager()->variable(black_internal::prime(var.name()));
  }

  template<typename R, typename F>
  bdd::node big_and(bdd::manager *mgr, R const& r, F f) {
    bdd::node acc = mgr->top();
    for(auto v : r)
      acc = acc && f(v);
    return acc;
  }

  template<typename R, typename F>
  bdd::node big_or(bdd::manager *mgr, R const& r, F f) {
    bdd::node acc = mgr->bottom();
    for(auto v : r)
      acc = acc || f(v);
    return acc;
  }

}

template<>
struct std::hash<black::bdd::variable> {
  size_t operator()(black::bdd::variable var) const {
    return std::hash<black::proposition>{}(var.name());
  }
};

template<>
struct std::hash<black::bdd::node> {
  size_t operator()(black::bdd::node node) const {
    return node.hash();
  }
};

#endif // BLACK_BDD_HPP