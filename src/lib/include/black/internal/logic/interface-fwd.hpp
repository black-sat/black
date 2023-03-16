//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_LOGIC_CUSTOM_HPP_
#define BLACK_LOGIC_CUSTOM_HPP_

#include <memory>

namespace black::logic::internal {

  class var_decl;
  
  template<fragment>
  class application;
  
  template<fragment>
  class atom;

  //
  // relations and functions support creating the associated atom or application
  // (respectively) with a simple call-like syntax such as f(x, y).
  //
  // Here we declare the needed specialization of `storage_custom_members` for
  // the purpose. We have both a vararg version and one taking a range.
  // The implementation is in `interface.hpp`
  //
  template<storage_type S, typename Derived>
  struct call_op_interface 
  {
    template<typename Arg, typename ...Args>
    auto operator()(Arg, Args ...) const;

    template<std::ranges::range R>
      requires (
        hierarchy<std::ranges::range_value_t<R>> &&
        !std::is_same_v<std::ranges::range_value_t<R>, var_decl>
      )
    auto operator()(R const& v) const;

    template<std::ranges::range R>
        requires std::is_same_v<std::ranges::range_value_t<R>, var_decl>
    auto operator()(R const& v) const;
  };

  //
  // A variable `x` can be subscripted with a sort `s`, as in `x[s]`, to obtain 
  // the corresponding `var_decl`.
  //
  class sort;
  
  template<storage_type, typename Derived>
  struct variable_decl_op {
    var_decl operator[](sort s) const;
  };

  class domain;
  using domain_ref = std::shared_ptr<domain>;

}


#endif // BLACK_LOGIC_CUSTOM_HPP_
