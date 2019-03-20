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

#ifndef BLACK_ALPHABET_STORAGE_HPP
#define BLACK_ALPHABET_STORAGE_HPP

#include <black/support/common.hpp>
#include <black/logic/formula.hpp>

#include <deque>
#include <map>

namespace black::details {

  class formula_storage
  {
  public:
    formula_storage()
      : _top(*this, true), _bottom(*this, false) {}

    template<typename F, typename ...Args>
    F *allocate_formula(Args&& ...args) {
      return allocate(_tag<F>{}, FWD(args)...);
    }

  protected:
    boolean_t            _top;
    boolean_t            _bottom;

  private:
    std::deque<atom_t>   _atoms;
    std::deque<unary_t>  _unaries;
    std::deque<binary_t> _binaries;

    using unary_key = std::tuple<unary_t::operator_type, formula_base const*>;
    using binary_key = std::tuple<binary_t::operator_type,
                                  formula_base const*,
                                  formula_base const*>;

    // TODO: switch to std::unordered_map by implementing std::hash on tuples
    std::map<std::string, atom_t*> _atoms_map;
    std::map<unary_key,  unary_t*> _unaries_map;
    std::map<binary_key, binary_t*> _binaries_map;

  private:
    template<typename>
    struct _tag {};

    atom_t *allocate(_tag<atom_t>, std::string const&name)
    {
      if(auto it = _atoms_map.find(name); it != _atoms_map.end())
        return it->second;

      atom_t *a = &_atoms.emplace_back(*this, name);
      _atoms_map.insert({name, a});

      return a;
    }

    unary_t *
    allocate(_tag<unary_t>, unary_t::operator_type t, formula_base const* arg)
    {
      if(auto it = _unaries_map.find({t, arg}); it != _unaries_map.end())
        return it->second;

      unary_t *f = &_unaries.emplace_back(*this, t, arg);
      _unaries_map.insert({{t, arg}, f});

      return f;
    }

    binary_t *
    allocate(_tag<binary_t>, binary_t::operator_type t,
             formula_base const* arg1, formula_base const* arg2)
    {
      auto it = _binaries_map.find({t, arg1, arg2});
      if(it != _binaries_map.end())
        return it->second;

      binary_t *f = &_binaries.emplace_back(*this, t, arg1, arg2);
      _binaries_map.insert({{t, arg1, arg2}, f});

      return f;
    }
  };

  // Out-of-line implementation from the handle class in formula.hpp,
  // to have a complete formula_storage type
  template<typename H, typename F>
  template<typename Arg, typename ...Args>
  F *handle_base<H, F>::allocate_formula(Arg&& arg, Args&& ...args)
  {
    black_assert(all_equal(&FWD(args)._formula->alphabet...));
    formula_storage &sigma =
      std::get<0>(std::tuple{FWD(args)...})._formula->alphabet;

    return sigma.allocate_formula<F>(FWD(arg), FWD(args)._formula...);
  }
}

#endif // BLACK_ALPHABET_STORAGE_HPP
