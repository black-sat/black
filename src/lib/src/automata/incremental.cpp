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

#include <black/automata/automaton.hpp>
#include <black/solver/encoding.hpp>
#include <black/logic/prettyprint.hpp>

#include <string>
#include <iostream>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

namespace black_internal::fresh {
  struct fresh_t {
    black::proposition id;
    size_t index;

    bool operator==(fresh_t const&) const = default;
  };

  inline std::string to_string(fresh_t f) {
    return black::to_string(f.id) + ":" + std::to_string(f.index);
  }
}

template<>
struct std::hash<black_internal::fresh::fresh_t> {
  size_t operator()(black_internal::fresh::fresh_t f) const {
    return std::hash<black::proposition>{}(f.id) + f.index;
  }
};

namespace black_internal {
  
  using formula = logic::formula<LTLXFG>;
  namespace sdd = black::sdd;

  struct incremental_t {
    
    incremental_t(sdd::manager *_mgr) 
      : mgr{_mgr}, sigma{*_mgr->sigma()} { }

    void collect_letters(formula f);
    sdd::variable fresh(black::identifier id = "fresh");

    black::proposition cover(
      black::proposition p,
      tsl::hopscotch_set<black::proposition> const& coverset
    );
    std::vector<black::proposition> cover(
      std::vector<black::proposition> ps,
      tsl::hopscotch_set<black::proposition> const& coverset
    );
    sdd::node cover(sdd::node f, tsl::hopscotch_set<black::proposition> const&);
    automaton cover(automaton f);

    automaton product(automaton, automaton);
    automaton sum(automaton, automaton);
    automaton negation(automaton);
    automaton not_empty();

    automaton encode(formula f);

    automaton to_automaton(logic::boolean, bool value);
    automaton to_automaton(logic::proposition, black::identifier name);
    automaton to_automaton(logic::negation<LTLXFG>, formula arg);
    automaton to_automaton(logic::conjunction<LTLXFG>, formula, formula);
    automaton to_automaton(logic::disjunction<LTLXFG>, formula, formula);
    automaton to_automaton(logic::implication<LTLXFG>, formula l, formula r);
    automaton to_automaton(logic::iff<LTLXFG>, formula l, formula r);
    automaton to_automaton(logic::eventually<LTLXFG>, formula arg);
    automaton to_automaton(logic::always<LTLXFG>, formula arg);
    automaton to_automaton(logic::tomorrow<LTLXFG>, formula arg);
    automaton to_automaton(logic::w_tomorrow<LTLXFG>, formula arg);
    automaton to_automaton(formula f);
    
    sdd::manager *mgr;
    logic::alphabet &sigma;
    std::vector<logic::proposition> letters;
    size_t next_fresh = 0;
  };

  [[maybe_unused]]
  static automaton print(formula f, automaton aut) {
    std::cerr << "\nautomaton for: " << black::to_string(f) << "\n";
    std::cerr << " - letters:\n";
    for(auto v : aut.letters)
      std::cerr << "   - " << black::to_string(v) << "\n";
    std::cerr << " - declared vars:\n";
    for(auto v : aut.variables)
      std::cerr << "   - " << black::to_string(v) << "\n";
    std::cerr << " - used vars:\n";
    std::cerr << "   - init:\n";
    for(auto v : aut.init.variables())
      std::cerr << "     - " << black::to_string(v.name()) << "\n";
    std::cerr << "   - trans:\n";
    for(auto v : aut.trans.variables())
      std::cerr << "     - " << black::to_string(v.name()) << "\n";
    std::cerr << "   - finals:\n";
    for(auto v : aut.finals.variables())
      std::cerr << "     - " << black::to_string(v.name()) << "\n";
    std::cerr << " - formulas:\n";
    std::cerr << "   - init: "
              << black::to_string(aut.manager->to_formula(aut.init)) << "\n";
    std::cerr << "   - trans: <snip>\n";
    std::cerr << "   - finals: "
              << black::to_string(aut.manager->to_formula(aut.finals)) << "\n";

    return aut;
  }

  black::proposition incremental_t::cover(
    black::proposition p, tsl::hopscotch_set<black::proposition> const& coverset
  ) {
    tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
    if(tag.primes == 0) {
      if(coverset.contains(p))
        return p.sigma()->proposition(p);
      return p;
    }
    size_t primes = tag.primes;
    tag.primes = 0;
    return prime(cover(p.sigma()->proposition(tag), coverset), primes);
  }

  std::vector<black::proposition> 
  incremental_t::cover(
    std::vector<black::proposition> ps,
    tsl::hopscotch_set<black::proposition> const& coverset
  ) {
    for(auto &p : ps)
      p = cover(p, coverset);

    return ps;
  }

  sdd::node incremental_t::cover(
    sdd::node f, tsl::hopscotch_set<black::proposition> const& coverset
  ) {
    return f.change([&](black::proposition p) {
      return cover(p, coverset);
    });
  }

  automaton incremental_t::cover(automaton aut) {
    tsl::hopscotch_set<black::proposition> coverset(
      begin(aut.variables), end(aut.variables)
    );

    return automaton {
      .manager = aut.manager,
      .letters = aut.letters,
      .variables = cover(aut.variables, coverset),
      .init = cover(aut.init, coverset),
      .trans = cover(aut.trans, coverset),
      .finals = cover(aut.finals, coverset)
    };
  }

  void incremental_t::collect_letters(formula f) {
    tsl::hopscotch_set<logic::proposition> props;
    transform(f, [&](auto child) {
      child.match(
        [&](logic::proposition p) {
          props.insert(p);
        },
        [](black::otherwise) { }
      );
    });

    letters.insert(begin(letters), begin(props), end(props));
  }

  sdd::variable incremental_t::fresh(black::identifier id) {
    return mgr->variable(
      sigma.proposition(
        fresh::fresh_t{sigma.proposition(id), next_fresh++}
      )
    );
  }

  automaton incremental_t::negation(automaton arg) {
    sdd::variable var = fresh();

    std::vector<logic::proposition> variables = arg.variables;
    variables.push_back(var.name());

    return automaton{
      .manager = mgr,
      .letters = letters,
      .variables = variables,
      .init = var && arg.init,
      .trans = arg.trans && !prime(var),
      .finals = !arg.finals && !var
    };
  }

  automaton incremental_t::product(automaton a1, automaton a2) {
    std::vector<logic::proposition> variables = a1.variables;
    variables.insert(end(variables), begin(a2.variables), end(a2.variables));

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = variables,
      .init = a1.init && a2.init,
      .trans = (a1.trans && a2.trans).minimize(),
      .finals = a1.finals && a2.finals
    };
  }

  automaton incremental_t::sum(automaton a1, automaton a2) {
    return negation(product(negation(a1), negation(a2)));
  }

  automaton incremental_t::encode(formula f) {
    collect_letters(f);
    return to_automaton(f);
  }

  automaton incremental_t::to_automaton(formula f) {
    return f.match(
      [&](auto ...args) { return to_automaton(args...); }
    );
  }

  automaton incremental_t::to_automaton(logic::boolean, bool value) {
    return automaton{
      .manager = mgr,
      .letters = letters,
      .variables = {},
      .init = mgr->top(),
      .trans = mgr->top(),
      .finals = value ? mgr->top() : mgr->bottom()
    };
  }

  automaton incremental_t::to_automaton(logic::proposition p, black::identifier)
  {
    sdd::variable var = fresh();

    return automaton{
      .manager = mgr,
      .letters = letters,
      .variables = {var.name()},
      .init = !var,
      .trans = iff(prime(var), var || mgr->variable(p)),
      .finals = var
    };
  }

  automaton incremental_t::to_automaton(logic::negation<LTLXFG>, formula arg) { 
    return negation(to_automaton(arg));
  }

  automaton incremental_t::to_automaton(
    logic::conjunction<LTLXFG> conj, formula, formula
  ) {
    automaton aut = to_automaton(sigma.top());
    for(auto op : conj.operands())
      aut = product(aut, to_automaton(op));
    return aut;
  }

  automaton incremental_t::to_automaton(
    logic::disjunction<LTLXFG> disj, formula, formula
  ) {
    automaton aut = to_automaton(sigma.top());
    for(auto op : disj.operands())
      aut = product(aut, negation(to_automaton(op)));
    return negation(aut);
  }

  automaton incremental_t::to_automaton(
    logic::implication<LTLXFG>, formula l, formula r
  ) {
    return to_automaton(!l || r);
  }

  automaton incremental_t::to_automaton(
    logic::iff<LTLXFG>, formula l, formula r
  ) {
    return to_automaton(implies(l, r) && implies(r, l));
  }

  automaton incremental_t::to_automaton(logic::eventually<LTLXFG>, formula arg) 
  {
    automaton aut = to_automaton(arg);
    sdd::variable var = fresh();

    std::vector<black::proposition> xfvars = aut.variables;
    xfvars.push_back(var.name());

    sdd::node trans = 
      (var && prime(var)) || 
      (!var && aut.trans && !prime(var)) || 
      (var && !prime(var) && aut.init[aut.variables / primed()]);

    automaton xf = automaton {
      .manager = mgr,
      .letters = letters,  
      .variables = xfvars,
      .init = var,
      .trans = trans,
      .finals = !var && aut.finals
    };

    return sum(aut,  cover(semideterminize(xf))); // aut || xf
  }
  
  automaton incremental_t::to_automaton(logic::always<LTLXFG>, formula arg) {
    return to_automaton(!F(!arg));
  }

  automaton incremental_t::to_automaton(logic::tomorrow<LTLXFG>, formula arg) {
    automaton aut = to_automaton(arg);

    sdd::variable var = fresh();

    std::vector<black::proposition> vars = aut.variables;
    vars.push_back(var.name());

    sdd::node trans = 
      (!var && aut.trans && !prime(var)) || 
      (!prime(var) && aut.init[aut.variables / primed()]);

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = vars,
      .init = var,
      .trans = trans,
      .finals = !var && aut.finals
    };
  }

  automaton incremental_t::to_automaton(logic::w_tomorrow<LTLXFG>, formula arg)
  {
    return to_automaton(!X(!arg));
  }

  automaton to_automaton_incremental(sdd::manager *mgr, formula f) {
    return incremental_t{mgr}.encode(f);
  }

}