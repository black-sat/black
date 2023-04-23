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

namespace black_internal {
  struct cache_key_t {
    logic::formula<LTLXFG> formula;
    size_t nonce = 0;

    bool operator==(cache_key_t const&) const = default;
  };
}

template<>
struct std::hash<black_internal::cache_key_t> {
  size_t operator()(black_internal::cache_key_t l) const {
    return 
      std::hash<black::logic::formula<black_internal::LTLXFG>>{}(l.formula) 
      + l.nonce;
  }
};

namespace black_internal {
  
  using formula = logic::formula<LTLXFG>;
  namespace sdd = black::sdd;

  struct XBool : logic::make_combined_fragment_t<
    logic::propositional,
    logic::make_fragment_t<
      logic::syntax_list<
        logic::syntax_element::tomorrow,
        logic::syntax_element::w_tomorrow
      >
    >
  > { };

  struct incremental_t {
    
    incremental_t(sdd::manager *_mgr) 
      : mgr{_mgr}, sigma{*_mgr->sigma()} { }

    void collect_letters(formula f);
    sdd::variable fresh();

    formula nnf(formula);

    automaton product(automaton, automaton);
    automaton sum(automaton, automaton);
    automaton negation(automaton);
    automaton implication(automaton a1, automaton a2);
    automaton not_empty(automaton);
    formula preprocess(formula f);

    automaton encode(formula f);

    automaton to_automaton(size_t, logic::formula<logic::propositional> f);
    automaton to_automaton(size_t, logic::negation<LTLXFG>, formula);
    automaton to_automaton(size_t, logic::conjunction<LTLXFG>,formula, formula);
    automaton to_automaton(size_t, logic::disjunction<LTLXFG>,formula, formula);
    automaton to_automaton(size_t, logic::implication<LTLXFG>,formula, formula);
    automaton to_automaton(size_t, logic::iff<LTLXFG>, formula, formula);
    automaton to_automaton(size_t, logic::eventually<LTLXFG>, formula);
    automaton to_automaton(size_t, logic::always<LTLXFG>, formula);
    automaton to_automaton(size_t, logic::tomorrow<LTLXFG>, formula);
    automaton to_automaton(size_t, logic::w_tomorrow<LTLXFG>, formula);
    automaton to_automaton(size_t, formula f);
    
    automaton to_automaton(size_t, logic::otherwise, auto ...) {
      black_unreachable();
    }

    sdd::manager *mgr;
    logic::alphabet &sigma;
    std::vector<logic::proposition> letters;
    size_t next_fresh = 0;
    size_t indentn = 0;
    size_t level = 0;
    tsl::hopscotch_map<cache_key_t, automaton> cache;
  };

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

  sdd::variable incremental_t::fresh() {
    std::string indent(indentn * 3, ' ');
    std::cerr << indent << "allocating fresh variable\n";
    return mgr->variable(
      renamings::freshed(sigma.proposition("fresh"))
    );
  }

  formula incremental_t::nnf(formula f) {
    logic::scope xi{sigma};
    return encoder::encoder{f, xi, true}.to_nnf(f).to<formula>().value();
  }

  automaton incremental_t::negation(automaton arg) {
    return not_empty(automaton{
      .manager = mgr,
      .letters = letters,
      .variables = arg.variables,
      .init = arg.init,
      .trans = arg.trans,
      .finals = !arg.finals
    });
  }

  automaton incremental_t::product(automaton a1, automaton a2) {
    tsl::hopscotch_set<logic::proposition> set;
    set.insert(begin(a1.variables), end(a1.variables));
    set.insert(begin(a2.variables), end(a2.variables));

    std::string indent(indentn * 3, ' ');

    std::cerr << indent << "computing trans1 && trans2...\n";
    sdd::node trans = a1.trans && a2.trans;
    std::cerr << indent << " - size: " << trans.count() << "\n";

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = std::vector<logic::proposition>(begin(set), end(set)),
      .init = a1.init && a2.init,
      .trans = trans,
      .finals = a1.finals && a2.finals
    };
  }

  automaton incremental_t::sum(automaton a1, automaton a2) {
    tsl::hopscotch_set<logic::proposition> set;
    set.insert(begin(a1.variables), end(a1.variables));
    set.insert(begin(a2.variables), end(a2.variables));

    std::string indent(indentn * 3, ' ');

    std::cerr << indent << "computing trans1 && trans2...\n";
    sdd::node trans = a1.trans && a2.trans;
    std::cerr << indent << " - size: " << trans.count() << "\n";

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = std::vector<logic::proposition>(begin(set), end(set)),
      .init = a1.init && a2.init,
      .trans = trans,
      .finals = a1.finals || a2.finals
    };
  }
  
  automaton incremental_t::implication(automaton a1, automaton a2) {
    tsl::hopscotch_set<logic::proposition> set;
    set.insert(begin(a1.variables), end(a1.variables));
    set.insert(begin(a2.variables), end(a2.variables));

    std::string indent(indentn * 3, ' ');

    std::cerr << indent << "computing trans1 && trans2...\n";
    sdd::node trans = a1.trans && a2.trans;
    std::cerr << indent << " - size: " << trans.count() << "\n";

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = std::vector<logic::proposition>(begin(set), end(set)),
      .init = a1.init && a2.init,
      .trans = trans,
      .finals = !a1.finals || a2.finals
    };
  }

  automaton incremental_t::not_empty(automaton aut) {
    sdd::variable var = fresh();

    std::vector<logic::proposition> variables = aut.variables;
    variables.push_back(var.name());

    return automaton{
      .manager = mgr,
      .letters = letters,
      .variables = variables,
      .init = var && aut.init,
      .trans = (aut.trans && !prime(var)),
      .finals = aut.finals && !var
    };
  }

  formula incremental_t::preprocess(formula f) {
    using namespace logic;

    return f.match(
      [](boolean b) { return b; },
      [](proposition p) { return p; },
      [&](always<LTLXFG>, auto arg) {
        return arg.match(
          [&](conjunction<LTLXFG> c) {
            return big_and(sigma, c.operands(), [&](auto op) {
              return preprocess(G(op));
            });
          },
          [&](otherwise) {
            return G(preprocess(arg));
          }
        );
      },
      [&](eventually<LTLXFG>, auto arg) {
        return arg.match(
          [&](disjunction<LTLXFG> c) {
            return big_or(sigma, c.operands(), [&](auto op) {
              return preprocess(F(op));
            });
          },
          [&](otherwise) {
            return F(preprocess(arg));
          }
        );
      },
      [&](unary<LTLXFG> u, auto arg) {
        return unary<LTLXFG>(u.node_type(), preprocess(arg));
      },
      [&](binary<LTLXFG> b, auto left, auto right) {
        return binary<LTLXFG>(
          b.node_type(), preprocess(left), preprocess(right)
        );
      }
    );
  }

  automaton incremental_t::encode(formula f) {
    collect_letters(f);
    return not_empty(to_automaton(0, preprocess(nnf(f))));
  }

  automaton incremental_t::to_automaton(size_t nonce, formula f) {
    std::string indent(indentn * 3, ' ');

    std::cerr << indent
              << "building automaton for: " << black::to_string(f) << "\n";

    if(cache.contains({f, nonce})) {
      std::cerr << indent << "cache hit!\n";
      return cache.at({f, nonce});
    }

    if(auto b = f.to<logic::formula<logic::propositional>>(); b)
      return to_automaton(nonce, *b);

    automaton result = f.match(
      [&](auto ...args) { return to_automaton(nonce, args...); }
    );

    cache.insert({{f, nonce}, result});
    
    return result;
  }

  automaton incremental_t::to_automaton(
    size_t, logic::formula<logic::propositional> f
  ) {
    sdd::variable var = fresh();

    return automaton{
      .manager = mgr,
      .letters = letters,
      .variables = {var.name()},
      .init = !var,
      .trans = iff(prime(var), var || mgr->to_node(f)),
      .finals = var
    };
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::negation<LTLXFG>, formula arg
  ) { 
    return negation(to_automaton(nonce, arg));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::conjunction<LTLXFG> conj, formula, formula
  ) {
    using namespace std;

    std::string indent(indentn * 3, ' ');

    std::vector<formula> operands(begin(conj.operands()), end(conj.operands()));
    auto temp = std::partition(begin(operands), end(operands), [](formula op) {
      return op.is<logic::formula<logic::propositional>>();
    });

    std::cerr << indent << "found " << std::distance(begin(operands), temp)
              << " boolean subformulas in conjunction\n";

    formula booleanf = 
      big_and(sigma, std::ranges::subrange(begin(operands), temp));

    std::cerr << indent << "building the boolean part...\n";
    automaton aut = to_automaton(nonce, booleanf);
    std::cerr << indent << "done!\n";
    for(auto it = temp; it != end(operands); ++it) {
      std::cerr << indent << "building a product ...\n";
      indentn++;
      aut = product(aut, to_automaton(nonce, *it));
      indentn--;
      std::cerr << indent << "done!\n";
    }
    return aut;
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::disjunction<LTLXFG>, formula l, formula r
  ) {
    return sum(to_automaton(nonce, l), to_automaton(nonce, r));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::implication<LTLXFG>, formula l, formula r
  ) {
    return implication(to_automaton(nonce, l), to_automaton(nonce, r));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::iff<LTLXFG>, formula l, formula r
  ) {
    return to_automaton(nonce, implies(l, r) && implies(r, l));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::eventually<LTLXFG> f, formula arg
  ) {
    std::string indent(indentn * 3, ' ');

    if(arg.is<logic::formula<XBool>>()) { // TODO: come up with other criteria
      std::cerr << indent << "bailing to monolithic procedure: " << std::flush;
      return semideterminize(black_internal::to_automaton(mgr, f));
    }

    automaton aut = to_automaton(nonce, arg);
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

    

    std::cerr << indent << "semideterminizing (" 
              << black::to_string(f) << ") ...\n";
    std::cerr << indent << " - vars: " << xf.variables.size() << "\n";
    std::cerr << indent << " - size: " << xf.trans.count() << "\n";
    std::cerr << indent << " - " << std::flush;

    return sum(aut, semideterminize(xf)); // aut || xf
  }
  
  automaton incremental_t::to_automaton(
    size_t nonce, logic::always<LTLXFG>, formula arg
  ) {
    return to_automaton(nonce, !F(nnf(!arg)));
  }

  automaton incremental_t::to_automaton(
    size_t, logic::tomorrow<LTLXFG>, formula arg
  ) {
    automaton aut = to_automaton(sigma.nonce(), arg);

    sdd::variable var = fresh();

    std::vector<black::proposition> vars = aut.variables;
    vars.push_back(var.name());

    std::string indent(indentn * 3, ' ');
    std::cerr << indent << "aut.init size: " << aut.init.count() << "\n";

    sdd::node trans = 
      (!var && aut.trans && !prime(var)) || 
      (var && !prime(var) && aut.init[aut.variables / primed()]);

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = vars,
      .init = var,
      .trans = trans,
      .finals = !var && aut.finals
    };
  }

  automaton incremental_t::to_automaton(
    size_t, logic::w_tomorrow<LTLXFG>, formula arg
  ) {
    automaton aut = to_automaton(sigma.nonce(), arg);

    sdd::variable var = fresh();

    std::vector<black::proposition> vars = aut.variables;
    vars.push_back(var.name());

    sdd::node trans = 
      (!var && aut.trans && !prime(var)) || 
      (var && !prime(var) && aut.init[aut.variables / primed()]);

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = vars,
      .init = var,
      .trans = trans,
      .finals = !var || aut.finals
    };
  }

  automaton to_automaton_incremental(sdd::manager *mgr, formula f) {
    return incremental_t{mgr}.encode(f);
  }

}