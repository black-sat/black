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

  class BLACK_EXPORT scope 
  {
  public:

    using enum rigid_t;

    scope(alphabet &sigma);
    
    scope(scope const&) = default;
    scope(scope &&) = default;

    scope &operator=(scope const&) = default;
    scope &operator=(scope &&) = default;

    bool operator==(scope const&) const = default;

    class std::optional<frame> frame() const { return _frame; }

    void push(std::vector<var_decl> decls, rigid_t = non_rigid);
    void push(std::vector<rel_decl> decls, rigid_t = non_rigid);
    void push(std::vector<fun_decl> decls, rigid_t = non_rigid);
    void push(std::vector<sort_decl> decls);
    
    void push(variable var, class sort s, rigid_t = non_rigid);
    void push(relation rel, std::vector<class sort> sorts, rigid_t = non_rigid);
    void push(relation rel, std::vector<var_decl> sorts, rigid_t = non_rigid);
    void push(
      function fun, class sort s, std::vector<class sort> sorts, 
      rigid_t = non_rigid
    );
    void push(
      function fun, class sort s, std::vector<var_decl> decls, 
      rigid_t = non_rigid
    );
    void push(named_sort s, domain_ref domain);

    void pop();

    std::optional<class sort> sort(variable) const;
    std::optional<class sort> sort(function) const;

    std::optional<std::vector<class sort>> signature(function) const;
    std::optional<std::vector<class sort>> signature(relation) const;

    class domain const*domain(class sort) const;
    
    std::optional<class sort>
    type_check(
      term<LTLPFO> t, std::optional<class sort> default_sort, 
      std::function<void(std::string)> err
    );

    bool type_check(
      formula<LTLPFO> f, std::optional<class sort> default_sort, 
      std::function<void(std::string)> err
    );

    bool is_rigid(variable) const;
    bool is_rigid(relation) const;
    bool is_rigid(function) const;

    size_t hash() const { 
      return std::hash<std::optional<class frame>>{}(_frame); 
    }

    friend std::string to_string(scope xi) {
      return "scope:" + to_string(xi._frame->unique_id());
    }

  private:
    alphabet *_sigma;
    std::optional<class frame> _frame;
  };

  class nest_scope_t 
  {
  public:
    nest_scope_t(scope &xi) : _old{xi}, _xi{xi} { }

    ~nest_scope_t() {
      while(_xi != _old)
        _xi.pop();
    }
    
  private:
    scope _old;
    scope &_xi;
  };
}

namespace std {
  template<>
  struct hash<::black_internal::logic::scope> {
    size_t operator()(::black_internal::logic::scope xi) const {
      return xi.hash();
    }
  };
}

#endif
