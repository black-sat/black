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

namespace black::internal::new_api {
  //
  // relations and functions support creating the associated atom or application
  // (respectively) with a simple call-like syntax such as f(x, y).
  //
  // Here we declare the needed specialization of `storage_custom_members` for
  // the purpose. We have both a vararg version and one taking a range.
  // The implementation is in `interface.hpp`
  //
  template<typename Derived>
  struct relation_call_op {
    template<hierarchy Arg, hierarchy ...Args>
    auto operator()(Arg, Args ...) const;

    template<std::ranges::range R>
        requires hierarchy<std::ranges::range_value_t<R>>
    auto operator()(R const& v) const;
  };

  template<typename Derived>
  struct function_call_op {
    template<hierarchy Arg, hierarchy ...Args>
    auto operator()(Arg, Args ...) const;

    template<std::ranges::range R>
        requires hierarchy<std::ranges::range_value_t<R>>
    auto operator()(R const& v) const;
  };
}


#endif // BLACK_LOGIC_CUSTOM_HPP_
