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

#include <black/synth/synth.hpp>
#include <black/qbf/qbf.hpp>
#include <black/qbf/backend.hpp>
#include <black/sdd/sdd.hpp>

#include <iostream>
#include <algorithm>

namespace black_internal::synth {

  namespace sdd = black::sdd;

  using FG = logic::make_fragment_t<
    logic::syntax_list<
      logic::syntax_element::eventually,
      logic::syntax_element::always
    >
  >;

  template<typename T>
  using formula = logic::formula<T>;

  using bformula = black::logic::fragments::propositional::formula;
  using qbformula = black::logic::fragments::QBF::formula;

  using game_t = formula<FG>::type;

  using quantifier_t = logic::qbf<logic::QBF>::type;

  enum class player_t {
    controller,
    environment
  };

  namespace {
    struct synth_t {

      synth_t(logic::alphabet &_sigma, automata_spec const& _spec)
        : sigma{_sigma}, spec{_spec}, aut{spec.spec} { }

      bformula to_formula(sdd::node n) { 
        return cover(aut.manager->to_formula(n)); 
      }

      bformula win(player_t player, game_t type, size_t n);
      qbformula unravel(size_t n);

      qbformula encode(player_t player, game_t type, size_t n);

      logic::alphabet &sigma;
      automata_spec spec;
      automaton &aut;
    };

    bformula synth_t::win(player_t player, game_t type, size_t n) {
      using namespace logic;

      bformula finals = 
        player == player_t::controller ? 
          to_formula(aut.finals) : !to_formula(aut.finals);

      bool reach = type.match(
        [&](game_t::eventually) {
          return player == player_t::controller;
        },
        [&](game_t::always) {
          return player == player_t::environment;
        }
      );

      if(reach)
        return big_or(sigma, black::range(0, n + 1), [&](auto i) {
          return rename(finals, aut.variables / stepped(i));
        });
      
      return big_or(sigma, black::range(0, n), [&](auto k) {
        auto loop = big_or(sigma, black::range(0, k), [&](auto j) {
          auto ell = [&](auto p) {
            return iff(step(p, k), step(p, j));
          };
          return big_and(sigma, aut.letters, ell);
        });

        auto safety = big_and(sigma, black::range(0, k + 1), [&](auto w) {
          return rename(finals, aut.variables / stepped(w));
        });

        return loop && safety;
      });
    }

    qbformula synth_t::unravel(size_t n) {
      using namespace black::logic::fragments::QBF;
      return 
        rename(to_formula(aut.init), aut.variables / stepped(0)) &&
        big_and(sigma, black::range(0, n), [&](auto i) {
          return 
            rename(
              rename(to_formula(aut.trans), aut.variables / stepped(i)),
              primed() * aut.variables / stepped(i + 1)
            );
        });
    }

    qbformula synth_t::encode(player_t player, game_t type, size_t n) 
    {
      using namespace black::logic::fragments::QBF;

      auto stepvars = [&](auto const& vars, size_t s) {
        std::vector<black::proposition> result;
        for(auto var : vars)
          result.push_back(step(var, s));
        return result;
      };

      qbformula result = sigma.top();
      if(player == player_t::controller) 
        result = thereis(stepvars(aut.variables, n), 
          unravel(n) && win(player, type, n)
        );
      else
        result = foreach(stepvars(aut.variables, n), 
          implies(unravel(n), win(player, type, n))
        );

      // defaults for Controller
      quantifier_t qvars = quantifier_t::thereis{};
      quantifier_t qfirst = quantifier_t::thereis{};
      quantifier_t qsecond = quantifier_t::foreach{};

      if(player == player_t::environment) {
        qvars = quantifier_t::foreach{};
        qfirst = quantifier_t::foreach{};
        qsecond = quantifier_t::thereis{};
      }

      for(size_t i = 0; i < n; i++) {
        size_t step = n - i - 1;
        
        result = 
          qbf(qvars, stepvars(aut.variables, step),
            qbf(qfirst, stepvars(spec.outputs, step),
              qbf(qsecond, stepvars(spec.inputs, step),
                result
              )
            )
          );
      }

      return result;
    }
  }

  static black::tribool solve(automata_spec spec, game_t type) {
    logic::alphabet &sigma = *spec.spec.init.manager()->sigma();

    size_t n = 3;
    while(true) {
      [[maybe_unused]]
      qbformula formulaC = 
        synth_t{sigma, spec}.encode(player_t::controller, type, n);
      qdimacs qdC = clausify(formulaC);
      
      [[maybe_unused]]
      qbformula formulaE = 
        synth_t{sigma, spec}.encode(player_t::environment, type, n);
      qdimacs qdE = clausify(formulaE);

      std::cerr << to_string(formulaC) << "\n";
      if(is_sat(qdC))
        return true;
      
      std::cerr << to_string(formulaE) << "\n";
      if(is_sat(qdE))
        return false;
      
      n++;
    }
  }

  black::tribool is_realizable(automata_spec const& spec) {
    automata_spec covered = spec;

    for(auto &p : covered.inputs)
      p = cover(p);
    for(auto &p : covered.outputs)
      p = cover(p);
    for(auto &p : covered.spec.letters)
      p = cover(p);
    for(auto &p : covered.spec.variables)
      p = cover(p);

    return solve(covered, game_t::eventually{});
  }


}