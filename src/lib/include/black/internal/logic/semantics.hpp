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

//
// This file contains code that deals with the semantic aspects of formulas and
// terms. These include type checking and other semantic checks to be done on
// formulas to ensure their correctness before being given to solvers or
// printers.
//
namespace black_internal::logic {
  
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

    scope(alphabet &sigma, std::optional<struct sort> def = std::nullopt);
    scope(chain_t);
    scope(scope const&);
    scope(scope &&);
    ~scope();

    scope &operator=(scope const&);
    scope &operator=(scope &&);

    void set_default_sort(std::optional<struct sort> s);
    std::optional<struct sort> default_sort() const;

    void declare(variable, struct sort, rigid_t = non_rigid);
    void declare(
      relation, std::vector<struct sort>, rigid_t = non_rigid
    );
    void declare(
      function, struct sort, std::vector<struct sort>, rigid_t = non_rigid
    );

    void declare(
      function, std::vector<struct sort>, rigid_t = non_rigid
    );

    void declare(var_decl d, rigid_t r = non_rigid) {
      declare(d.variable(), d.sort(), r);
    }

    std::optional<struct sort> sort(variable) const;
    std::optional<struct sort> sort(function) const;

    std::optional<std::vector<struct sort>> signature(function) const;
    std::optional<std::vector<struct sort>> signature(relation) const;
    
    std::optional<struct sort>
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
    void set_data_inner(variable, std::any);
    void set_data_inner(relation, std::any);
    void set_data_inner(function, std::any);
    std::any data_inner(variable) const;
    std::any data_inner(relation) const;
    std::any data_inner(function) const;
    
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