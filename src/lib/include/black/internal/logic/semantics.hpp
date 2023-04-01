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

#ifndef BLACK_LOGIC_SEMANTICS_HPP
#define BLACK_LOGIC_SEMANTICS_HPP

#include <memory>
#include <vector>

//
// This file contains code that deals with the semantic aspects of formulas and
// terms. These include type checking and other semantic checks to be done on
// formulas to ensure their correctness before being given to solvers or
// printers.
//
namespace black_internal::logic {
  
  //
  // The `domain` class represents a finite domain associated with a named sort.
  // Named sorts can be either infinite and uninterpreted, in which case they
  // have no associated domain, or finite and enumerated, in which case they are
  // associated with a `domain` object by the `scope` class below. 
  //
  class BLACK_EXPORT domain : public std::enable_shared_from_this<domain>
  {
  public:
    domain(std::vector<variable> elements) 
      : _elements{std::move(elements)}
      { }

    domain(domain const&) = delete;
    domain(domain &&) = delete;
    
    domain &operator=(domain const&) = delete;
    domain &operator=(domain &&) = delete;

    std::vector<variable> const& elements() const { return _elements; }

  private:
    std::vector<variable> _elements;
  };

  // this `using` is forward declared in `fragments.hpp`.
  // using domain_ref = std::unique_ptr<domain>;

  inline domain_ref make_domain(std::vector<variable> elements) {
    return std::make_shared<domain>(std::move(elements));
  }

  //
  // The `scope` class handles all the semantic information needed to manage a
  // formula. A `formula` object only describes the syntax of the formula, but
  // to interpret it often semantics information are needed (for first-order
  // ones, not so much for propositional LTL), such as the sort of variables and
  // the signatures/arities of functions and relations. The `scope` class
  // handles these information.
  //
  // Crucially, scopes can be chained to support nested scopes. This is done as
  // follows:
  // 
  // scope s1 = ...;
  // scope s2 = chain(s1);
  //

  class BLACK_EXPORT scope 
  {
  public:
    struct chain_t {
      scope const& s;
    };

    enum rigid_t : bool {
      non_rigid = 0,
      rigid = 1
    };

    scope(alphabet &sigma, std::optional<class sort> def = std::nullopt);
    scope(chain_t);
    scope(scope const&) = delete;
    scope(scope &&);
    ~scope();

    scope &operator=(scope const&) = delete;
    scope &operator=(scope &&);

    void set_default_sort(std::optional<class sort> s);
    std::optional<class sort> default_sort() const;

    void declare(variable, class sort, rigid_t = non_rigid);
    void declare(
      relation, std::vector<class sort>, rigid_t = non_rigid
    );
    void declare(
      function, class sort, std::vector<class sort>, rigid_t = non_rigid
    );
    void declare(
      function, std::vector<class sort>, rigid_t = non_rigid
    );

    void declare(
      relation rel, std::vector<var_decl> decls, rigid_t r = non_rigid
    ) {
      std::vector<class sort> sorts;
      for(auto decl : decls)
        sorts.push_back(decl.sort());
      declare(rel, sorts, r);
    }
    void declare(
      function fun, class sort s, std::vector<var_decl> decls, 
      rigid_t r = non_rigid
    ) {
      std::vector<class sort> sorts;
      for(auto decl : decls)
        sorts.push_back(decl.sort());
      declare(fun, s, sorts, r);
    }

    void declare(named_sort s, domain_ref d);
    
    
    void declare(var_decl d, rigid_t r = non_rigid) {
      declare(d.variable(), d.sort(), r);
    }
    void declare(fun_decl d, rigid_t r = non_rigid) {
      declare(d.function(), d.sort(), d.signature(), r);
    }
    void declare(rel_decl d, rigid_t r = non_rigid) {
      declare(d.relation(), d.signature(), r);
    }
    void declare(sort_decl d) {
      declare(d.sort(), d.domain());
    }

    std::optional<class sort> sort(variable) const;
    std::optional<class sort> sort(function) const;

    std::optional<std::vector<class sort>> signature(function) const;
    std::optional<std::vector<class sort>> signature(relation) const;

    class domain const*domain(class sort) const;
    
    std::optional<class sort>
    type_check(term<LTLPFO> t, std::function<void(std::string)> err);

    bool type_check(formula<LTLPFO> f, std::function<void(std::string)> err);

    bool is_rigid(variable) const;
    bool is_rigid(relation) const;
    bool is_rigid(function) const;

    template<typename T, typename Key>
    void set_data(Key k, T&& t) {
      set_data_inner(k, std::any{std::forward<T>(t)});
    }

    template<typename T, typename Key>
    std::optional<T> data(Key k) const {
      std::any a = data_inner(k);

      if(T *t = std::any_cast<T>(&a); t != nullptr)
        return *t;

      return {};
    }


  private:
    void set_data_inner(proposition, std::any);
    void set_data_inner(variable, std::any);
    void set_data_inner(relation, std::any);
    void set_data_inner(function, std::any);
    void set_data_inner(class sort, std::any);
    std::any data_inner(proposition) const;
    std::any data_inner(variable) const;
    std::any data_inner(relation) const;
    std::any data_inner(function) const;
    std::any data_inner(class sort) const;
    
    struct impl_t;
    std::unique_ptr<impl_t> _impl;
  };

  inline scope::chain_t chain(scope const&s) {
    return {s};
  }

  class nest_scope_t 
  {
  public:
    nest_scope_t(scope &xi) : _ref{xi}, _old{std::move(xi)} {
      _ref = chain(_old);
    }

    ~nest_scope_t() {
      _ref = std::move(_old);
    }
    
  private:
    scope &_ref;
    scope _old;
  };
}

#endif
