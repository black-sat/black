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

#ifndef BLACK_LOGIC_SYNTAX_HPP
#define BLACK_LOGIC_SYNTAX_HPP

#include <memory>
#include <tuple>
#include <format>
#include <ranges>

#include <black/support.hpp>

namespace black::logic::internal {
  
  struct alphabet;
  struct node_base_t;

  template<typename H>
  struct type_of_handle;
  
  template<typename H>
  inline constexpr auto type_of_handle_v = type_of_handle<H>::value;

  class term {
  public:
    enum class type : uint8_t; // to specialize

    template<typename Handle>
      requires requires { type_of_handle_v<Handle>; }
    std::optional<Handle> to() const;

    template<typename Handle>
      requires requires { type_of_handle_v<Handle>; }
    bool is() const {
      return to<Handle>().has_value();
    }    

  protected:
    term(std::shared_ptr<node_base_t const> n) : _node{n} { }

    std::shared_ptr<node_base_t const> _node;

    template<term::type Type>
    friend struct term_handle_base;
  };

  struct node_base_t {
    term::type type;

    bool operator==(node_base_t const&) const = default;
  };

  template<typename>
  struct term_types;

  template<typename T>
  using term_types_t = term_types<T>::type;

  template<term::type Type>
  struct node_data; // to specialize

  template<term::type Type>
  using node_data_t = typename node_data<Type>::type;

  template<term::type Type>
  struct node_t : node_base_t { 
    template<typename ...Args>
    node_t(Args ...args)
      : node_base_t{Type}, data{std::move(args)...} { }

    bool operator==(node_t const&) const = default;

    node_data_t<Type> data;
  };

  template<term::type Type, typename Args = node_data_t<Type>>
  struct term_handle_ctor_base;
  
  template<term::type Type, typename ...Args>
  struct term_handle_ctor_base<Type, std::tuple<Args...>> : term
  {
    term_handle_ctor_base(Args ...args)
      : term{std::make_shared<node_t<Type> const>(std::move(args)...)} { }

  private:
    term_handle_ctor_base(std::shared_ptr<node_t<Type> const> ptr)
      : term{ptr} { }

    template<term::type, typename>
    friend struct alphabet_named_factory_base;
  };

  template<term::type Type, typename Derived>
  struct term_handle_fields; // to specialize

  template<term::type Type>
  struct term_handle_base 
    : term_handle_ctor_base<Type>, 
      term_handle_fields<Type, term_handle_base<Type>> 
  {
    friend term_handle_fields<Type, term_handle_base<Type>>;

    using term_handle_ctor_base<Type>::term_handle_ctor_base;
  };

  template<typename Handle>
    requires requires { type_of_handle_v<Handle>; }
  std::optional<Handle> term::to() const {
    if(this->_node->type == type_of_handle_v<Handle>)
      return {static_cast<Handle const&>(*this)};
    return {};
  }

  template<typename T>
  struct is_child : std::false_type { };

  template<>
  struct is_child<term> : std::true_type { };

  template<std::ranges::range R>
    requires std::is_same_v<std::ranges::range_value_t<R>, term>
  struct is_child<R> : std::true_type { };

  template<typename T>
  inline constexpr bool is_child_v = is_child<T>::value;

  template<term::type Type, typename = node_data_t<Type>>
  struct is_leaf : std::false_type { };

  template<term::type Type, typename ...Fields>
    requires ((!is_child_v<Fields>) && ...)
  struct is_leaf<Type, std::tuple<Fields...>> : std::true_type { };

  template<term::type Type>
  inline constexpr bool is_leaf_v = is_leaf<Type>::value;

  template<term::type Type, typename Args = node_data_t<Type>>
  class alphabet_factory_base { };

  template<term::type Type, typename ...Args>
  class alphabet_factory_base<Type, std::tuple<Args...>> 
  {
  protected:
    std::shared_ptr<node_t<Type> const>
    construct(Args ...args) {
      node_t<Type> node{std::move(args)...};

      if(auto ptr = _pool[node].lock(); ptr)
        return ptr;
      
      auto ptr = std::make_shared<node_t<Type> const>(std::move(node));
      _pool[*ptr] = ptr;

      return ptr;
    }

  private:
    support::map<node_t<Type>, std::weak_ptr<node_t<Type> const>> _pool;
  };

  template<term::type Type, typename Args = node_data_t<Type>>
  struct alphabet_named_factory_base { }; // to specialize

  template<typename T = void, typename Types = term_types_t<T>>
  struct alphabet_base;
  
  template<term::type ...Types>
  struct alphabet_base<
    void, std::tuple<std::integral_constant<term::type, Types>...>
  > : std::conditional_t<
        is_leaf_v<Types>,
        alphabet_named_factory_base<Types>,
        std::monostate
      >... { };

}

template<black::logic::internal::term::type Type>
struct std::hash<black::logic::internal::node_t<Type>> {
  size_t operator()(black::logic::internal::node_t<Type> const& n) const {
    return black::support::hash(n.type, n.data);
  }
};

namespace black::logic {
  using support::match;
  using internal::term;
}

#endif // BLACK_LOGIC_SYNTAX_HPP