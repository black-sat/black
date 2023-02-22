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

#include <black/python/hierarchy.hpp>
#include <black/python/support.hpp>

#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>

#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include <vector>
#include <tuple>
#include <variant>

namespace pyblack 
{
  template<typename T>
  struct clean_init_arg {
    using type = T;
  };

  template<internal::hierarchy_type H>
  struct clean_init_arg<internal::child_arg<H, syntax>>
    : internal::hierarchy_type_of<syntax, H> { };
  
  template<internal::hierarchy_type H>
  struct clean_init_arg<internal::children_arg<H, syntax>> {
    using type = std::vector<internal::hierarchy_type_of_t<syntax, H>>;
  };

  template<typename T>
  using clean_init_arg_t = typename clean_init_arg<T>::type;

  template<typename Tuple>
  struct init_aux;
  
  template<typename ...Args>
  struct init_aux<std::tuple<Args...>> {
    static auto init() {
      return py::init<clean_init_arg_t<Args>...>();
    }
  };

  template<black::hierarchy H, typename F>
  void def_bin_op_simple(
    py::class_<H> &class_, std::string name, F op
  ) {
    using base_t =  internal::hierarchy_type_of_t<syntax, H::hierarchy>;
    std::string opname = "__" + name + "__";

    class_.def(opname.c_str(), [&](H h, base_t t) {
      return specialize(op(h, t));
    }, py::is_operator());
  }


  template<black::hierarchy H, typename F>
  void def_bin_op(
    py::class_<H> &class_, std::string name, F op
  ) {
    std::string opname = "__" + name + "__";
    std::string ropname = "__r" + name + "__";

    def_bin_op_simple(class_, name, op);

    class_.def(opname.c_str(), [&](H h, int64_t x) {
      return specialize(op(h, x));
    }, py::is_operator());

    class_.def(opname.c_str(), [&](H h, double x) {
      return specialize(op(h, x));
    }, py::is_operator());
    
    class_.def(ropname.c_str(), [&](H h, int64_t x) {
      return specialize(op(h, x));
    }, py::is_operator());

    class_.def(ropname.c_str(), [&](H h, double x) {
      return specialize(op(h, x));
    }, py::is_operator());
  }

  template<black::hierarchy H, typename F>
  void def_unary_op(py::class_<H> &class_, std::string name, F op) {
    class_.def(("__" + name + "__").c_str(), [&](H h) {
      return specialize(op(h));
    }, py::is_operator());
  }

  template<typename T>
  void register_operators(T) { }

  template<internal::hierarchy H>
    requires (H::hierarchy == internal::hierarchy_type::term)
  void register_operators(py::class_<H> &class_) {
    def_bin_op(class_, "add", [](auto x, auto y) { return x + y; });
    def_bin_op(class_, "sub", [](auto x, auto y) { return x - y; });
    def_bin_op(class_, "mul", [](auto x, auto y) { return x * y; });
    def_bin_op(class_, "truediv", [](auto x, auto y) { return x / y; });
    def_unary_op(class_, "negate", [](auto x) { return -x; });
    def_bin_op(class_, "eq", [](auto x, auto y) { return x == y; });
    def_bin_op(class_, "ne", [](auto x, auto y) { return x != y; });
    def_bin_op(class_, "lt", [](auto x, auto y) { return x < y; });
    def_bin_op(class_, "le", [](auto x, auto y) { return x <= y; });
    def_bin_op(class_, "gt", [](auto x, auto y) { return x > y; });
    def_bin_op(class_, "ge", [](auto x, auto y) { return x >= y; });

    if constexpr(H::storage == internal::storage_type::variable) {
      class_.def("__getitem__", [&](H var, black::sort s) {
        return specialize(var[s]);
      });
    }
  }

  template<internal::hierarchy H>
    requires (H::hierarchy == internal::hierarchy_type::formula)
  void register_operators(py::class_<H> &class_) {
    def_unary_op(class_, "invert", std::logical_not<>{});
    def_bin_op_simple(class_, "and", std::logical_and<>{});
    def_bin_op_simple(class_, "or", std::logical_or<>{});
    def_bin_op_simple(class_, "eq", std::equal_to<>{});
    def_bin_op_simple(class_, "ne", std::not_equal_to<>{});
  }
  
  template<internal::hierarchy H>
    requires (H::storage == internal::storage_type::relation)
  void register_operators(py::class_<H> &class_) {
    class_.def("__call__", [](black::relation self, py::args args) {
      std::vector<black::term> terms;
      for(auto it = args.begin(); it != args.end(); ++it) {
        try {
          int64_t value = py::cast<int64_t>(*it);
          terms.push_back(black::constant(self.sigma()->integer(value)));
          continue;
        } catch(...) { }
        try {
          double value = py::cast<double>(*it);
          terms.push_back(black::constant(self.sigma()->real(value)));
          continue;
        } catch(...) { }

        terms.push_back(py::cast<black::term>(*it));
      }
      return black::atom(self, terms);
    });
  }
  
  template<internal::hierarchy H>
    requires (H::storage == internal::storage_type::function)
  void register_operators(py::class_<H> &class_) {
    class_.def("__call__", [](black::function self, py::args args) {
      std::vector<black::term> terms;
      for(auto it = args.begin(); it != args.end(); ++it) {
        terms.push_back(py::cast<black::term>(*it));
      }
      return black::application(self, terms);
    });
  }

  template<typename T>
  void register_api(py::module &, T) { }


  template<internal::hierarchy H>
    requires (H::hierarchy == internal::hierarchy_type::formula)
  void register_api(py::module &m, py::class_<H> &) {
    m.def("X", [](H h) { return specialize(X(h)); });
    m.def("wX", [](H h) { return specialize(wX(h)); });
    m.def("Y", [](H h) { return specialize(Y(h)); });
    m.def("Z", [](H h) { return specialize(Z(h)); });
    m.def("G", [](H h) { return specialize(G(h)); });
    m.def("F", [](H h) { return specialize(F(h)); });
    m.def("O", [](H h) { return specialize(O(h)); });
    m.def("H", [](H h) { return specialize(H(h)); });
    m.def("implies", [](H h1, H h2) { return specialize(implies(h1, h2)); });
    m.def("U", [](H h1, H h2) { return specialize(U(h1, h2)); });
    m.def("R", [](H h1, H h2) { return specialize(R(h1, h2)); });
    m.def("W", [](H h1, H h2) { return specialize(W(h1, h2)); });
    m.def("M", [](H h1, H h2) { return specialize(M(h1, h2)); });
    m.def("S", [](H h1, H h2) { return specialize(S(h1, h2)); });
    m.def("T", [](H h1, H h2) { return specialize(T(h1, h2)); });
  }

  template<black::syntax_element Element, typename HClass>
  void register_hierarchy_element(
    std::string_view name, py::module &m, 
    py::class_<black::alphabet> &alphabet, HClass &hclass
  ) {
    static constexpr auto element = Element;
    static constexpr auto storage = internal::storage_of_element(element);
    static constexpr auto hierarchy = internal::hierarchy_of_storage(storage);

    using element_type = internal::element_type_of_t<syntax, element>;
    // using storage_type = internal::storage_type_of_t<syntax, storage>;
    using hierarchy_type = internal::hierarchy_type_of_t<syntax, hierarchy>;

    py::class_<element_type> class_{m, name.data()};
    hclass.def(py::init<element_type>());

    py::implicitly_convertible<element_type, hierarchy_type>();

    if constexpr(!internal::storage_has_children(storage))
      alphabet.def(
        name.data(), 
        &internal::alphabet_ctor_base<
          element, internal::alphabet_base
        >::construct
      );

    if constexpr(internal::storage_has_children(storage))
      class_.def(
        init_aux<internal::storage_alloc_args_t<syntax, storage>>::init()
      );


    auto declare_fields = [&]<size_t ...I>(std::index_sequence<I...>) {
      (class_.def_property_readonly(
        internal::storage_fields_v<storage>[I].data(), 
        [](element_type self) {
          return specialize(black_internal::logic::get<I>(self));
        }
      ), ...);
    };

    declare_fields(std::make_index_sequence<std::tuple_size_v<element_type>>{});

    auto repr = [](element_type e) {
      return to_string(hierarchy_type{e});
    };

    class_.def("__str__", repr);
    //class_.def("__repr__", repr);

    register_operators(class_);
  }

  template<internal::hierarchy_type Hierarchy>
  void register_hierarchy(
    std::string_view name, py::module &m, py::class_<black::alphabet> &alphabet
  ) {
    using hierarchy_type = internal::hierarchy_type_of_t<syntax, Hierarchy>;
    py::class_<hierarchy_type> class_{m, name.data()};

    auto declare = [&]<black::syntax_element E>(black::syntax_list<E>) {
      if constexpr(Hierarchy == hierarchy_of_storage(storage_of_element(E)))
        register_hierarchy_element<E>(to_string(E), m, alphabet, class_);
    };

    auto declare_all = 
      [&]<black::syntax_element ...E>(black::syntax_list<E...>) {
        (declare(black::syntax_list<E>{}), ...);
      };

    declare_all(internal::universal_fragment_t::list{});

    register_api(m, class_);
  }
    
  template<internal::hierarchy_type H>
  bool hierarchy_registered_v = false;

  void register_hierarchies(
    py::module &m, py::class_<black::alphabet> &alphabet
  ) {

    auto declare = [&]<black::syntax_element E>(black::syntax_list<E>) {
      static constexpr auto H = hierarchy_of_storage(storage_of_element(E));
      if(!hierarchy_registered_v<H>) {
        register_hierarchy<H>(to_string(H), m, alphabet);
        hierarchy_registered_v<H> = true;
      }
    };

    auto declare_all = 
      [&]<black::syntax_element ...E>(black::syntax_list<E...>) {
        (declare(black::syntax_list<E>{}), ...);
      };

    declare_all(internal::universal_fragment_t::list{});
  }

}
