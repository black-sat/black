//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <black/python/logic.hpp>
#include <black/python/support.hpp>
#include <black/python/ast.hpp>

#include <black/support>

#include <iostream>

namespace black::python {

  struct identifiable_wrapper {
    py::object obj;

    bool operator==(identifiable_wrapper const&w) const {
      return obj.is(w.obj);
    }

    size_t hash() const {
      return (int64_t)pybind11::cast<pybind11::int_>(obj.attr("__hash__")());
    }
  };

}

template<> 
struct std::formatter<black::python::identifiable_wrapper> 
  : std::formatter<string_view>
{
  template <typename FormatContext>
  auto 
  format(black::python::identifiable_wrapper const& w, FormatContext& ctx) 
  const 
  {
    return formatter<string_view>::format(
      pybind11::cast<std::string>(w.obj.attr("__str__")()), 
      ctx
    );
  }
};

namespace black::python {

  py::object to_python(black::ast::core::label const& label) {
    if(auto w = label.get<identifiable_wrapper>(); w)
      return w->obj;
    if(auto w = label.get<std::string>(); w)
      return py::cast(*w);
    return py::cast(label);
  }

  void register_ast(py::module &m) {
    using black::ast::core::label;

    py::class_<label>(m, "Label")
      .def(py::init([](std::string const&s) {
        return label{s};
      }))
      .def(py::init([](py::object o) {
        return label{identifiable_wrapper{o}};
      }))
      .def("__eq__", [](label self, label other) {
        return self == other;
      }, py::is_operator())
      .def("__str__", [](label self) {
        return std::format("{}", self);
      })
      .def("__hash__", [](label self) {
        return support::hash(self);
      })
      .def_property_readonly("value", [](label const& self) {
        return to_python(self);
      });

    py::implicitly_convertible<py::object, label>();

  }

}