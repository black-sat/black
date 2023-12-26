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

#ifndef BLACK_PYTHON_SUPPORT_HPP_
#define BLACK_PYTHON_SUPPORT_HPP_

#include <black/support.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <expected>

namespace black::python
{  
  using namespace black;
  namespace py = pybind11;

  //
  // A generic `to_python` function to use to return values of BLACK's types to 
  // Python
  //
  template<typename T>
  T&& to_python(T&& v) {
    return std::forward<T>(v);
  }

  //
  // When mapping functions that return `std::expected<T, E>`, the idiomatic
  // Python way is to throw the `E`.
  //
  template<typename T, typename E>
  T to_python(std::expected<T, E> const& e) {
    if(e.has_value())
      return e.value();
    throw e.error();
  }

  //
  // We do not export a `tribool` type to Python.
  // We dynamicaly return either a Bool or None
  //
  using tribool_py_t = std::variant<bool, py::none>;

  tribool_py_t to_python(support::tribool t);

  //
  // Any matchable type is returned as a variant of its alternatives
  //
  template<typename ...Args>
  struct tuple_to_variant { };
  
  template<typename ...Args>
  struct tuple_to_variant<std::tuple<Args...>> 
    : std::type_identity<std::variant<Args...>> { };

  template<typename ...Args>
  using tuple_to_variant_t = typename tuple_to_variant<Args...>::type;

  template<support::matchable T>
  using matchable_py_t = tuple_to_variant_t<typename T::alternatives>;
  
  template<support::matchable T>
  matchable_py_t<T> to_python(T&& v) {
    return std::forward<T>(v).match(
      [](auto x) -> matchable_py_t<T> {
        return x;
      }
    );
  }


  void register_support(py::module &m);

}

#endif // BLACK_PYTHON_SUPPORT_HPP_

