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

#ifndef BLACK_SDD_HPP
#define BLACK_SDD_HPP

#include <black/logic/logic.hpp>
#include <black/logic/renamings.hpp>

struct sdd_node_t;
struct sdd_manager_t;
struct wmc_manager_t;

namespace black::sdd {

  class variable;
  class literal;
  class node;

  class manager {
  public:
    manager(alphabet *sigma);
    manager(manager const&) = delete;
    manager(manager &&);
    ~manager();
    
    manager &operator=(manager const&) = delete;
    manager &operator=(manager &&);

    class variable variable(proposition p);
    std::vector<class variable> variables(std::vector<proposition> props);
    node top();
    node bottom();

    alphabet *sigma() const;
    sdd_manager_t *handle() const;

    node to_node(black::logic::formula<black::logic::QBF>);
    logic::formula<logic::propositional> to_formula(node);

  private:
    friend class variable;
    friend class literal;
    friend class node;

    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

  class variable 
  {
  public:
    variable(class manager *, unsigned);
    variable(variable const&) = default;
    variable(variable &&) = default;
    
    variable &operator=(variable const&) = default;
    variable &operator=(variable &&) = default;
    
    bool operator==(variable const&) const = default;

    proposition name() const { return _name; }
    class manager *manager() const { return _mgr; }

    literal operator!() const;

    unsigned handle() const { return _var; }

  private:
    friend class manager;

    variable(class manager *mgr, proposition name, unsigned var) 
      : _mgr{mgr}, _name{name}, _var{var} { }

    class manager *_mgr;
    proposition _name;
    unsigned _var;
  };

  class literal 
  {
  public:
    literal(class variable var, bool sign = true) : _var{var}, _sign{sign} { }
    literal(class manager *, long lit);

    class variable variable() const { return _var; }
    proposition name() const { return _var.name(); }
    bool sign() const { return _sign; }
    class manager *manager() const { return _var.manager(); }

    literal operator!() const {
      return literal{_var, !_sign};
    }

    long handle() const {
      return _sign ? long(_var.handle()) : -long(_var.handle());
    }

  private:
    class variable _var;
    bool _sign = true;
  };

  inline literal variable::operator!() const {
    return literal{*this, false};
  }

  struct element;

  class node {
  public:
    node(node const&) = default;
    node(node &&) = default;
    node(class manager *, sdd_node_t *);
    
    node &operator=(node const&) = default;
    node &operator=(node &&) = default;

    class manager *manager() const { return _mgr; }
    sdd_node_t *handle() const { return _node.get(); }

    std::vector<variable> variables() const;

    bool operator==(node const&other) const = default;

    size_t count() const;

    bool is_valid() const;
    bool is_unsat() const;
    bool is_sat() const;
    
    bool is_literal() const;
    bool is_decision() const;

    std::optional<class literal> literal() const;
    std::vector<element> elements() const;

    node rename(std::function<sdd::variable(sdd::variable)> renaming);
    node operator[](std::function<black::proposition(black::proposition)> map) {
      return rename([&](sdd::variable var) {
        return manager()->variable(map(var.name()));
      });
    }

  private:
    friend class manager;
    
    class manager *_mgr;
    std::shared_ptr<sdd_node_t> _node;
  };

  struct element {
    node prime;
    node sub;
  };

  inline node to_node(node n) { return n; }
  node to_node(literal);
  node to_node(variable);

  node operator!(node n);
  node operator&&(node n1, node n2);
  node operator||(node n1, node n2);
  
  node exists(variable var, node n);
  node forall(variable var, node n);
  node exists(std::vector<variable> const& vars, node n);
  node forall(std::vector<variable> const& vars, node n);

  template<black_internal::matcher M>
  node exists(M const& m, node n) {
    auto node_vars = n.variables();
    auto it = std::remove_if(begin(node_vars), end(node_vars), [&](auto v) {
      return !m.match(v.name());
    });
    node_vars.erase(it, end(node_vars));
    
    return exists(node_vars, n);
  }

  template<black_internal::matcher M>
  node forall(M const& m, node n) {
    return !exists(m, !n);
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

  inline sdd::variable prime(sdd::variable var, size_t n) {
    return var.manager()->variable(black_internal::prime(var.name(), n));
  }

  inline sdd::variable step(sdd::variable var, size_t n) {
    return var.manager()->variable(black_internal::step(var.name(), n));
  }

};

template<>
struct std::hash<black::sdd::variable> {
  size_t operator()(black::sdd::variable var) const {
    return std::hash<black::proposition>{}(var.name());
  }
};

template<>
struct std::hash<black::sdd::literal> {
  size_t operator()(black::sdd::literal lit) const {
    return std::hash<black::proposition>{}(lit.name());
  }
};

template<>
struct std::hash<black::sdd::node> {
  size_t operator()(black::sdd::node node) const {
    return std::hash<sdd_node_t *>{}(node.handle());
  }
};

#endif // BLACK_SDD_HPP
