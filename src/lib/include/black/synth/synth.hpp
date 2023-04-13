//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#ifndef BLACK_SYNTH_HPP
#define BLACK_SYNTH_HPP

#include <black/logic/logic.hpp>
#include <black/logic/parser.hpp>
#include <black/support/tribool.hpp>
#include <black/support/range.hpp>

#include <black/automata/automaton.hpp>
#include <black/sdd/sdd.hpp>

#include <istream>

namespace black_internal::synth {

  struct ltlp_spec {
    std::vector<black::proposition> inputs;
    std::vector<black::proposition> outputs;

    black::logic::formula<black::logic::LTLP> spec;
  };

  std::optional<ltlp_spec> parse_ltlp_spec(
    logic::alphabet &sigma, std::istream &fstr, std::istream &partstr,
    parser::error_handler error
  );

  std::string to_string(ltlp_spec const& spec);

  struct automata_spec {
    std::vector<black::proposition> inputs;
    std::vector<black::proposition> outputs;

    automaton spec;
  };

  automata_spec to_automata_spec(black::sdd::manager *mgr, ltlp_spec spec);

  black::tribool is_realizable(automata_spec const&);

}

namespace black {
  using black_internal::synth::ltlp_spec;
  using black_internal::synth::automata_spec;
  using black_internal::synth::parse_ltlp_spec;
  using black_internal::synth::is_realizable;
}

#endif // BLACK_SYNTH_HPP
