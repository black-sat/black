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
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include <black/support/tribool.hpp>
#include <black/support/identifier.hpp>
#include <black/logic/logic.hpp>
#include <black/logic/parser.hpp>
#include <black/solver/solver.hpp>
#include <black/solver/core.hpp>

#include <black/python/support.hpp>
#include <black/python/hierarchy.hpp>

namespace pyblack {

  static void register_basic_types(py::module &m) {
    using identifier = black::identifier;
    
    py::class_<identifier>(m, "identifier")
      .def(py::init<std::string>())
      .def("__str__", [](identifier self) {
        return to_string(self);
      });

    py::implicitly_convertible<std::string, identifier>();
  }

  static void register_api(py::module &m, py::class_<black::alphabet> &sigma) {
    namespace logic = black::logic;

    sigma.def("top", &black::alphabet::top);
    sigma.def("bottom", &black::alphabet::bottom);

    m.def("big_and", [](logic::alphabet &s, py::list args) {
      std::vector<black::formula> formulas;
      for(auto arg : args) {
        black::formula f = py::cast<black::formula>(arg);
        formulas.push_back(f);
      }
      return specialize(black::big_and(s, formulas));
    });

    m.def("big_or", [](logic::alphabet &s, py::list args) {
      std::vector<black::formula> formulas;
      for(auto arg : args) {
        black::formula f = py::cast<black::formula>(arg);
        formulas.push_back(f);
      }
      return specialize(black::big_or(s, formulas));
    });


    py::class_<logic::domain, logic::domain_ref> domain{m, "domain"};
    domain.def(py::init(
      [](std::vector<black::variable> elements) {
        return make_domain(std::move(elements));
      }
    ));
    domain.def("elements", &logic::domain::elements);

    py::class_<logic::scope> scope{m, "scope"};
    scope.def(py::init<logic::alphabet &>());
    scope.def(py::init<logic::alphabet &, logic::sort>());

    scope.def_property(
      "default_sort",
      &logic::scope::default_sort, &logic::scope::set_default_sort
    );

    py::enum_<logic::scope::rigid_t>{scope, "rigid_t"}
      .value("rigid", logic::scope::rigid)
      .value("non_rigid", logic::scope::non_rigid)
      .export_values();

    scope.def("declare", 
      [](logic::scope &self, 
         logic::variable v, logic::sort s, logic::scope::rigid_t r) {
        return self.declare(v, s, r);
      }, py::arg("v"), py::arg("s"), py::arg("r") = logic::scope::non_rigid
    );
    
    scope.def("declare", 
      [](logic::scope &self, 
         logic::relation rel, std::vector<logic::sort> sorts, 
         logic::scope::rigid_t r
        ) {
        return self.declare(rel, std::move(sorts), r);
      }, 
      py::arg("rel"), py::arg("sorts"), 
      py::arg("r") = logic::scope::non_rigid
    );
    
    scope.def("declare", 
      [](logic::scope &self, 
         logic::relation rel, std::vector<logic::var_decl> decls, 
         logic::scope::rigid_t r
        ) {
        return self.declare(rel, std::move(decls), r);
      }, 
      py::arg("rel"), py::arg("decls"), 
      py::arg("r") = logic::scope::non_rigid
    );
    
    scope.def("declare", 
      [](logic::scope &self, 
         logic::function fun, logic::sort sort, std::vector<logic::sort> sorts, 
         logic::scope::rigid_t r
        ) {
        return self.declare(fun, sort, std::move(sorts), r);
      }, 
      py::arg("fun"), py::arg("sort"), py::arg("sorts"), 
      py::arg("r") = logic::scope::non_rigid
    );
    
    scope.def("declare", 
      [](logic::scope &self, 
         logic::function fun, logic::sort sort, 
         std::vector<logic::var_decl> decls, 
         logic::scope::rigid_t r
        ) {
        return self.declare(fun, sort, std::move(decls), r);
      }, 
      py::arg("fun"), py::arg("sort"), py::arg("decls"), 
      py::arg("r") = logic::scope::non_rigid
    );

    scope.def("declare", 
      [](logic::scope &self, logic::named_sort s, logic::domain_ref d) {
        self.declare(s, d);
      }
    );
    
    scope.def("declare", 
      [](logic::scope &self, logic::sort_decl decl) {
        self.declare(decl);
      }
    );

    scope.def("sort", [](logic::scope &self, logic::variable v) {
      return self.sort(v);
    });
    scope.def("sort", [](logic::scope &self, logic::function fun) {
      return self.sort(fun);
    });
    scope.def("signature", [](logic::scope &self, logic::function fun) {
      return self.signature(fun);
    });
    scope.def("signature", [](logic::scope &self, logic::relation rel) {
      return self.signature(rel);
    });
    scope.def("domain", &logic::scope::domain);
    scope.def("type_check", [](logic::scope &self, black::formula f) {
      return self.type_check(f, [](auto) { });
    });
    
    scope.def("type_check", [](
      logic::scope &self, black::formula f, 
      std::function<void(std::string)> err
    ) {
      return self.type_check(f, err);
    });

    scope.def("is_rigid", [](logic::scope &self, logic::variable v) {
      return self.is_rigid(v);
    });
    scope.def("is_rigid", [](logic::scope &self, logic::relation rel) {
      return self.is_rigid(rel);
    });
    scope.def("is_rigid", [](logic::scope &self, logic::function fun) {
      return self.is_rigid(fun);
    });

    py::class_<black::solver> solver{m, "solver"};
    py::class_<black::model> model{m, "model"};
    
    solver.def(py::init<>());
    solver.def("solve", 
      [](black::solver &self, logic::scope const&xi, 
         black::formula f, bool finite, std::optional<size_t> timeout) 
      {
        std::optional<std::chrono::seconds> seconds;
        if(timeout)
          seconds = std::chrono::seconds(*timeout);

        bool semi_decision = !f.is<logic::formula<logic::LTLP>>();
        return tribool_to_py(
          self.solve(
            xi, f, finite, std::numeric_limits<size_t>::max(), 
            seconds, semi_decision
          )
        );
      },
      py::arg("xi"), py::arg("f"), py::arg("finite") = false,
      py::arg("timeout") = py::none{}
    );
    solver.def("is_valid", 
      [](black::solver &self, logic::scope const&xi, 
         black::formula f, bool finite, std::optional<size_t> timeout)
      {
        std::optional<std::chrono::seconds> seconds;
        if(timeout)
          seconds = std::chrono::seconds(*timeout);

        bool semi_decision = !f.is<logic::formula<logic::LTLP>>();
        return tribool_to_py(
          self.is_valid(
            xi, f, finite, std::numeric_limits<size_t>::max(), 
            seconds, semi_decision
          )
        );
      },
      py::arg("xi"), py::arg("f"), py::arg("finite") = false,
      py::arg("timeout") = py::none{}
    );

    solver.def_property_readonly("model", &black::solver::model);
    model.def_property_readonly("size", &black::model::size);
    model.def_property_readonly("loop", &black::model::loop);
    model.def("value", [](
      black::model const&self, logic::proposition p, size_t t) {
        return self.value(p, t);
      });
    model.def("value", [](
      black::model const&self, black::atom a, size_t t) {
        return self.value(a, t);
      });
    model.def("value", [](
      black::model const&self, black::equal e, size_t t) {
        return self.value(e, t);
      });
    model.def("value", [](
      black::model const&self, black::distinct e, size_t t) {
        return self.value(e, t);
      });
    model.def("value", [](
      black::model const&self, black::less_than c, size_t t) {
        return self.value(c, t);
      });
    model.def("value", [](
      black::model const&self, black::less_than_equal c, size_t t) {
        return self.value(c, t);
      });
    model.def("value", [](
      black::model const&self, black::greater_than c, size_t t) {
        return self.value(c, t);
      });
    model.def("value", [](
      black::model const&self, black::greater_than_equal c, size_t t) {
        return self.value(c, t);
      });

    m.def("unsat_core",
      [](logic::scope const& xi, black::formula f, bool finite) {
        return specialize(black::unsat_core(xi, f, finite));
      }, py::arg("xi"), py::arg("f"), py::arg("finite") = false
    );

    m.def("parse_formula", 
      [](logic::alphabet &sigma_, std::string const&s) { 
        return specialize(black::parse_formula(sigma_, s, [](auto) { }));
      }
    );
    
    m.def("parse_formula", 
      [](logic::alphabet &sigma_, std::string const&s,
         std::function<void(std::string)> err) { 
        return specialize(black::parse_formula(sigma_, s, err));
      }
    );
  }

}

PYBIND11_MODULE(black_sat, m) {

  using namespace pyblack;

  register_basic_types(m);

  py::class_<black::alphabet> alphabet{m, "alphabet"};
  alphabet.def(py::init<>());

  register_hierarchies(m, alphabet);

  register_api(m, alphabet);

}

