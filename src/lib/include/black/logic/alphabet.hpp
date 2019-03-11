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

#ifndef BLACK_ALPHABET_HPP__
#define BLACK_ALPHABET_HPP__

#include <black/logic/formula.hpp>

#include <vector>
#include <memory>

namespace black::details {

  // This class manages an alphabet of propositional symbols and the allocation
  // of all the formulas built on top of such symbols.
  class alphabet
  {
  public:
    alphabet() = default;
    alphabet(alphabet const&) = delete;

    template<typename T, typename ...Args, REQUIRES(is_formula<T>)>
    T *create(Args &&...args) {
      auto ptr = std::make_unique<T>(*this, std::forward<Args>(args)...);
      T *ret = ptr.get();
      _formulas.push_back(std::move(ptr));

      return ret;
    }

    atom var() { return atom{create<atom_t>()}; }

  private:
    std::vector<std::unique_ptr<formula_t>> _formulas;
  };

}

#endif // BLACK_ALPHABET_HPP__
