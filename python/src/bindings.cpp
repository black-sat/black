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

#include <pybind11/pybind11.h>

#include <black/support/tribool.hpp>
#include <black/support/identifier.hpp>

#include <black/python/support.hpp>
#include <black/python/hierarchy.hpp>

namespace pyblack {

  static void register_basic_types(py::module_ &m) {
    using tribool = black::tribool;
    using identifier = black::identifier;

    auto tribool_to_string = [](tribool b) {
      return b == true ? "True" : 
             b == false ? "False" : "tribool.undef";
    };

    py::class_<black::tribool::undef_t>(m, "tribool_undef_t")
      .def("__str__", [](tribool::undef_t) { return "tribool.undef"; })
      .def("__repr__", [](tribool::undef_t) { return "tribool.undef"; })
      .def("__bool__", [](tribool::undef_t) { return false; });

    py::class_<tribool>(m, "tribool")
      .def(py::init<bool>())
      .def(py::init<tribool::undef_t>())
      .def("__bool__", [](tribool b) { return b == true; })
      .def("__str__", tribool_to_string)
      .def("__repr__", tribool_to_string)
      .def_readonly_static("undef", &tribool::undef);
    
    py::class_<identifier>(m, "identifier")
      .def(py::init<std::string>());

    py::implicitly_convertible<std::string, identifier>();
  }

}

PYBIND11_MODULE(black, m) {

  using namespace pyblack;

  register_basic_types(m);

  py::class_<black::alphabet> alphabet{m, "alphabet"};
  alphabet.def(py::init<>());

  register_hierarchies(m, alphabet);

}

