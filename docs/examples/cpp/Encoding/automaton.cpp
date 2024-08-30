//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <black/support>
#include <black/logic>
#include <black/ast/algorithms>
#include <black/pipes>

namespace black::pipes::internal {

  using namespace support;
  using namespace logic;
  using namespace ast;

  struct automaton_t::impl_t : public consumer
  {
    impl_t(class consumer *next) : _next{next}{ }

    class consumer *_next;

    std::optional<object> translate(object x);
    term undo(term x);

    virtual void import(logic::module) override;
    virtual void adopt(std::shared_ptr<logic::root const>) override;
    virtual void state(logic::term t, logic::statement s) override;
    virtual void push() override;
    virtual void pop(size_t) override;

    private:
    term to_snf(term t);
    std::vector<variable>::iterator find_var(std::vector<variable> &, variable);
    std::vector<variable> vec_union(std::vector<variable>, std::vector<variable>);
    std::vector<variable> quantify(std::vector<decl>, std::vector<variable>);
    types::type type_from_module(module, variable);
    module surrogates(term);
    std::vector<variable> surrogates(module &, module &, term, bool &);

  };

  automaton_t::automaton_t( class consumer *next ) : _impl{std::make_unique<impl_t>(next)} { }

  automaton_t::~automaton_t() = default;

  consumer *automaton_t::consumer() { return _impl.get(); }

  std::optional<object> automaton_t::translate(object x) { 
    return _impl->translate(x); 
  }
  
  term automaton_t::undo(term x) { return _impl->undo(x); }

  void automaton_t::impl_t::import(logic::module m) {
    _next->import(std::move(m));
  }

  void automaton_t::impl_t::adopt(std::shared_ptr<logic::root const>) { }

  void automaton_t::impl_t::state(logic::term t, logic::statement s) {
    if(s == logic::statement::requirement) {
      module mod = surrogates(to_snf(t));
      _next->adopt(mod.resolve());      
    }
  }

  void automaton_t::impl_t::push() { }

  void automaton_t::impl_t::pop(size_t n) { _next->pop(n); }

  std::optional<object> automaton_t::impl_t::translate(object) { return {}; }

  term automaton_t::impl_t::undo(term t) { return t; }
     
  term automaton_t::impl_t::to_snf(term t) {
    return match(t)(
      [](object obj)   { return obj; },
      [](variable var) { return var; },

      /*
        Boolean and first-order predicates.
      */
      [](equal e)     { return e; },
      [](distinct d)  { return d; },
      [](atom a)      { return a; },
      [&](exists, std::vector<decl> decls, term body) { return exists(decls, to_snf(body)); },
      [&](forall, std::vector<decl> decls, term body) { return forall(decls, to_snf(body)); },

      /*
        Boolean connectives.
      */
      [&](negation, term argument)                  { return !to_snf(argument); },
      [&](conjunction, std::vector<term> arguments) { return to_snf(arguments[0]) && to_snf(arguments[1]); },
      [&](disjunction, std::vector<term> arguments) { return to_snf(arguments[0]) || to_snf(arguments[1]); },
      [&](implication, std::vector<term> arguments) { return implication(to_snf(arguments[0]), to_snf(arguments[1])); },
      
      /*
        Future LTL operators.
      */
      [&](tomorrow t) { return t; },
      [&](w_tomorrow wt) { return wt; },
      [&](eventually f, term argument)       { return to_snf(argument) || X(f); },
      [&](always a, term argument)           { return to_snf(argument) && wX(a); },
      [&](until u, term left, term right)    { return to_snf(right) || (to_snf(left) && X(u)); },
      [&](release r, term left, term right)  { return to_snf(right) && (to_snf(left) || wX(r)); },

      /*
        Past LTL operators.
      */
      [](yesterday y)            { return y; },
      [](w_yesterday z)          { return z; },
      [&](once o, term argument)                 { return to_snf(argument) || Y(o); },
      [&](historically h, term argument)         { return to_snf(argument) && Z(h); },
      [&](since s, term left, term right)        { return to_snf(right) || (to_snf(left) && Y(s)); },
      [&](triggered t, term left, term right)    { return to_snf(right) && (to_snf(left) && Z(t)); },

      /* 
        Arithmetic operators.
      */
      [](minus m)         { return m; },
      [](sum s)           { return s; },
      [](product p)       { return p; },
      [](difference diff) { return diff; },
      [](division div)    { return div; },
      
      /*
        Relational comparisons.
      */
      [](less_than lt)            { return lt; },
      [](less_than_eq lte)        { return lte;  },
      [](greater_than gt)         { return gt; },
      [](greater_than_eq gte)     { return gte;  }
    );
  }

  /*
    Returns an iterator pointing to the first occurrence of target in vec.
  */
  std::vector<variable>::iterator automaton_t::impl_t::find_var(std::vector<variable> &vec, variable target) {
    return std::find_if(vec.begin(), vec.end(), [target](variable var) {
        return term_equal(target, var);
    });
  }

  std::vector<variable> automaton_t::impl_t::vec_union(std::vector<variable> vec1, std::vector<variable> vec2) {
    for(variable var2: vec2) {
        if(find_var(vec1, var2) == vec1.end()) {
            vec1.push_back(var2);
        }
    }
    return vec1;
  }

  std::vector<variable> automaton_t::impl_t::quantify(std::vector<decl> q_vars, std::vector<variable> vars) {
    for (decl d : q_vars) {
        vars.erase(find_var(vars, d.name));
    }
    return vars;
  }

  types::type automaton_t::impl_t::type_from_module(module mod, variable var) {
    return mod.lookup(var).value().entity()->type;
  }

  module automaton_t::impl_t::surrogates(term t){
    module gamma, aux;
    object x_phi = gamma.declare({"X_PHI", types::boolean()}, resolution::delayed);
    
    _next->state(t == x_phi,    logic::statement::transition);

    bool future = false;

    std::vector<variable> free = surrogates(gamma, aux, t, future);
    
    if(future) {
      _next->state(x_phi,         logic::statement::init);
      _next->state(!x_phi,        logic::statement::final);
    }
    else{
      _next->state(x_phi,         logic::statement::final);
      _next->state(!x_phi,        logic::statement::init);
    }
    return gamma;
  }
  
  std::vector<variable> automaton_t::impl_t::surrogates(module &gamma, module &aux, term t, bool &future) {
    return match(t)(
      /*
        Base cases.
      */
      [](variable var) -> std::vector<variable> { return { var }; },
      [](object)       -> std::vector<variable> { return { }; },

      /*
        Quantifiers. All quantified variables are stored (declared) in a new module aux2. The recursive relies on aux2 to retrieve the types
        of all free variables in term body.
      */
      [&](exists, std::vector<decl> decls, term body) -> std::vector<variable> {
        module aux2 = aux;
        for(decl d : decls) {
            aux2.declare(d, resolution::immediate);
        }
        
        return quantify(decls, surrogates(gamma, aux2, body, future));
      },
      [&](forall, std::vector<decl> decls, term body) -> std::vector<variable> {
        module aux2 = aux;
        for(decl d : decls) {
            aux2.declare(d, resolution::immediate);
        }

        return quantify(decls, surrogates(gamma, aux2, body, future));
      },

      /*
          Temporal operators.
              (i) Free variables in term body are retrieved;
              (ii) The surrogate is defined accordingly;
              (iii) The conjuncts containing the surrogate are passed to the next stage in the pipeline.
      */
      [&](tomorrow, term body) -> std::vector<variable> {
        future = true;

        // (i)
        std::vector<variable> free_vars = surrogates(gamma, aux, body, future);
        std::vector<term> free_terms = { };
        std::vector<decl> decls = { };

        // Retrieve free variable types.
        std::vector<types::type> types = { };
        for(variable var : free_vars){
            types::type t = type_from_module(aux, var);
            decls.push_back({var, t});
            types.push_back(t);
            free_terms.push_back(var);
        }

        // (ii)
        object surr = gamma.declare(
            {"XS", types::function(types, types::boolean())},
            resolution::delayed
        );
        term a = atom(surr, free_terms);

        // (iii)
        _next->state(forall(decls, body == a),   logic::statement::transition);
        _next->state(forall(decls, !a),         logic::statement::final);

        return free_vars;
      },
      [&](w_tomorrow, term body) -> std::vector<variable> {
        future = true;
        // (i)
        std::vector<variable> free_vars = surrogates(gamma, aux, body, future);
        std::vector<term> free_terms = { };
        std::vector<decl> decls = { };

        // Retrieve free variable types.
        std::vector<types::type> types = { };
        for(variable var : free_vars){
            types::type t = type_from_module(aux, var);
            decls.push_back({var, t});
            types.push_back(t);
            free_terms.push_back(var);
        }

        // (ii)
        object surr = gamma.declare(
            {"XW", types::function(types, types::boolean())},
            resolution::delayed
        );
        term a = atom(surr, free_terms);

        // (iii)
        _next->state(forall(decls, body == a),   logic::statement::transition);
        _next->state(forall(decls, a),         logic::statement::final);

        return free_vars;
      },

      [&](yesterday, term body) -> std::vector<variable> {
          
        // (i)
        std::vector<variable> free_vars = surrogates(gamma, aux, body, future);
        std::vector<term> free_terms = { };
        std::vector<decl> decls = { };

        // Retrieve free variable types.
        std::vector<types::type> types = { };
        for(variable var : free_vars){
            types::type t = type_from_module(aux, var);
            decls.push_back({var, t});
            types.push_back(t);
            free_terms.push_back(var);
        }

        // (ii)
        object surr = gamma.declare(
            {"XY", types::function(types, types::boolean())},
            resolution::delayed
        );
        term a = atom(surr, free_terms);

        // (iii)
        _next->state(forall(decls, body == a),   logic::statement::transition);
        _next->state(forall(decls, !a),         logic::statement::init);

        return free_vars;
      },

      [&](w_yesterday, term body) -> std::vector<variable> {
        // (i)
        std::vector<variable> free_vars = surrogates(gamma, aux, body, future);
        std::vector<term> free_terms = { };
        std::vector<decl> decls = { };

        // Retrieve free variable types.
        std::vector<types::type> types = { };
        for(variable var : free_vars){
            types::type t = type_from_module(aux, var);
            decls.push_back({var, t});
            types.push_back(t);
            free_terms.push_back(var);
        }

        // (ii)
        object surr = gamma.declare(
            {"XY", types::function(types, types::boolean())},
            resolution::delayed
        );
        term a = atom(surr, free_terms);

        // (iii)
        _next->state(forall(decls, body == a),   logic::statement::transition);
        _next->state(forall(decls, a),         logic::statement::init);

        return free_vars;
      },

      /* 
          Boolean and first-order predicates
      */
      [&](equal, std::vector<term> arguments) -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], future), surrogates(gamma, aux, arguments[1], future)); },
      [&](distinct, std::vector<term> arguments) -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], future), surrogates(gamma, aux, arguments[1], future)); },
      [&](atom, std::vector<term> arguments) -> std::vector<variable> { 
        std::vector<variable> result = { };
        for(term t : arguments) {
            result = vec_union(result, surrogates(gamma, aux, t, future));
        }
        return result;
      },

      /*
          Boolean connectives.
      */
      [&](negation, term argument)                    -> std::vector<variable> { return surrogates(gamma, aux, argument, future); },
      [&](conjunction, std::vector<term> arguments)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], future), surrogates(gamma, aux, arguments[1], future)); },
      [&](disjunction, std::vector<term> arguments)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, arguments[0], future), surrogates(gamma, aux, arguments[1], future)); },
      [&](implication, term left, term right)         -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },

      /*
          Other temporal operators.
      */
      [&](eventually, term argument)          -> std::vector<variable> { return surrogates(gamma, aux, argument, future); },
      [&](always, term argument)              -> std::vector<variable> { return surrogates(gamma, aux, argument, future); },
      [&](until, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](release, term left, term right)     -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](once, term argument)                -> std::vector<variable> { return surrogates(gamma, aux, argument, future); },
      [&](historically, term argument)        -> std::vector<variable> { return surrogates(gamma, aux, argument, future); },
      [&](since, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](triggered, term left, term right)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },

      /* 
          Arithmetic operators.
      */
      [&](minus, term argument)                -> std::vector<variable> { return surrogates(gamma, aux, argument, future); },
      [&](sum, term left, term right)          -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](product, term left, term right)      -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](difference, term left, term right)   -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](division, term left, term right)     -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },

      /*
          Relational comparisons.
      */
      [&](less_than, term left, term right)          -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](less_than_eq, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](greater_than, term left, term right)       -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); },
      [&](greater_than_eq, term left, term right)    -> std::vector<variable> { return vec_union(surrogates(gamma, aux, left, future), surrogates(gamma, aux, right, future)); }
    );
  }
}