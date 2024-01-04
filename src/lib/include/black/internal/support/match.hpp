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

#ifndef BLACK_SUPPORT_MATCH_HPP
#define BLACK_SUPPORT_MATCH_HPP

#include <variant>
#include <expected>

//
// Here we declare the infrastructure for pattern matching. The machinery is
// based on a specialization of `match_trait<T>` which has to be provided by
// any matchable type `T`.
//
// The specialization must provide:
// - a `cases` alias to a tuple of alternative types for the match
// - a `downcast<U>(T)` function that downcasts the `T` to an `std::optional<U>`
//   if possible, or std::nullopt otherwise (only when `U` is among `cases`)
//

namespace black::support {
  template<typename T>
  struct match_trait { };
}

namespace black::support::internal {

  template<typename T>
  concept matchable = requires (T t) {
    
    requires (std::tuple_size_v<typename match_trait<T>::cases> > 0);
    
    match_trait<T>::template downcast<
      typename std::tuple_element<0, typename match_trait<T>::cases>::type
    >(t);
  };

  template<matchable M, typename R, typename = typename match_trait<M>::cases>
  struct matcher_impl;
  
  template<matchable M, typename R, typename Case, typename ...Cases>
  struct matcher_impl<M, R, std::tuple<Case, Cases...>>
  {
    template<typename ...Handlers>
    R match(
      M const &m, std::source_location loc, Handlers ...handlers
    ) const {
      auto casted = match_trait<M>::template downcast<Case>(std::forward<M>(m));
      if(casted)
        return dispatch(handlers...)(*casted, loc);
      return matcher_impl<M, std::tuple<Cases...>>::match(m, loc, handlers...);
    }
  };
  
  template<matchable M, typename R, typename Case>
  struct matcher_impl<M, R, std::tuple<Case>>
  {
    template<typename ...Handlers>
    R match(
      M const &m, std::source_location loc, Handlers ...handlers
    ) const {
      auto casted = match_trait<M>::template downcast<Case>(std::forward<M>(m));
      black_assert(casted);

      return dispatch(handlers...)(*casted, loc);
    }
  };

  template<matchable M, typename = typename match_trait<M>::cases>
  class matcher_t;

  template<matchable M, typename ...Cases>
  class matcher_t<M, std::tuple<Cases...>>
  {
    template<typename ...Handlers>
    using return_t = std::common_type_t<
      decltype(dispatch(std::declval<Handlers>()...)(std::declval<Cases>()))...
    >;
  public:
    matcher_t(M const& m, std::source_location loc) : _m{m}, _loc{loc} { }
  
    template<typename ...Handlers>
    auto operator()(Handlers ...handlers) const {
      return matcher_impl<M, return_t<Handlers...>>::match(handlers...);
    }

  private:
    M const& _m;
    std::source_location _loc;
  };

  template<matchable H>
  matcher_t<H> match(
    H const& h, std::source_location loc = std::source_location::current()
  ) {
    return {h, loc};
  }

}

//
// Specialization of `match_trait` for standard types `std::variant`, and
// `std::expected`.
//
namespace black::support {

  template<typename ...Cases>
  struct match_trait<std::variant<Cases...>> {
    using cases = std::tuple<Cases...>;
    
    template<typename U>
      requires (std::is_same_v<U, Cases> || ...)
    static std::optional<U> downcast(std::variant<Cases...> const& v) {
      if(std::holds_alternative<U>(v))
        return {std::get<U>(v)};
      return {};
    }
  };
  
  template<typename T, typename E>
  struct match_trait<std::expected<T, E>> {
    using cases = std::tuple<T, E>;
    
    template<typename U>
      requires (std::is_same_v<U, T>)
    static std::optional<U> downcast(std::expected<T, E> const& v) {
      if(v.has_value())
        return {v.value()};
      return {};
    }
    
    template<typename U>
      requires (std::is_same_v<U, E>)
    static std::optional<U> downcast(std::expected<T, E> const& v) {
      if(!v.has_value())
        return {v.error()};
      return {};
    }
    
  };
  
  template<typename T>
  struct match_trait<std::optional<T>> {
    using cases = std::tuple<T, internal::otherwise>;
    
    template<typename U>
      requires (std::is_same_v<U, T>)
    static std::optional<U> downcast(std::optional<T> const& v) {
      if(v.has_value())
        return {v.value()};
      return {};
    }
    
    template<typename U>
      requires (std::is_same_v<U, internal::otherwise>)
    static std::optional<U> downcast(std::optional<T> const& v) {
      if(!v.has_value())
        return {internal::otherwise{}};
      return {};
    }
    
  };

}

template<>
struct std::common_type<
  ::black::support::internal::missing_case_t, 
  ::black::support::internal::missing_case_t
> : std::type_identity<::black::support::internal::missing_case_t> { };

template<typename T>
struct std::common_type<::black::support::internal::missing_case_t, T> : 
  std::type_identity<T> { };

template<typename T>
struct std::common_type<T, ::black::support::internal::missing_case_t> : 
  std::type_identity<T> { };

namespace black::support {
  using internal::match;
  using internal::matchable;
}

#endif // BLACK_SUPPORT_MATCH_HPP
