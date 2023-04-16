//
// Synthetico - Pure-past LTL synthesizer based on BLACK
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

#include <black/qbf/qbf.hpp>
#include <black/logic/cnf.hpp>
#include <black/logic/prettyprint.hpp>

#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iostream>

namespace black_internal::synth {

  using bformula = black::logic::formula<black::logic::propositional>;
  using qbformula = black::logic::formula<black::logic::QBF>;
  using quantifier_t = logic::qbf<logic::QBF>::type;

  struct prenex_qbf {
    std::vector<black::logic::qbf<black::logic::QBF>> blocks;
    bformula matrix;
  };

  static prenex_qbf extract_prenex(qbformula f) 
  {
    using namespace black::logic::fragments::QBF;

    std::vector<qbf> blocks;
    qbformula matrix = f;

    while(matrix.is<qbf>()) {
      auto q = *matrix.to<qbf>();
      blocks.push_back(q);
      matrix = q.matrix();
    }

    auto m = matrix.to<bformula>();
    black_assert(m.has_value());
    
    return prenex_qbf{blocks, *m};
  }

  [[maybe_unused]]
  static std::string to_string(black::cnf cnf) {
    std::stringstream str;

    for(black::clause cl : cnf.clauses) {
      str << " ∧\n";
      for(auto [sign, prop] : cl.literals) {
        if(sign)
          str << "  ∨ " << to_string(!prop) << "\n";
        else
          str << "  ∨ " << to_string(prop) << "\n";
      }
    }

    return str.str();
  }

  qdimacs clausify(qbformula f)
  {
    std::cerr << "f to clausify: " << black::to_string(f) << "\n";

    prenex_qbf qbformula = extract_prenex(f);

    std::cerr << "blocks: \n";
    for(auto q : qbformula.blocks) {
      if(q.node_type() == logic::qbf<logic::QBF>::type::thereis{})
        std::cerr << "thereis ";
      else
        std::cerr << "foreach ";
      for(auto v : q.variables()) {
        std::cerr << black::to_string(v) << " ";
      }
      std::cerr << "\n";
    }

    black::cnf cnf = black::to_cnf(qbformula.matrix);

    std::unordered_map<var_t, black::proposition> props;
    std::unordered_map<black::proposition, var_t> vars;

    // clauses
    std::vector<clause> clauses;

    var_t next_var = 1;
    for(auto cl : cnf.clauses) {
      std::vector<lit_t> literals;
      for(auto [sign, prop] : cl.literals) {
        var_t var;
        if(auto it = vars.find(prop); it != vars.end()) 
          var = it->second;
        else {
          var = next_var++;
          props.insert({var, prop});
          vars.insert({prop, var});
        }

        literals.push_back(sign ? -lit_t{var} : lit_t{var});
      }
      clauses.push_back(clause{literals});
    }

    std::vector<qdimacs_block> blocks;
    std::unordered_set<var_t> declared_vars;

    // quantifiers
    for(auto block : qbformula.blocks) {
      if(block.variables().empty())
        continue;
      
      std::vector<var_t> block_vars;
      for(auto p : block.variables()) {
        if(!vars.contains(p)) 
          continue;

        declared_vars.insert(vars[p]);
        block_vars.push_back(vars[p]);
      }

      if(block.node_type() == quantifier_t::thereis{}) {
        std::cerr << "existential block:\n";
        for(auto var : block_vars) {
          std::cerr << "- var: " << var << "\n";
        }
        blocks.push_back(qdimacs_block{qdimacs_block::existential, block_vars});
      } else {
        std::cerr << "universal block:\n";
        for(auto var : block_vars) {
          std::cerr << "- var: " << var << "\n";
        }
        blocks.push_back(qdimacs_block{qdimacs_block::universal, block_vars});
      }

    }

    // quantifiers for Tseitin variables
    std::unordered_set<var_t> last;
    for(auto cl : cnf.clauses) {
      for(auto [sign, prop] : cl.literals) {
        if(!vars.contains(prop))
          continue;
        var_t var = vars[prop];
        if(!declared_vars.contains(var)) {
          last.insert(var);
        }
      }
    }
    if(!last.empty())
      blocks.push_back(qdimacs_block{
        qdimacs_block::existential, std::vector(last.begin(), last.end())
      });

    return qdimacs{next_var - 1, blocks, clauses, props, vars};
  }

  std::string to_string(qdimacs qd) {
    std::stringstream str;

    // header
    str << "p cnf " << qd.n_vars << " " << qd.clauses.size() << "\n";

    // quantifiers
    for(auto block : qd.blocks) {
      if(block.type == qdimacs_block::existential)
        str << "e ";
      else
        str << "a ";

      for(var_t var : block.variables) 
        str << var << " ";
      str << "0\n";
    }

    // clauses
    for(clause cl : qd.clauses) {
      for(lit_t lit : cl.literals) {
        str << lit << " ";
      }
      str << "0\n";
    }

    return str.str();
  }

}
