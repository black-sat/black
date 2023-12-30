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

    term(term const&) = default;
    term(term &&) = default;
    
    term &operator=(term const&) = default;
    term &operator=(term &&) = default;

    bool operator==(term const&) const = default;

    template<typename Handle>
      requires requires { type_of_handle_v<Handle>; }
    std::optional<Handle> to() const;

    template<typename Handle>
      requires requires { type_of_handle_v<Handle>; }
    bool is() const {
      return to<Handle>().has_value();
    }    

    alphabet *sigma() const { return _sigma; }

    size_t hash() const { return support::hash(_node); }

  private:
    term(alphabet *sigma, std::shared_ptr<node_base_t const> n) 
      : _sigma{sigma}, _node{n} { }

    alphabet *_sigma;
    std::shared_ptr<node_base_t const> _node;

    template<term::type Type>
    friend struct concrete_term;
  };

  struct node_base_t {
    term::type type;

    bool operator==(node_base_t const&) const = default;
  };

  template<typename>
  struct term_types;

  template<typename T>
  using term_types_t = term_types<T>::type;

  template<typename T>
  struct has_sigma : std::false_type { };

  template<typename T>
  inline constexpr bool has_sigma_v = has_sigma<T>::value;

  template<typename T>
    requires requires(T v) { { v.sigma() } -> std::convertible_to<alphabet *>; }
  struct has_sigma<T> : std::true_type { };

  template<std::ranges::range R>
    requires has_sigma_v<std::ranges::range_value_t<R>>
  struct has_sigma<R> : std::true_type { };

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

  template<term::type Type, typename = node_data_t<Type>>
  struct is_leaf : std::false_type { };

  template<term::type Type, typename ...Fields>
    requires ((!has_sigma_v<Fields>) && ...)
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

    template<term::type, typename>
    friend struct concrete_term_specific_base;
  };

  template<typename Derived, term::type Type, typename Args = node_data_t<Type>>
  struct alphabet_named_factory_base { }; // to specialize

  template<typename Derived, typename Types = term_types_t<Derived>>
  struct alphabet_base;
  
  template<typename Derived, term::type ...Types>
  struct alphabet_base<
    Derived, std::tuple<std::integral_constant<term::type, Types>...>
  > : std::conditional_t<
        is_leaf_v<Types>,
        alphabet_named_factory_base<Derived, Types>,
        alphabet_factory_base<Types>
      >... { };

  struct concrete_term_common_base 
  {
  protected:
    concrete_term_common_base(
      alphabet *sigma, std::shared_ptr<node_base_t const> node
    ) : _sigma{sigma}, _node{node} { }

    alphabet *_sigma;
    std::shared_ptr<node_base_t const> _node;

    template<typename, term::type, typename>
    friend struct alphabet_named_factory_base;

    friend term;
  };

  template<typename Arg>
    requires has_sigma_v<Arg>
  alphabet *sigma(std::source_location loc, Arg const& arg) {
    if constexpr(std::ranges::range<Arg>) {
      black_assume(!empty(arg), loc, "vector argument cannot be empty");

      return begin(arg)->sigma();
    } else {
      return arg.sigma();
    }
  }

  template<typename Arg, typename ...Args>
  alphabet *sigma(
    std::source_location loc, Arg const& arg, Args const& ...args
  ) {
    if constexpr (has_sigma_v<Arg>)
      return sigma(loc, arg);
    else
      return sigma(loc, args...);
  }

  template<term::type Type, typename Args = node_data_t<Type>>
  struct concrete_term_specific_base;
  
  template<term::type Type, typename ...Args>
  struct concrete_term_specific_base<Type, std::tuple<Args...>>
    : concrete_term_common_base
  {
    using concrete_term_common_base::concrete_term_common_base;
  };

  template<term::type Type, typename ...Args>
    requires (!is_leaf_v<Type>)
  struct concrete_term_specific_base<Type, std::tuple<Args...>>
    : concrete_term_common_base
  {
    using concrete_term_common_base::concrete_term_common_base;

    concrete_term_specific_base(
      Args ...args, std::source_location loc = std::source_location::current()
    )
      : concrete_term_common_base{
          sigma(loc, args...), 
          sigma(loc, args...)->alphabet_factory_base<Type>::construct(
            std::move(args)...
          )
        } { }

  };

  template<term::type Type, typename Derived>
  struct concrete_term_fields; // to specialize

  template<term::type Type>
  struct concrete_term 
    : concrete_term_specific_base<Type>, 
      concrete_term_fields<Type, concrete_term<Type>> 
  {
    friend concrete_term_fields<Type, concrete_term<Type>>;

    using concrete_term_specific_base<Type>::concrete_term_specific_base;

    alphabet *sigma() const { return this->_sigma; }

    operator term() const {
      return term{this->_sigma, this->_node};
    }
  };

  template<typename Handle>
    requires requires { type_of_handle_v<Handle>; }
  std::optional<Handle> term::to() const {
    if(this->_node->type == type_of_handle_v<Handle>)
      return Handle{_sigma, _node};
    return {};
  }

}

template<black::logic::internal::term::type Type>
struct std::hash<black::logic::internal::node_t<Type>> {
  size_t operator()(black::logic::internal::node_t<Type> const& n) const {
    return black::support::hash(n.type, n.data);
  }
};

template<>
struct std::hash<black::logic::internal::term> {
  size_t operator()(black::logic::internal::term t) const {
    return t.hash();
  }
};

namespace black::logic {
  using support::match;
  using internal::term;
}

#endif // BLACK_LOGIC_SYNTAX_HPP