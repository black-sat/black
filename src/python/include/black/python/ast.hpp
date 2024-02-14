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

#ifndef BLACK_PYTHON_AST_HPP
#define BLACK_PYTHON_AST_HPP

#include <black/python/support.hpp>

#include <black/ast/core>

#include <cctype>
#include <format>

namespace black::python 
{
  namespace core = black::ast::core;

  void register_ast(py::module &m);

  py::object to_python(black::ast::core::label const& label);

  template<typename TupleLike>
  void for_each_type(auto f) {
    [&]<size_t ...Is>(std::index_sequence<Is...>) {
      (f(std::type_identity<std::tuple_element_t<Is, TupleLike>>{}), ...);
    }(std::make_index_sequence<std::tuple_size_v<TupleLike>>{});
  }

  template<typename T, typename Args>
  struct constructor { };

  template<typename T, typename ...Args>
  struct constructor<T, std::tuple<Args...>> {
    auto operator()(Args ...args) const {
      return T(std::move(args)...);
    }
  };
  
  template<core::ast AST, core::ast_node_of<AST> Node, typename Args>
  struct factory_method { };

  template<core::ast_node Node>
  auto to_python(Node const& n) {
    return to_python(core::ast_of_t<Node>(n));
  }

  template<core::ast AST, core::ast_node_of<AST> Node, typename ...Args>
  struct factory_method<AST, Node, std::tuple<Args...>> {
    auto operator()(core::ast_factory_type_t<AST> &self, Args ...args) const {
      return to_python(self.template construct<Node>(std::move(args)...));
    }
  };

  template<core::ast AST>
  py::class_<AST> register_ast_types(py::module &m, auto f) 
  { 
    std::string ast_name = support::to_camel(core::ast_name_v<AST>);
    py::class_<AST> ast(m, ast_name.c_str());

    py::class_<core::ast_factory_type_t<AST>> factory(
      m, support::to_camel(core::ast_factory_name_v<AST>).c_str()
    );
    factory.def(py::init<>());
    
    register_range_iterable<AST>(m, std::format("{}Iterable", ast_name));

    for_each_type<core::ast_node_list_t<AST>>(
      [&]<typename Node>(std::type_identity<Node>) {
        py::class_<Node> node(
          m, support::to_camel(core::ast_node_name_v<AST, Node>).c_str());

        ast.def(py::init([](Node n) { return AST(n); }));
        py::implicitly_convertible<Node, AST>();
        
        if constexpr(core::ast_node_is_composite_v<AST, Node>) {
          node.def(
            py::init(
              constructor<Node, core::ast_node_field_types_t<AST, Node>>()
            )
          );
        } else {
          factory.def(core::ast_node_name_v<AST, Node>.data(), 
            factory_method<AST, Node, core::ast_node_field_types_t<AST, Node>>()
          );
        }

        node.doc() = core::ast_node_doc_v<AST, Node>;

        using field_t = core::ast_node_field_index_t<AST, Node>;

        py::tuple match_args(
          std::tuple_size_v<core::ast_node_field_list_t<AST, Node>>
        );

        size_t i = 0;
        for_each_type<core::ast_node_field_list_t<AST, Node>>(
          [&]<field_t Field>
            (std::type_identity<std::integral_constant<field_t, Field>>) 
          {
            std::string name{core::ast_node_field_name_v<AST, Node, Field>};
            node.def_property_readonly(
              name.c_str(),
              [](Node self) { return to_python(self.template field<Field>()); },
              py::keep_alive<0, 1>(),
              core::ast_node_field_doc_v<AST, Node, Field>.data()
            );
            match_args[i++] = name;
          }
        );
        node.attr("__match_args__") = match_args;

        node.def("__eq__", [](Node self, AST other) {
          return self == other;
        });
        
        node.def("__ne__", [](Node self, AST other) {
          return self != other;
        });

        f(node);
      }
    );

    return std::move(ast);
  }

}

#endif // BLACK_PYTHON_AST_HPP
