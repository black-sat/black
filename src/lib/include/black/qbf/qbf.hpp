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

#ifndef SYNTH_QBF_HPP
#define SYNTH_QBF_HPP

#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>

#include <unordered_map>

namespace black_internal::synth {

  namespace QBF = black::logic::fragments::QBF;

  using var_t = uint32_t;
  using lit_t = int64_t;

  struct clause {
    std::vector<lit_t> literals;
  };

  struct qdimacs_block {
    enum {
      existential,
      universal
    } type;

    std::vector<var_t> variables;
  };

  struct qdimacs {
    size_t n_vars;
    std::vector<qdimacs_block> blocks;
    std::vector<clause> clauses;

    std::unordered_map<var_t, black::proposition> props;
    std::unordered_map<black::proposition, var_t> vars;
  };

  qdimacs clausify(QBF::formula f);
  std::string to_string(qdimacs qd);

}

#endif // SYNTH_QBF_HPP
