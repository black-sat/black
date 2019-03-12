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

#ifndef BLACK_ALPHABET_HPP
#define BLACK_ALPHABET_HPP

#include <black/support/common.hpp>
#include <black/logic/formula.hpp>

#include <unordered_map>
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

    atom var(std::string const&name)
    {
      if(auto it = _atoms.find(name); it != _atoms.end())
        return atom{it->second};

      return create<atom>(name);
    }

  private:
    std::unordered_map<std::string, atom_t *> _atoms;
    std::vector<std::unique_ptr<formula_t>> _formulas;

    template<typename>
    friend class unary_wrapper;

    template<typename>
    friend class binary_wrapper;

    template<
      typename W, typename F = unwrap<W>, typename ...Args,
      REQUIRES(is_formula<F>)
    >
    W create(Args &&...args) {
      auto ptr = std::make_unique<F>(*this, std::forward<Args>(args)...);
      F *raw = ptr.get();
      _formulas.push_back(std::move(ptr));

      if constexpr (std::is_same_v<F,atom_t>) {
        static_assert(sizeof...(Args) == 1);
        register_atom(raw, std::forward<Args>(args)...);
      }

      return W{raw};
    }

    template<typename ...Args>
    void register_atom(atom_t *a, std::string const& name) {
      _atoms.insert({name, a});
    }
  };

} // namespace black::details

namespace black {
  using details::alphabet;
}

#endif // BLACK_ALPHABET_HPP
