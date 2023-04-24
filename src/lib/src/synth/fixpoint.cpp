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

#include <black/synth/fixpoint.hpp>

namespace black_internal::synth {

  namespace bdd = black::bdd;

  namespace { 

    struct encoder {

      encoder(automata_spec const& _spec)
        : spec{_spec}, aut{spec.spec}, sigma{*aut.manager->sigma()},
        coverset{begin(aut.variables), end(aut.variables)} { 
          // aut.variables = cover(aut.variables);
          // aut.init = cover(aut.init);
          // aut.trans = cover(aut.trans);
          // aut.finals = cover(aut.finals);
        }

      // black::proposition cover(black::proposition);
      // std::vector<black::proposition> cover(std::vector<black::proposition>);
      // bdd::node cover(bdd::node);

      [[maybe_unused]]
      bdd::node fixpoint(std::optional<bdd::node> previous = {});

      [[maybe_unused]]
      bdd::node test(bdd::node fp, bdd::node prevfp);

      [[maybe_unused]]
      bdd::node win(bdd::node fp);

      automata_spec spec;
      automaton &aut;
      logic::alphabet &sigma;
      std::unordered_set<black::proposition> coverset;
    };

    // black::proposition encoder::cover(black::proposition p) {
    //   tag_t tag = p.name().to<tag_t>().value_or(tag_t{p});
    //   if(tag.primes == 0) {
    //     if(coverset.contains(p))
    //       return p.sigma()->proposition(p);
    //     return p;
    //   }
    //   size_t primes = tag.primes;
    //   tag.primes = 0;
    //   return prime(cover(p.sigma()->proposition(tag)), primes);
    // }

    // std::vector<black::proposition> 
    // encoder::cover(std::vector<black::proposition> ps) {
    //   for(auto &p : ps)
    //     p = cover(p);

    //   return ps;
    // }

    // bdd::node encoder::cover(bdd::node f) {
    //   return f.rename([&](black::proposition p) {
    //     return cover(p);
    //   });
    // }

    bdd::node encoder::fixpoint(std::optional<bdd::node> base) {
      if(!base)
        return aut.finals;

      bdd::node previous = *base;
      return previous ||
        forall(spec.inputs,
          exists(spec.outputs,
            exists(primed() * aut.variables,
              aut.trans && previous[aut.variables / primed()]
            )
          )
        );
    }

    bdd::node encoder::test(bdd::node fp, bdd::node prevfp) {
      return forall(aut.variables, implies(fp, prevfp));
    }

    bdd::node encoder::win(bdd::node fp) {
      return exists(aut.variables, fp && aut.init);
    }

  }

  static black::tribool solve(automata_spec const& spec) { 
    encoder enc{spec};

    size_t k = 0;

    std::cerr << "Solving the game (fixpoint)... " << std::flush;

    bdd::node prevfp = enc.fixpoint();
    bdd::node fp = enc.fixpoint(prevfp);
    while(!enc.test(fp, prevfp).is_one()) {
      if(k == 0)
        std::cerr << "k = " << std::flush;
      else 
        std::cerr << ", " << std::flush;
      std::cerr << k << std::flush;

      prevfp = fp;
      fp = enc.fixpoint(fp);

      ++k;
    }
    std::cerr << (k > 1 ? ", " : "") << "done!\n";

    return enc.win(fp).is_one();
  }

  black::tribool is_realizable(automata_spec const& spec)
  {
    return solve(spec);
  }

}