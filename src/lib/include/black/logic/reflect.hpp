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

#ifndef BLACK_LOGIC_REFLECT_HPP
#define BLACK_LOGIC_REFLECT_HPP

#include <string_view>

namespace black::reflect {

  template<typename AST>
  struct is_ast : std::false_type { };
  
  template<typename AST>
  inline constexpr bool is_ast_v = is_ast<AST>::value;

  template<typename AST>
  concept ast = is_ast_v<AST>;
  
  template<typename Node, ast AST>
  struct is_ast_node_of : std::false_type { };
  
  template<typename Node, ast AST>
  inline constexpr bool is_ast_node_of_v = is_ast_node_of<Node, AST>::value;

  template<typename Node, typename AST>
  concept ast_node_of = ast<AST> && is_ast_node_of_v<Node, AST>;

  template<ast AST>
  struct ast_nodes { };
  
  template<ast AST>
  using ast_nodes_t = typename ast_nodes<AST>::type;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_field { };
  
  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_t = typename ast_node_field<AST, Node>::type;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_field_list { };

  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_list_t = typename ast_node_field_list<AST, Node>::type;

  template<ast AST, ast_node_of<AST> Node, ast_node_field_t<AST, Node> Field>
  struct ast_node_field_type { };

  template<ast AST, ast_node_of<AST> Node, ast_node_field_t<AST, Node> Field>
  using ast_node_field_type_t = 
    typename ast_node_field_type<AST, Node, Field>::type;

  template<ast AST, ast_node_of<AST> Node, ast_node_field_t<AST, Node> Field>
  struct ast_node_field_name { };
  
  template<ast AST, ast_node_of<AST> Node, ast_node_field_t<AST, Node> Field>
  inline constexpr auto ast_node_field_name_v = 
    ast_node_field_name<AST, Node, Field>::value;

  template<
    ast AST, ast_node_of<AST> Node, 
    typename = ast_node_field_list_t<AST, Node>
  >
  struct ast_node_field_types { };

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_t<AST, Node> ...Fields
  >
  struct ast_node_field_types<
    AST, Node, 
    std::tuple<std::integral_constant<ast_node_field_t<AST, Node>, Fields>...>
  > : std::type_identity<
        std::tuple<ast_node_field_type_t<AST, Node, Fields>...>
      > { };

  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_types_t = typename ast_node_field_types<AST, Node>::type;

  template<
    typename Derived,
    ast AST, ast_node_of<AST> Node,
    auto Member
  >
  struct ast_node_member_base { };
  
  template<
    typename Derived,
    ast AST, ast_node_of<AST> Node, ast_node_field_t<AST, Node> Field,
    auto Member
  >
  struct ast_node_field_member_base { };

}

//
// implementation of the traits above through x-pattern expansions 
//

namespace black {

  template<int Dummy, typename ...Args>
  struct tuple_cpp : std::type_identity<std::tuple<Args...>> { };

  #define declare_ast(NS, AST) \
    namespace NS { \
      struct AST; \
    }
  #include <black/logic/defs.hpp>

  #define declare_ast_node(NS, AST, Node) \
    namespace NS { \
      struct Node; \
    }
  #include <black/logic/defs.hpp>

}

namespace black::reflect {
  
  #define declare_ast(NS, AST) \
    template<> \
    struct is_ast<NS::AST> : std::true_type { };
  #include <black/logic/defs.hpp>
  
  #define declare_ast_node(NS, AST, Node) \
    template<> \
    struct is_ast_node_of<NS::Node, NS::AST> : std::true_type { };
  #include <black/logic/defs.hpp>

  #define declare_ast(NS, AST) \
    template<> \
    struct ast_nodes<NS::AST> : tuple_cpp<0

  #define declare_ast_node(NS, AST, Node) \
      , NS::Node

  #define end_ast(NS, AST) \
    > { };

  #include <black/logic/defs.hpp>

  #define declare_ast_node(NS, AST, Node) \
    template<> \
    struct ast_node_field<NS::AST, NS::Node> { \
      enum class type : size_t {

  #define declare_field(NS, AST, Node, Field, Type) \
        Field,

  #define end_ast_node(NS, AST, Node) \
      }; \
    };

  #include <black/logic/defs.hpp>


  #define declare_ast_node(NS, AST, Node) \
    template<> \
    struct ast_node_field_list<NS::AST, NS::Node> : tuple_cpp<0

  #define declare_field(NS, AST, Node, Field, Type) \
      , std::integral_constant< \
          ast_node_field_t<NS::AST, NS::Node>, \
          ast_node_field_t<NS::AST, NS::Node>::Field \
        >
  
  #define end_ast_node(NS, AST, Node) \
    > { };

  #include <black/logic/defs.hpp>

  #define declare_field(NS, AST, Node, Field, Type) \
    template<> \
    struct ast_node_field_type< \
      NS::AST, NS::Node, ast_node_field_t<NS::AST, NS::Node>::Field \
    > : std::type_identity<Type> { };

  #include <black/logic/defs.hpp>

  #define declare_field(NS, AST, Node, Field, Type) \
    template<> \
    struct ast_node_field_name< \
      NS::AST, NS::Node, ast_node_field_t<NS::AST, NS::Node>::Field \
    > { \
      static constexpr std::string_view value = #Field; \
    };

  #include <black/logic/defs.hpp>

  #define declare_ast_node(NS, AST, Node) \
    template< \
      typename Derived, \
      typename Base, typename R, typename ...Args, \
      R (Base::*Ptr)(Args...) \
    > \
    struct ast_node_member_base<Derived, NS::AST, NS::Node, Ptr> { \
      R Node(Args const& ...args) { \
        return (static_cast<Derived *>(this)->*Ptr)(args...); \
      } \
    };

  #include <black/logic/defs.hpp>

  #define declare_field(NS, AST, Node, Field, Type) \
    template< \
      typename Derived, \
      typename Base, typename R, \
      R (Base::*Ptr)() const \
    > \
    struct ast_node_field_member_base< \
      Derived, \
      NS::AST, NS::Node, ast_node_field_t<NS::AST, NS::Node>::Field, Ptr \
    > { \
      R Field() const { \
        return (static_cast<Derived const*>(this)->*Ptr)(); \
      } \
    };

  #include <black/logic/defs.hpp>

} // namespace black::reflect

#endif // BLACK_LOGIC_REFLECT_HPP
