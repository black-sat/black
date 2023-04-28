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
    logic::formula<logic::LTLP> formula;
    size_t nonce = 0;

    bool operator==(cache_key_t const&) const = default;
  };
}

template<>
struct std::hash<black_internal::cache_key_t> {
  size_t operator()(black_internal::cache_key_t l) const {
    return 
      std::hash<black::logic::formula<black::logic::LTLP>>{}(l.formula) 
      + l.nonce;
  }
};

namespace black_internal {
  
  using LTLP = logic::LTLP;
  using formula = logic::formula<LTLP>;
  namespace bdd = black::bdd;

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
    
    incremental_t(bdd::manager *_mgr) 
      : mgr{_mgr}, sigma{*_mgr->sigma()} { }

    void collect_letters(formula f);
    bdd::variable fresh();

    formula nnf(formula);

    automaton product(automaton, automaton);
    automaton sum(automaton, automaton);
    automaton negation(automaton);
    automaton implication(automaton a1, automaton a2);
    automaton not_empty(automaton);
    automaton reverse(automaton);
    formula reverse(formula);
    formula preprocess(formula f);

    automaton encode(formula f);

    automaton to_automaton(size_t, logic::formula<logic::propositional> f);
    automaton to_automaton(size_t, logic::negation<LTLP>, formula);
    automaton to_automaton(size_t, logic::conjunction<LTLP>,formula, formula);
    automaton to_automaton(size_t, logic::disjunction<LTLP>,formula, formula);
    automaton to_automaton(size_t, logic::implication<LTLP>,formula, formula);
    automaton to_automaton(size_t, logic::iff<LTLP>, formula, formula);
    automaton to_automaton(size_t, logic::eventually<LTLP>, formula);
    automaton to_automaton(size_t, logic::always<LTLP>, formula);
    automaton to_automaton(size_t, logic::tomorrow<LTLP>, formula);
    automaton to_automaton(size_t, logic::w_tomorrow<LTLP>, formula);
    automaton to_automaton(size_t, logic::until<LTLP>, formula, formula);
    // automaton to_automaton(size_t, logic::release<LTLP>, formula, formula);
    // automaton to_automaton(size_t, logic::since<LTLP>, formula, formula);
    // automaton to_automaton(size_t, logic::triggered<LTLP>, formula, formula);
    automaton to_automaton(size_t, logic::once<LTLP>, formula);
    automaton to_automaton(size_t, logic::historically<LTLP>, formula);
    // automaton to_automaton(size_t, logic::yesterday<LTLP>, formula);
    // automaton to_automaton(size_t, logic::w_yesterday<LTLP>, formula);
    automaton to_automaton(size_t, formula f);
    
    automaton to_automaton(size_t, logic::otherwise, auto ...) {
      throw std::runtime_error("unimplemented");
    }

    bdd::manager *mgr;
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

  bdd::variable incremental_t::fresh() {
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
    bdd::node trans = a1.trans && a2.trans;
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
    bdd::node trans = a1.trans && a2.trans;
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
    bdd::node trans = a1.trans && a2.trans;
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
    bdd::variable var = fresh();

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

  automaton incremental_t::reverse(automaton aut) {
    return semideterminize(automaton {
      .manager = mgr,
      .letters = letters,
      .variables = aut.variables,
      .init = aut.finals,
      .trans = aut.trans[aut.variables / primed(2)]
                        [primed(1) * aut.variables / to_plain()]
                        [primed(2) * aut.variables / primed(1)],
      .finals = aut.init
    });
  }

  formula incremental_t::reverse(formula f) {
    //using namespace logic;

    return f.match(
      [](logic::boolean b) { return b; },
      [](logic::proposition p) { return p; },
      [&](logic::negation<LTLP>, auto arg) {
        return !reverse(arg);
      },
      [&](logic::conjunction<LTLP> c) {
        return big_and(*c.sigma(), c.operands(), [&](auto op) {
          return reverse(op);
        });
      },
      [&](logic::disjunction<LTLP> c) {
        return big_or(*c.sigma(), c.operands(), [&](auto op) {
          return reverse(op);
        });
      },
      [&](logic::implication<LTLP>, auto left, auto right) {
        return implies(reverse(left), reverse(right));
      },
      [&](logic::iff<LTLP>, auto left, auto right) {
        return logic::iff(reverse(left), reverse(right));
      },
      [&](logic::tomorrow<LTLP>, auto arg) {
        return Y(reverse(arg));
      },
      [&](logic::w_tomorrow<LTLP>, auto arg) {
        return Z(reverse(arg));
      },
      [&](logic::always<LTLP>, auto arg) {
        return H(reverse(arg));
      },
      [&](logic::eventually<LTLP>, auto arg) {
        return O(reverse(arg));
      },
      [&](logic::until<LTLP>, auto left, auto right) {
        return S(reverse(left), reverse(right));
      },
      [&](logic::release<LTLP>, auto left, auto right) {
        return T(reverse(left), reverse(right));
      },
      [&](logic::yesterday<LTLP>, auto arg) {
        return X(reverse(arg));
      },
      [&](logic::w_yesterday<LTLP>, auto arg) {
        return wX(reverse(arg));
      },
      [&](logic::once<LTLP>, auto arg) {
        return F(reverse(arg));
      },
      [&](logic::historically<LTLP>, auto arg) {
        return G(reverse(arg));
      },
      [&](logic::since<LTLP>, auto left, auto right) {
        return U(reverse(left), reverse(right));
      },
      [&](logic::triggered<LTLP>, auto left, auto right) {
        return R(reverse(left), reverse(right));
      },
      [](logic::otherwise) -> logic::formula<LTLP> {
        black_unreachable();
      }
    );
  }

  formula incremental_t::preprocess(formula f) {
    using namespace logic;

    return f.match(
      [](boolean b) { return b; },
      [](proposition p) { return p; },
      [&](always<LTLP>, auto arg) {
        return arg.match(
          [&](conjunction<LTLP> c) {
            return big_and(sigma, c.operands(), [&](auto op) {
              return preprocess(G(op));
            });
          },
          [&](otherwise) {
            return G(preprocess(arg));
          }
        );
      },
      [&](eventually<LTLP>, auto arg) {
        return arg.match(
          [&](disjunction<LTLP> c) {
            return big_or(sigma, c.operands(), [&](auto op) {
              return preprocess(F(op));
            });
          },
          [&](otherwise) {
            return F(preprocess(arg));
          }
        );
      },
      [&](unary<LTLP> u, auto arg) {
        return unary<LTLP>(u.node_type(), preprocess(arg));
      },
      [&](binary<LTLP> b, auto left, auto right) {
        return binary<LTLP>(
          b.node_type(), preprocess(left), preprocess(right)
        );
      }
    );
  }

  automaton incremental_t::encode(formula f) {
    collect_letters(f);
    return to_automaton(0, preprocess(nnf(f)));
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
      [&](auto ...args) { 
        indentn++;
        automaton r = to_automaton(nonce, args...); 
        indentn--;
        return r;
      }
    );

    cache.insert({{f, nonce}, result});
    
    return result;
  }

  automaton incremental_t::to_automaton(
    size_t, logic::formula<logic::propositional> f
  ) {
    bdd::variable var = fresh();

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
    size_t nonce, logic::negation<LTLP>, formula arg
  ) { 
    return negation(to_automaton(nonce, arg));
  }

  template<typename It, typename F>
  auto binreduce(It begin, It end, F op) {
    auto d = std::distance(begin, end);

    black_assert(d > 0);
    if(d == 1)
      return *begin;
    if(d == 2)
      return op(*begin, *(begin + 1));
    
    auto pivot = begin + (d / 2);
    return op(binreduce(begin, pivot, op), binreduce(pivot, end, op));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::conjunction<LTLP> conj, formula, formula
  ) {
    using namespace std;

    std::vector<automaton> partials;

    std::vector<formula> operands(begin(conj.operands()), end(conj.operands()));
    auto temp = std::partition(begin(operands), end(operands), [](formula op) {
      return op.is<logic::formula<logic::propositional>>();
    });

    if(temp != begin(operands))
      partials.push_back(
        to_automaton(nonce, 
          big_and(sigma, std::ranges::subrange(begin(operands), temp))
        )
      );

    for(auto v : std::ranges::subrange(temp, end(operands)))
      partials.push_back(to_automaton(nonce, v));
    
    return binreduce(begin(partials), end(partials), [&](auto a, auto b) {
      return product(a,b);
    });
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::disjunction<LTLP> conj, formula, formula
  ) {
    using namespace std;

    std::vector<automaton> partials;

    std::vector<formula> operands(begin(conj.operands()), end(conj.operands()));
    auto temp = std::partition(begin(operands), end(operands), [](formula op) {
      return op.is<logic::formula<logic::propositional>>();
    });

    if(temp != begin(operands))
      partials.push_back(
        to_automaton(nonce, 
          big_and(sigma, std::ranges::subrange(begin(operands), temp))
        )
      );

    for(auto v : std::ranges::subrange(temp, end(operands)))
      partials.push_back(to_automaton(nonce, v));
    
    return binreduce(begin(partials), end(partials), [&](auto a, auto b) {
      return sum(a,b);
    });
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::implication<LTLP>, formula l, formula r
  ) {
    return implication(to_automaton(nonce, l), to_automaton(nonce, r));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::iff<LTLP>, formula l, formula r
  ) {
    return to_automaton(nonce, implies(l, r) && implies(r, l));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::eventually<LTLP> f, formula arg
  ) {
    std::string indent(indentn * 3, ' ');

    if(arg.is<logic::formula<XBool>>()) { // TODO: come up with other criteria
      std::cerr << indent << "bailing to monolithic procedure: " << std::flush;
      return semideterminize(black_internal::to_automaton(mgr, f));
    }

    automaton aut = to_automaton(nonce, arg);
    bdd::variable var = fresh();

    std::vector<black::proposition> xfvars = aut.variables;
    xfvars.push_back(var.name());

    bdd::node trans = 
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
    //std::cerr << indent << " - size: " << xf.trans.count() << "\n";
    std::cerr << indent << " - " << std::flush;

    return sum(aut, semideterminize(xf)); // aut || xf
  }
  
  automaton incremental_t::to_automaton(
    size_t nonce, logic::always<LTLP>, formula arg
  ) {
    return to_automaton(nonce, !F(nnf(!arg)));
  }

  automaton incremental_t::to_automaton(
    size_t, logic::tomorrow<LTLP>, formula arg
  ) {
    automaton aut = to_automaton(sigma.nonce(), arg);

    bdd::variable var = fresh();

    std::vector<black::proposition> vars = aut.variables;
    vars.push_back(var.name());

    std::string indent(indentn * 3, ' ');
    //std::cerr << indent << "aut.init size: " << aut.init.count() << "\n";

    bdd::node trans = 
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
    size_t, logic::w_tomorrow<LTLP>, formula arg
  ) {
    automaton aut = to_automaton(sigma.nonce(), arg);

    bdd::variable var = fresh();

    std::vector<black::proposition> vars = aut.variables;
    vars.push_back(var.name());

    bdd::node trans = 
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

  automaton incremental_t::to_automaton(
    size_t nonce, logic::until<LTLP>, formula left, formula right
  ) {
    return 
      reverse(to_automaton(nonce, F(reverse(right) && wX(G(reverse(left))))));
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::once<LTLP>, formula arg
  ) {
    automaton aut = to_automaton(nonce, arg);
    
    bdd::variable var = fresh();

    std::vector<black::proposition> vars = aut.variables;
    vars.push_back(var.name());

    bdd::node trans = aut.trans && iff(prime(var), aut.finals);

    return automaton {
      .manager = mgr,
      .letters = letters,
      .variables = vars,
      .init = !var && aut.init,
      .trans = trans,
      .finals = var
    }; 
  }

  automaton incremental_t::to_automaton(
    size_t nonce, logic::historically<LTLP>, formula arg
  ) {
    return to_automaton(nonce, !O(!arg));
  }

  // automaton incremental_t::to_automaton(
  //   size_t nonce, logic::yesterday<LTLP>, formula arg
  // ) {

  // }

  // automaton incremental_t::to_automaton(
  //   size_t nonce, logic::w_yesterday<LTLP>, formula arg
  // ) {

  // }

  automaton to_automaton_incremental(bdd::manager *mgr, formula f) {
    return incremental_t{mgr}.encode(f);
  }

}