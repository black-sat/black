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
#include <black/logic/formula_storage.hpp>

#include <unordered_map>
#include <deque>
#include <memory>

namespace black::details {

  class alphabet : protected formula_storage
  {
    template<typename, typename>
    friend struct handle_base;

  public:
    alphabet() = default;
    alphabet(alphabet const&) = delete; // Alphabets are non-copyable
    alphabet(alphabet &&) = default; // but movable

    struct boolean boolean(bool value) {
      return value ? top() : bottom();
    }

    struct boolean top() {
      return black::details::boolean{&_top};
    }
    struct boolean bottom() {
      return black::details::boolean{&_bottom};
    }

    atom var(std::string const&name) {
      return atom{allocate_formula<atom_t>(name)};
    }
  };
} // namespace black::details

// Names exported to the user
namespace black {
  using details::alphabet;
}


#endif // BLACK_ALPHABET_HPP
