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

#ifndef BLACK_AST_CORE_HPP
#define BLACK_AST_CORE_HPP

#include <string_view>

//
// Public interface for reflection of AST nodes.
//
// See reflect.hpp to see how these are implemented from the expansion 
// of defs.hpp
//
namespace black::ast::reflect {

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
  struct ast_node_list { };
  
  template<ast AST>
  using ast_node_list_t = typename ast_node_list<AST>::type;

  template<ast AST>
  struct ast_node_index { };

  template<ast AST>
  using ast_node_index_t = typename ast_node_index<AST>::type;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_index_of { };
  
  template<ast AST, ast_node_of<AST> Node>
  inline constexpr auto ast_node_index_of_v = 
    ast_node_index_of<AST, Node>::value;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_field_index { };
  
  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_index_t = typename ast_node_field_index<AST, Node>::type;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_field_list { };

  template<ast AST, ast_node_of<AST> Node>
  using ast_node_field_list_t = typename ast_node_field_list<AST, Node>::type;

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  struct ast_node_field_type { };

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  using ast_node_field_type_t = 
    typename ast_node_field_type<AST, Node, Field>::type;

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  struct ast_node_field_name { };
  
  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field
  >
  inline constexpr auto ast_node_field_name_v = 
    ast_node_field_name<AST, Node, Field>::value;

  template<
    ast AST, ast_node_of<AST> Node, 
    typename = ast_node_field_list_t<AST, Node>
  >
  struct ast_node_field_types { };

  template<
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> ...Fields
  >
  struct ast_node_field_types<
    AST, Node, 
    std::tuple<
      std::integral_constant<ast_node_field_index_t<AST, Node>, Fields>...
    >
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
    ast AST, ast_node_of<AST> Node, ast_node_field_index_t<AST, Node> Field,
    auto Member
  >
  struct ast_node_field_member_base { };

  template<typename Derived, ast AST>
  struct ast_factory_named_member { };

}

//
// Implementation of AST nodes
//
namespace black::ast::internal {

  using namespace reflect;

  template<ast AST>
  struct ast_impl_base {
    const ast_node_index_t<AST> index;
  };

  template<ast AST>
  using ast_impl_ref = std::shared_ptr<ast_impl_base<AST> const>;

  template<ast AST, ast_node_of<AST> Node>
  struct ast_impl : ast_impl_base<AST> {
    
    ast_impl(ast_node_field_types_t<AST, Node> _data) 
      : ast_impl_base<AST>{ast_node_index_of_v<AST, Node>}, data{_data} { }
    
    const ast_node_field_types_t<AST, Node> data;
  };

  template<ast AST>
  class ast_factory { };

  template<ast AST>
  class node_holder {
    public:
      size_t hash() const { return support::hash(_factory, _impl); }
    
    protected:
      ast_factory<AST> *factory() const { return _factory; }
      ast_impl_ref<AST> impl() const { return _impl; }

      node_holder(ast_factory<AST> *f, ast_impl_ref<AST> i)
        : _factory{f}, _impl{i} { }

      ast_factory<AST> *_factory;
      ast_impl_ref<AST> _impl;
  };

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_base;

  template<ast AST>
  struct ast_base 
    : node_holder<AST>, 
      ast_factory_named_member<ast_base<AST>, AST>
  {
    using node_holder<AST>::node_holder;

    ast_base() = delete;
    ast_base(ast_base const&) = default;
    ast_base(ast_base &&) = default;
    
    template<ast_node_of<AST> Node>
    ast_base(ast_node_base<AST, Node> n) 
      : node_holder<AST>(n._factory, n._impl) { }

    ast_base &operator=(ast_base const&) = default;
    ast_base &operator=(ast_base &&) = default;

    bool operator==(ast_base const&) const = default;

    template<ast A, ast_node_of<A>>
      friend struct ast_node_base;
  };

  template<ast AST, ast_node_of<AST> Node>
  struct ast_node_base 
    : node_holder<AST>, 
      ast_factory_named_member<ast_node_base<AST, Node>, AST>
  {
    using node_holder<AST>::node_holder;

    ast_node_base() = delete;

    ast_node_base(ast_node_base const&) = default;
    ast_node_base(ast_node_base &&) = default;
    
    ast_node_base &operator=(ast_node_base const&) = default;
    ast_node_base &operator=(ast_node_base &&) = default;

    bool operator==(ast_node_base const&) const = default;

    template<ast>
      friend struct ast_base;
  };

}

#endif // BLACK_AST_CORE_HPP
