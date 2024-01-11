//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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

#include <black/python/support.hpp>

#include <type_traits>

namespace black::python {

  tribool_py_t to_python(support::tribool t) {
    if(t == true)
      return true;
    if(t == false)
      return false;
    return py::none();
  }

  void register_support(py::module &m) {
    using namespace support;
    
    //
    // Exception types
    //
    py::register_exception<exception>(m, "Error", PyExc_RuntimeError);
    
    py::register_exception<bad_unreachable>(
      m, "BadUnreachableError", PyExc_RuntimeError
    );
    py::register_exception<bad_assert>(
      m, "BadAssertError", PyExc_RuntimeError
    );
    py::register_exception<bad_assumption>(
      m, "BadAssumptionError", PyExc_RuntimeError
    );
    py::register_exception<bad_pattern>(
      m, "BadPatternError", PyExc_RuntimeError
    );

    
    //
    // Error types for use with `std::expected<T,E>`
    // 
    // These are not constructible from the Python side because often it does 
    // not make sense, but we expose properties to query the errors
    //
    py::class_<syntax_error>(m, "SyntaxError")
      .def_readonly("filename", &syntax_error::filename)
      .def_readonly("line", &syntax_error::line)
      .def_readonly("column", &syntax_error::column)
      .def_readonly("message", &syntax_error::message);
    
    py::class_<type_error>(m, "TypeError")
      .def_readonly("message", &type_error::message);
    
    py::class_<backend_error>(m, "BackendError")
      .def_readonly("message", &backend_error::message);
    
    py::class_<io_error> io_error_cls(m, "IOError");
    io_error_cls.def_readonly("filename", &io_error::filename);
    io_error_cls.def_readonly("op", &io_error::op);
    io_error_cls.def_property_readonly("error", [](io_error const&self) {
      return strerror(self.err);
    });
    io_error_cls.def_readonly("message", &io_error::message);

    py::enum_<io_error::operation>(io_error_cls, "Operation")
      .value("Opening", io_error::opening)
      .value("Reading", io_error::reading)
      .value("Writing", io_error::writing);
  }

}
