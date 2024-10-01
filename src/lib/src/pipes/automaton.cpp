//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante, Alex Della Schiava
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
#include <black/support/private>
#include <black/logic>
#include <black/ast/algorithms>
#include <black/pipes>

namespace black::pipes::internal {

  using namespace support;
  using namespace logic;
  using namespace ast;

  enum next_ops {
    next,
    w_next,
    no_next
  };

  /*
    Parameters:
      - term SNFSed: Is a, assumingly, term in primed SNFS form, that is, SNFS in SNF and all (weak) tomorrow and (weak) yesterday
                    operators have been surrogated and primed;
      - std::vector<variable> free_vars: a vector containing all the free variables occurring in SNFSed in order from left to right;
      - next_ops next_control: Let body be the argument of SNFSed. Then next_control is equal to:
              - next if body is a term of the form X(obj) where obj is an object term;
              - w_next if body is a term of the form wX(obj) where obj is an object term;
              - no_next otherwise.
  */
  struct SNFSer
  {
    term SNFSed;
    std::vector<variable> free_vars;
    next_ops next_control;
  };

  struct  automaton_t::impl_t : public consumer
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

    template <typename T>
    std::vector<T>::iterator find_var(std::vector<T> &, T);
    template <typename T>
    std::vector<T> vec_union(std::vector<T>, std::vector<T>);
    std::vector<variable> quantify(std::vector<decl>, std::vector<variable>);
    types::type type_from_module(module, variable);
    module surrogates(term);
    SNFSer to_SNFS(module &, module &, term, bool &, int &, term, bool &, bool);

  };

  automaton_t:: automaton_t( class consumer *next ) : _impl{std::make_unique<impl_t>(next)} {
  }

  automaton_t::~ automaton_t() = default;

  consumer * automaton_t::consumer() { return _impl.get(); }

  std::optional<object>  automaton_t::translate(object x) { 
    return _impl->translate(x); 
  }
  
  term  automaton_t::undo(term x) { return _impl->undo(x); }

  void  automaton_t::impl_t::import(logic::module m) {
    _next->import(std::move(m));
  }

  void  automaton_t::impl_t::adopt(std::shared_ptr<logic::root const>) { }

  void  automaton_t::impl_t::state(logic::term t, logic::statement s) {
    if(s == logic::statement::requirement) {
      module mod = surrogates(t);
      _next->adopt(mod.resolve());      
    }
  }

  void  automaton_t::impl_t::push() { }
  
  void  automaton_t::impl_t::pop(size_t n) { _next->pop(n); }
  
  std::optional<object>  automaton_t::impl_t::translate(object) { return {}; }
  
  term automaton_t::impl_t::undo(term t) { return t; }

  /*
    Returns an iterator pointing to the first occurrence of target in vec.
  */
  template <typename T>
  std::vector<T>::iterator automaton_t::impl_t::find_var(std::vector<T> &vec, T target) {
    return std::find_if(vec.begin(), vec.end(), [target](T var) {
        return term_equal(target, var);
    });
  }

  template <typename T>
  std::vector<T> automaton_t::impl_t::vec_union(std::vector<T> vec1, std::vector<T> vec2) {
    for(T var2: vec2) {
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
    
    bool future = false;
    bool is_next = false;
    int surrID = 0;

    object last = gamma.declare({"last", types::boolean()}, resolution::delayed);
    SNFSer phi = to_SNFS(gamma, aux, t, future, surrID, last, is_next, false);
    
    if(future) {
      object x_phi = gamma.declare({"xs_phi", types::boolean()}, resolution::delayed);

      _next->state(x_phi,                       logic::statement::init);
      _next->state(x_phi == primed(phi.SNFSed), logic::statement::transition);
      _next->state(!x_phi,                      logic::statement::final);
      
      if(next){
        _next->state(!last, logic::statement::init);
        _next->state(!last, logic::statement::transition);
        _next->state(last,  logic::statement::final);
      }
    }
    else{
      object x_phi = gamma.declare({"xy_phi", types::boolean()}, resolution::delayed);
      _next->state(x_phi,                       logic::statement::final);
      _next->state(primed(x_phi) == phi.SNFSed, logic::statement::transition);
      _next->state(!x_phi,                      logic::statement::init);
    }
    return gamma;
  }

  /*
    Parameters:
      - gamma: The module in which all surrogates are declared;
      - aux: Auxiliary modules in which free variables are declared. aux is thus used to retrieve the free variables through lookup();
      - t: The term to be converted into SNFS';
      - future: Flag. True if and only if t is part of a future LTL formula;
      - surrID: Incremental ID used to identify surrogates in debug;
      - is_next: Flag. True if and only if t is part of a formula containing a next or a weak next operator.
      - from_atom: Flag. True if the term containing t is an atom. Refer to comments at lines 233, 234, 235.
  */
  SNFSer automaton_t::impl_t::to_SNFS(module &gamma, module &aux, term t, bool &future, int &surrID, const term last, bool &is_next, bool from_atom) {
    return match(t)(
      /*
        Base cases.
      */
      [&](variable var) -> SNFSer { return {var, { var }, no_next}; },
      [&](any_of<integer, real, boolean, object> auto c) -> SNFSer { return { c, { }, no_next}; },

      /*
        Quantifiers.
      */
      [&]<any_of<exists, forall> T>(T, std::vector<decl> decls, term body) -> SNFSer {
        module aux2 = aux;

        for(decl d : decls) {
          aux2.declare(d, resolution::immediate);
        }

        SNFSer rec = to_SNFS(gamma, aux2, body, future, surrID, last, is_next, false);
        term new_term = T(decls, rec.SNFSed);
        std::vector<variable> new_fv = quantify(decls, rec.free_vars);

        if(rec.next_control == next){
          return { last && new_term, new_fv, no_next };
        }
        if(rec.next_control == w_next){
          return { implication(last, new_term), new_fv, no_next };
        }        
        return { T(decls, rec.SNFSed), quantify(decls, rec.free_vars), no_next };
      },
      
      [&](atom, term P, std::vector<term> arguments) -> SNFSer { 
        std::vector<variable> free_vars = { };
        std::vector<term> new_arguments = { };
        
        bool in_next = false;
        bool in_weak = false;

        for(term t : arguments) {
          // Here we need a different way of treating the case t = X(obj), where obj is boolean.
          // Suppose to have a predicate P(X(obj)).
          // Then, the SNFSed form should be last && P(xc_obj'), and not P(last && xc_obj').
          SNFSer rec = to_SNFS(gamma, aux, t, future, surrID, last, is_next, true);
          free_vars = vec_union(free_vars, rec.free_vars);
          new_arguments.push_back(rec.SNFSed);
          in_next = in_next || (rec.next_control == next);
          in_weak = in_weak || (rec.next_control == w_next); 
        }
        
        atom new_atom = atom(P, new_arguments);
        if(in_next){
          return { !last && new_atom, free_vars, no_next };
        }
        if(in_weak){
            return { implication(!last, new_atom), free_vars, no_next };
        }
        return { new_atom, free_vars, no_next };
      },

      [&](any_of<tomorrow, w_tomorrow> auto t, term argument) -> SNFSer {
        future = true;

        std::string surr_label;
        next_ops nc;

        match(t)(
          [&](tomorrow){ surr_label = "x"; nc = next; },
          [&](w_tomorrow){ surr_label = "w"; nc = w_next; }
        );

        return match(argument)(
          [&](object obj) -> SNFSer {
            is_next = true;
            object surr = gamma.declare(
              surr_label + "c_" + obj.entity()->name.name().to_string(),
              obj.entity()->type,
              role::state,
              resolution::delayed
            );

            _next->state(surr == argument, logic::statement::transition);

            /*
              The following if statement is satisfied whenever a (weak) next operator is applied to a boolean constant,
              and if such term is not an argument of an atom. See comments at lines 233, 234, 235.
              In such case, the last condition is immediately checked.
            */
            if(obj.entity()->type == types::boolean() && !from_atom){
              if(nc == next){
                return { !last && primed(surr), { }, no_next };
              }
              else{
                return { implication(!last, primed(surr)), { }, no_next };
              }
            }
            return { primed(surr), { }, nc };
          },
          [&](otherwise) -> SNFSer {
            surrID ++;
            SNFSer rec = to_SNFS(gamma, aux, argument, future, surrID, last, is_next, false);

            std::vector<term> free_terms = { };
            std::vector<decl> decls = { };
            std::vector<types::type> types = { };

            for(variable var : rec.free_vars){
                types::type t = type_from_module(aux, var);
                decls.push_back({var, t});
                types.push_back(t);
                free_terms.push_back(var);
            }
            
            term surrogate = atom(
              gamma.declare(
                surr_label + "s_" + std::to_string(surrID), types::function(types, types::boolean()),
                role::state,
                resolution::delayed
                ),
              free_terms
            );

            return match(t)(
              [&](tomorrow) -> SNFSer {
                _next->state(forall(decls, surrogate == primed(rec.SNFSed)), logic::statement::transition);
                _next->state(forall(decls, !surrogate),                      logic::statement::final);
                return { surrogate, rec.free_vars, no_next };
              },
              [&](w_tomorrow) -> SNFSer {
                _next->state(forall(decls, surrogate == primed(rec.SNFSed)), logic::statement::transition);
                _next->state(forall(decls, surrogate),                       logic::statement::final);
                return { surrogate, rec.free_vars, no_next };
              }
            );
          }
        );
      },

      [&](any_of<yesterday, w_yesterday, eventually, always, once, historically> auto t, term argument) -> SNFSer {
        surrID ++;
        SNFSer rec = to_SNFS(gamma, aux, argument, future, surrID, last, is_next, false);

        std::vector<term> free_terms = { };
        std::vector<decl> decls = { };
        std::vector<types::type> types = { };

        for(variable var : rec.free_vars){
            types::type t = type_from_module(aux, var);
            decls.push_back({var, t});
            types.push_back(t);
            free_terms.push_back(var);
        }

        std::string surr_label = match(t)(
          [&](eventually)                             { future = true; return "xs_"; },
          [&](always)                                 { future = true; return "xw_"; },
          [&](any_of<yesterday, once> auto)           { return "xy_"; },
          [&](any_of<w_yesterday, historically> auto) { return "xz_"; }
        ) + std::to_string(surrID);

        term surrogate = atom(
          gamma.declare(
            surr_label, types::function(types, types::boolean()),
            role::state,
            resolution::delayed
          ),
          free_terms
        );

        return match(t)(
          [&](yesterday) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed), logic::statement::transition);
            _next->state(forall(decls, !surrogate),                      logic::statement::init);
            return { surrogate, rec.free_vars, no_next };
          },
          [&](w_yesterday) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed), logic::statement::transition);
            _next->state(forall(decls, surrogate),                       logic::statement::init);
            return { surrogate, rec.free_vars, no_next };
          },
          [&](eventually) -> SNFSer {
            _next->state(forall(decls, surrogate == primed(rec.SNFSed || surrogate)), logic::statement::transition);
            _next->state(forall(decls, !surrogate), logic::statement::final);
            return { rec.SNFSed || surrogate, rec.free_vars, no_next };
          },
          [&](always) -> SNFSer {
            _next->state(forall(decls, surrogate == primed(rec.SNFSed && surrogate)), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::final);
            return { rec.SNFSed && surrogate, rec.free_vars, no_next };
          },
          [&](once) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed || surrogate), logic::statement::transition);
            _next->state(forall(decls, !surrogate), logic::statement::init);
            return { rec.SNFSed || surrogate, rec.free_vars, no_next };
          },
          [&](historically) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed && surrogate), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::init);
            return { rec.SNFSed && surrogate, rec.free_vars, no_next };
          }
        );
      },
      
      [&](any_of<until, release, since, triggered> auto t, term left, term right) -> SNFSer {
        surrID ++;

        SNFSer rec_left  = to_SNFS(gamma, aux, left,  future, surrID, last, is_next, false);
        SNFSer rec_right = to_SNFS(gamma, aux, right, future, surrID, last, is_next, false);

        std::vector<variable> free_vars = vec_union(rec_left.free_vars, rec_right.free_vars);
        std::vector<term> free_terms = { };
        std::vector<decl> decls = { };

        std::vector<types::type> types = { };
        for(variable var : free_vars){
            types::type t = type_from_module(aux, var);
            decls.push_back({var, t});
            types.push_back(t);
            free_terms.push_back(var);
        }

        std::string surr_label = match(t)(
          [&](until)      { future = true; return "xs_"; },
          [&](release)    { future = true; return "xw_"; },
          [&](since)      { return "xy_"; },
          [&](triggered)  { return "xz_"; }
        ) + std::to_string(surrID);

        term surrogate = atom(
          gamma.declare(
            surr_label, types::function(types, types::boolean()),
            role::state,
            resolution::delayed
          ),
          free_terms
        );

        return match(t)(
          [&](until) -> SNFSer {
            _next->state(forall(decls, surrogate == primed((rec_right.SNFSed || (rec_left.SNFSed && surrogate)))), logic::statement::transition);
            _next->state(forall(decls, !surrogate), logic::statement::final);
            return { rec_right.SNFSed || (rec_left.SNFSed && surrogate), free_vars, no_next };
          },

          [&](release) -> SNFSer {            
            _next->state(forall(decls, surrogate == primed((rec_right.SNFSed && (rec_left.SNFSed || surrogate)))), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::final);
            return { rec_right.SNFSed && (rec_left.SNFSed || surrogate), free_vars, no_next };
          },

          [&](since) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec_right.SNFSed || (rec_left.SNFSed && surrogate)), logic::statement::transition);
            _next->state(forall(decls, !surrogate), logic::statement::init);
            return { rec_right.SNFSed || (rec_left.SNFSed && surrogate), free_vars, no_next };
          },

          [&](triggered) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec_right.SNFSed && (rec_left.SNFSed || surrogate)), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::init);
            return { rec_right.SNFSed && (rec_left.SNFSed || surrogate), free_vars, no_next };
          }
        );
      },

      [&]<any_of<equal, distinct, conjunction, disjunction> T>(T, std::vector<term> arguments) -> SNFSer { 
        SNFSer rec_left = to_SNFS(gamma, aux, arguments[0], future, surrID, last, is_next, false);
        SNFSer rec_right = to_SNFS(gamma, aux, arguments[1], future, surrID, last, is_next, false);

        if(rec_left.next_control == next || rec_right.next_control == next){
          return {!last && T({rec_left.SNFSed, rec_right.SNFSed}), vec_union(rec_left.free_vars, rec_right.free_vars), no_next };
        }
        if(rec_left.next_control == w_next || rec_right.next_control == w_next){
          return {implication(!last, T({rec_left.SNFSed, rec_right.SNFSed})), vec_union(rec_left.free_vars, rec_right.free_vars), no_next };
        }
        return { T({rec_left.SNFSed, rec_right.SNFSed}), vec_union(rec_left.free_vars, rec_right.free_vars), no_next };
      },

      [&]<any_of<implication, less_than, less_than_eq, greater_than, greater_than_eq> T>(T, term left, term right) -> SNFSer {
        SNFSer rec_left = to_SNFS(gamma, aux, left, future, surrID, last, is_next, false);
        SNFSer rec_right = to_SNFS(gamma, aux, right, future, surrID, last, is_next, false);

        if(rec_left.next_control == next || rec_right.next_control == next){
          return { last && T(rec_left.SNFSed, rec_right.SNFSed), vec_union(rec_left.free_vars, rec_right.free_vars), no_next };
        }
        if(rec_left.next_control == w_next || rec_right.next_control == w_next){
          return { implication(last, T(rec_left.SNFSed, rec_right.SNFSed)), vec_union(rec_left.free_vars, rec_right.free_vars), no_next };
        }
        return { T(rec_left.SNFSed, rec_right.SNFSed), vec_union(rec_left.free_vars, rec_right.free_vars), no_next };
      },

      /*
        WARNING: These are not predicates. As such, the addition of "last && t" (resp. "last => t") is postponed.
        e.g.: to_SNFS(X(c) + 1 > x) is last && (xc_c' + 1 > x), and not (last && xc_c' + 1) > x.
      */
      [&]<any_of<sum, product, difference, division> T>(T, term left, term right) -> SNFSer {
        SNFSer rec_left = to_SNFS(gamma, aux, left, future, surrID, last, is_next, false);
        SNFSer rec_right = to_SNFS(gamma, aux, right, future, surrID, last, is_next, false);

        if(rec_left.next_control == next || rec_right.next_control == next){
          return { T(rec_left.SNFSed, rec_right.SNFSed), vec_union(rec_left.free_vars, rec_right.free_vars), next };
        }
        if(rec_left.next_control == w_next || rec_right.next_control == w_next){
          return { T(rec_left.SNFSed, rec_right.SNFSed), vec_union(rec_left.free_vars, rec_right.free_vars), w_next };
        }
        return { T(rec_left.SNFSed, rec_right.SNFSed), vec_union(rec_left.free_vars, rec_right.free_vars), no_next };
      }
    );
  }
}
