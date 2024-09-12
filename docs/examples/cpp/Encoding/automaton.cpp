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
#include <vector>

namespace black::pipes::internal {

  using namespace support;
  using namespace logic;
  using namespace ast;

  struct SNFSer
  {
    term SNFSed;
    std::vector<variable> free_vars;
  };

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
    std::vector<variable>::iterator find_var(std::vector<variable> &, variable);
    std::vector<variable> vec_union(std::vector<variable>, std::vector<variable>);
    std::vector<variable> quantify(std::vector<decl>, std::vector<variable>);
    types::type type_from_module(module, variable);
    module surrogates(term);
    SNFSer to_SNFS(module &, module &, term, bool &, int &);

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
      module mod = surrogates(t);
      _next->adopt(mod.resolve());      
    }
  }

  void automaton_t::impl_t::push() { }

  void automaton_t::impl_t::pop(size_t n) { _next->pop(n); }

  std::optional<object> automaton_t::impl_t::translate(object) { return {}; }

  term automaton_t::impl_t::undo(term t) { return t; }

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
    
    bool future = false;
    int surrID = 0;

    SNFSer phi = to_SNFS(gamma, aux, t, future, surrID);
    
    if(future) {
      object x_phi = gamma.declare({"xs_phi", types::boolean()}, resolution::delayed);
      _next->state(x_phi,         logic::statement::init);
      _next->state(x_phi == primed(phi.SNFSed),    logic::statement::transition);
      _next->state(!x_phi,        logic::statement::final);
    }
    else{
      object x_phi = gamma.declare({"xy_phi", types::boolean()}, resolution::delayed);
      _next->state(x_phi,         logic::statement::final);
      _next->state(primed(x_phi) == phi.SNFSed,    logic::statement::transition);
      _next->state(!x_phi,        logic::statement::init);
    }
    return gamma;
  }

  SNFSer automaton_t::impl_t::to_SNFS(module &gamma, module &aux, term t, bool &future, int &surrID) {
    return match(t)(
      /*
        Base cases.
      */
      [&](variable var) -> SNFSer { return {var, { var }}; },
      [&](object obj)   -> SNFSer { return {obj, {     }}; },

      /*
        Quantifiers.
      */
      [&]<any_of<exists, forall> T>(T, std::vector<decl> decls, term body) -> SNFSer {
        module aux2 = aux;

        for(decl d : decls) {
          aux2.declare(d, resolution::immediate);
        }

        SNFSer rec = to_SNFS(gamma, aux2, body, future, surrID);
        
        return {
          T(decls, rec.SNFSed),
          quantify(decls, rec.free_vars)
        };
      },

      /*
          LTL operators.
      */
      [&](any_of<tomorrow, w_tomorrow, yesterday, w_yesterday, eventually, always, once, historically> auto t, term argument) -> SNFSer {
        surrID ++;
        SNFSer rec = to_SNFS(gamma, aux, argument, future, surrID);

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
          [&](any_of<tomorrow, eventually> auto)      { future = true; return "xs_"; },
          [&](any_of<w_tomorrow, always> auto)        { future = true; return "xw_"; },
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
          [&](tomorrow) -> SNFSer {
            _next->state(forall(decls, surrogate == primed(rec.SNFSed)), logic::statement::transition);
            _next->state(forall(decls, !surrogate),                      logic::statement::final);
            return { surrogate, rec.free_vars };
          },
          [&](w_tomorrow) -> SNFSer {
            _next->state(forall(decls, surrogate == primed(rec.SNFSed)), logic::statement::transition);
            _next->state(forall(decls, surrogate),                       logic::statement::final);
            return { surrogate, rec.free_vars };
          },
          [&](yesterday) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed), logic::statement::transition);
            _next->state(forall(decls, !surrogate),                      logic::statement::init);
            return { surrogate, rec.free_vars };
          },
          [&](w_yesterday) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed), logic::statement::transition);
            _next->state(forall(decls, surrogate),                       logic::statement::init);
            return { surrogate, rec.free_vars };
          },
          [&](eventually) -> SNFSer {
            _next->state(forall(decls, surrogate == primed(rec.SNFSed || surrogate)), logic::statement::transition);
            _next->state(forall(decls, !surrogate), logic::statement::final);
            return { rec.SNFSed || surrogate, rec.free_vars };
          },
          [&](always) -> SNFSer {
            _next->state(forall(decls, surrogate == primed(rec.SNFSed && surrogate)), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::final);
            return { rec.SNFSed && surrogate, rec.free_vars };
          },
          [&](once) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed || surrogate), logic::statement::transition);
            _next->state(forall(decls, !surrogate), logic::statement::init);
            return { rec.SNFSed || surrogate, rec.free_vars };
          },
          [&](historically) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec.SNFSed && surrogate), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::init);
            return { rec.SNFSed && surrogate, rec.free_vars };
          }
        );
      },
      [&](any_of<until, release, since, triggered> auto t, term left, term right) -> SNFSer {
        surrID ++;

        SNFSer rec_left  = to_SNFS(gamma, aux, left,  future, surrID);
        SNFSer rec_right = to_SNFS(gamma, aux, right, future, surrID);

        std::vector<variable> free_vars = vec_union(rec_left.free_vars, rec_right.free_vars);
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
            return { rec_right.SNFSed || (rec_left.SNFSed && surrogate), free_vars };
          },

          [&](release) -> SNFSer {            
            _next->state(forall(decls, surrogate == primed((rec_right.SNFSed && (rec_left.SNFSed || surrogate)))), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::final);
            return { rec_right.SNFSed && (rec_left.SNFSed || surrogate), free_vars };
          },

          [&](since) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec_right.SNFSed || (rec_left.SNFSed && surrogate)), logic::statement::transition);
            _next->state(forall(decls, !surrogate), logic::statement::init);
            return { rec_right.SNFSed || (rec_left.SNFSed && surrogate), free_vars };
          },

          [&](triggered) -> SNFSer {
            _next->state(forall(decls, primed(surrogate) == rec_right.SNFSed && (rec_left.SNFSed || surrogate)), logic::statement::transition);
            _next->state(forall(decls, surrogate), logic::statement::init);
            return { rec_right.SNFSed && (rec_left.SNFSed || surrogate), free_vars };
          }
        );
      },

      /* 
        Other terms.
      */
      [&]<any_of<equal, distinct, conjunction, disjunction> T>(T, std::vector<term> arguments) -> SNFSer { 
        SNFSer rec_left = to_SNFS(gamma, aux, arguments[0], future, surrID);
        SNFSer rec_right = to_SNFS(gamma, aux, arguments[1], future, surrID);
        return { T({rec_left.SNFSed, rec_right.SNFSed}), vec_union(rec_left.free_vars, rec_right.free_vars) };
      },

      [&](atom a, std::vector<term> arguments) -> SNFSer { 
        std::vector<variable> result = { };
        for(term t : arguments) {
          result = vec_union(result, to_SNFS(gamma, aux, t, future, surrID).free_vars);
        }
        return { a, result };
      },

      [&]<any_of<negation, minus> T>(T, term argument) -> SNFSer {
        SNFSer rec = to_SNFS(gamma, aux, argument, future, surrID);
        return { T(rec.SNFSed), rec.free_vars };
      },

      [&]<any_of<sum, product, difference, division, implication, less_than, less_than_eq, greater_than, greater_than_eq> T>(T, term left, term right) -> SNFSer {
        SNFSer rec_left = to_SNFS(gamma, aux, left, future, surrID);
        SNFSer rec_right = to_SNFS(gamma, aux, right, future, surrID);
        return { T(rec_left.SNFSed, rec_right.SNFSed), vec_union(rec_left.free_vars, rec_right.free_vars) };
      }
    );
  }
}