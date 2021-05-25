//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#include <black/logic/alphabet.hpp>

#include <deque>
#include <tsl/hopscotch_map.h>

namespace black::internal {
  
  struct alphabet::alphabet_impl
  {
    alphabet_impl(alphabet *sigma) : _sigma{sigma} {}

    alphabet            *_sigma;

    boolean_t            _top{true};
    boolean_t            _bottom{false};

    std::deque<atom_t>   _atoms;
    std::deque<unary_t>  _unaries;
    std::deque<binary_t> _binaries;

    using unary_key = std::tuple<unary::type, formula_base*>;
    using binary_key = std::tuple<binary::type,
                                  formula_base*,
                                  formula_base*>;

    tsl::hopscotch_map<any_hashable, atom_t*> _atoms_map;
    tsl::hopscotch_map<unary_key,   unary_t*> _unaries_map;
    tsl::hopscotch_map<binary_key, binary_t*> _binaries_map;
  };

  alphabet::alphabet()
    : _impl{std::make_unique<alphabet_impl>(this)} {}

  alphabet::~alphabet() = default;

  alphabet::alphabet(alphabet&&) = default;
  alphabet &alphabet::operator=(alphabet&&) = default;

  boolean alphabet::boolean(bool value) {
    return value ? top() : bottom();
  }

  boolean alphabet::top() {
    return internal::boolean{this, &_impl->_top};
  }

  boolean alphabet::bottom() {
    return internal::boolean{this, &_impl->_bottom};
  }

  formula alphabet::from_id(formula_id id) {
    return
    formula{this, reinterpret_cast<formula_base *>(static_cast<uintptr_t>(id))};
  }

  atom_t *alphabet::allocate_atom(any_hashable _label)
  {
    any_hashable label{FWD(_label)};

    if(auto it = _impl->_atoms_map.find(label); it != _impl->_atoms_map.end())
      return it->second;

    atom_t *a = &_impl->_atoms.emplace_back(label);
    _impl->_atoms_map.insert({label, a});

    return a;
  }

  unary_t *alphabet::allocate_unary(unary::type type, formula_base* arg)
  {
    auto it = _impl->_unaries_map.find({type, arg});
    if(it != _impl->_unaries_map.end())
      return it->second;

    unary_t *f =
      &_impl->_unaries.emplace_back(static_cast<formula_type>(type), arg);
    _impl->_unaries_map.insert({{type, arg}, f});

    return f;
  }

  binary_t *alphabet::allocate_binary(
    binary::type type, formula_base* arg1, formula_base* arg2
  ) {
    auto it = _impl->_binaries_map.find({type, arg1, arg2});
    if(it != _impl->_binaries_map.end())
      return it->second;

    binary_t *f =
      &_impl->_binaries.emplace_back(
        static_cast<formula_type>(type), arg1, arg2
      );
    _impl->_binaries_map.insert({{type, arg1, arg2}, f});

    return f;
  }

}
