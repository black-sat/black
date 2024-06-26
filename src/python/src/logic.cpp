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

namespace black::python {
  
  void register_logic(py::module &m) {
    using namespace black::logic;

    auto term_cls = register_ast_types<term>(m, 
      []<typename Node>(py::class_<Node> &node) {
        node.def("__call__", [](Node self, py::args args) {
          return atom(self, py::cast<std::vector<term>>(args));
        });

        node.def("__add__", [](Node self, term t) {
          return self + t;
        }, py::is_operator());

        node.def("__sub__", [](Node self, term t) {
          return self - t;
        }, py::is_operator());
        
        node.def("__neg__", [](Node self) {
          return -self;
        }, py::is_operator());
        
        node.def("__mul__", [](Node self, term t) {
          return self * t;
        }, py::is_operator());
        
        node.def("__truediv__", [](Node self, term t) {
          return self / t;
        }, py::is_operator());

        node.def("__lt__", [](Node self, term t) {
          return self < t;
        }, py::is_operator());
        
        node.def("__le__", [](Node self, term t) {
          return self <= t;
        }, py::is_operator());
        
        node.def("__gt__", [](Node self, term t) {
          return self > t;
        }, py::is_operator());
        
        node.def("__ge__", [](Node self, term t) {
          return self >= t;
        }, py::is_operator());
      }
    );

    py::class_<eq_wrapper_t<true>>(m, "TermEQWrapper")
      .def("__bool__", [](eq_wrapper_t<true> w) { return (bool)w; });
    py::class_<eq_wrapper_t<false>>(m, "TermNEWrapper")
      .def("__bool__", [](eq_wrapper_t<false> w) { return (bool)w; });

    term_cls.def(py::init([](eq_wrapper_t<true> w) { return (term)w; }));
    term_cls.def(py::init([](eq_wrapper_t<false> w) { return (term)w; }));

    py::implicitly_convertible<eq_wrapper_t<true>, term>();
    py::implicitly_convertible<eq_wrapper_t<false>, term>();

    m.def("Not", [](term t) {
      return negation(t);
    });

    m.def("And", [](py::args args) {
      return conjunction(py::cast<std::vector<term>>(args));
    });
    
    m.def("Or", [](py::args args) {
      return disjunction(py::cast<std::vector<term>>(args));
    });
  

  }

}