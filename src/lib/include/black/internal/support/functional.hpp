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

#ifndef BLACK_SUPPORT_FUNCTIONAL_HPP
#define BLACK_SUPPORT_FUNCTIONAL_HPP

#include <variant>
#include <expected>
#include <source_location>

namespace black::support::internal {

  //
  // unpacking() combinator
  //
  template<typename F, typename Arg, size_t ...I>
  auto do_unpack(F f, Arg arg, std::index_sequence<I...>) 
    -> decltype(std::invoke(f, arg, get<I>(arg)...))
  {
    return std::invoke(f, arg, get<I>(arg)...);
  }

  struct no_unpack_t { };

  template<size_t N, typename F, typename Arg>
  auto unpackN(F f, Arg arg) {
    if constexpr (requires{ do_unpack(f, arg, std::make_index_sequence<N>{}); })
      return do_unpack(f, arg, std::make_index_sequence<N>{});
    else if constexpr (N > 0)
      return unpackN<N - 1>(f, arg);
    else 
      return no_unpack_t{};
  }

  template<typename F, typename Arg>
  auto unpack(F f, Arg arg)
  {
    return unpackN<0>(f, arg);
  }
  
  template<typename F, typename Arg>
    requires requires { std::tuple_size<Arg>::value; }
  auto unpack(F f, Arg arg) {
    return unpackN<std::tuple_size_v<Arg>>(f, arg);
  }

  template<typename Arg, typename F>
  concept can_unpack_over = requires(Arg arg, F f) {
    requires (!std::same_as<decltype(unpack(f, arg)), no_unpack_t>);
  };

  template<typename F>
  auto unpacking(F f) {
    return [=](can_unpack_over<F> auto arg) {
      return unpack(f, arg);
    };
  }

  //
  // dispatch combinator
  //
  struct no_dispatch_t { };

  template<typename Arg, typename F, typename ...Fs>
  auto dispatch(Arg arg, F f, Fs ...fs) 
  {
    if constexpr (requires { std::invoke(f, arg); })
      return std::invoke(f, arg);
    else if constexpr (sizeof...(Fs) > 0)
      return dispatch(arg, fs...);
    else
      return no_dispatch_t{};
  }

  template<typename Arg, typename ...Fs>
  concept can_dispatch_over = requires(Arg arg, Fs ...fs) {
    requires (!std::same_as<decltype(dispatch(arg, fs...)), no_dispatch_t>);
  };

  template<typename ...Fs>
  auto dispatching(Fs ...fs) {
    return [=](can_dispatch_over<Fs...> auto arg) {
      return dispatch(arg, fs...);
    };
  }

  struct otherwise { 
    otherwise() = default;
    otherwise(auto const&) { }
  };

}

namespace black::support {
  template<typename T>
  struct match_cases { };

  template<typename T>
  using match_cases_t = typename match_cases<T>::type;

  template<typename From, typename To>
  struct match_downcast { };

  template<typename T>
  concept matchable = requires (T t) 
  {
    requires (std::tuple_size_v<typename match_cases<T>::type> > 0);
    
    match_downcast<
      T, typename std::tuple_element<0, typename match_cases<T>::type>::type
    >::downcast(t);
  };
}

namespace black::support {

  template<typename ...Args>
  struct common_result { };
  
  template<typename ...Args>
  using common_result_t = typename common_result<Args...>::type;

  template<typename Arg1, std::common_with<Arg1> Arg2>
  struct common_result<Arg1, Arg2> : std::common_type<Arg1, Arg2> { };

  template<typename Arg1, typename Arg2>
    requires (!std::common_with<Arg1, Arg2> && std::convertible_to<Arg2, Arg1>)
  struct common_result<Arg1, Arg2> : std::type_identity<Arg1> { };
  
  template<typename Arg1, typename Arg2>
    requires (
      !std::common_with<Arg1, Arg2> && 
      !std::convertible_to<Arg2, Arg1> &&
      std::convertible_to<Arg1, Arg2>
    )
  struct common_result<Arg1, Arg2> : std::type_identity<Arg2> { };

  template<typename Arg, typename ...Args>
    requires (sizeof...(Args) > 1)
  struct common_result<Arg, Args...>
    : common_result<Arg, common_result_t<Args...>> { };

}

namespace black::support::internal {

  template<typename F, typename Cases>
  struct visit_result_ { };
  
  template<typename F, typename ...Cases>
  struct visit_result_<F, std::tuple<Cases...>> : common_result<
    decltype(std::declval<F>()(std::declval<Cases>()))...
  > { };

  template<typename F, matchable M>
  struct visit_result : visit_result_<F, match_cases_t<M>> { };

  template<typename F, typename Cases>
  using visit_result_t = typename visit_result<F, Cases>::type;

  template<typename M, typename F>
  concept can_visit_over = matchable<M> && requires {
    typename visit_result<F, M>::type;
  };

  template<matchable M, typename F, size_t I, size_t ...Is>
  visit_result_t<F, M> visit(M m, F f, std::index_sequence<I, Is...>)
  {
    using T = std::tuple_element_t<I, match_cases_t<M>>;
    auto casted = match_downcast<M, T>::downcast(m);
    if(casted) 
      return static_cast<visit_result_t<F, M>>(std::invoke(f, *casted));

    if constexpr(sizeof...(Is) > 0)
      return visit(m, f, std::index_sequence<Is...>{});
    
    black_unreachable();
  }

  template<matchable M, typename F>
  auto visit(M m, F f) {
    constexpr auto N = std::tuple_size_v<match_cases_t<M>>;
    
    return visit(m, f, std::make_index_sequence<N>{});
  }

  template<typename F>
  auto visitor(F f) {
    return [=](can_visit_over<F> auto arg) {
      return visit(arg, f);
    };
  }

  //
  // Matching combinator and match function
  //
  struct missing_case_t { 
    template<typename T>
    [[noreturn]] operator T() const { black_unreachable(); }
  };
  
  template<matchable M>
  auto match(M m, std::source_location loc = std::source_location::current()) {
    return [=](auto ...fs) {
      return visitor(
        dispatching(
          unpacking(fs)...,
          [&](auto missing) -> missing_case_t { 
            throw bad_pattern(missing, loc); 
          }
        )
      )(m);
    };
  }

  template<matchable M>
  auto match(
    std::optional<M> m, 
    std::source_location loc = std::source_location::current()
  ) 
  {
    return [=](auto ...fs) -> std::optional<decltype(match(*m, loc)(fs...))> {
      if(m)
        return match(*m, loc)(fs...);
      return {};
    };
  }
  
  template<matchable M, typename E>
  auto match(
    std::expected<M, E> m, 
    std::source_location loc = std::source_location::current()
  ) {
    return [=](auto ...fs) -> std::expected<decltype(match(*m, loc)(fs...)), E> 
    {
      if(m)
        return match(*m, loc)(fs...);
      return std::unexpected(m.error());
    };
  }

  template<typename ...Fs>
  auto matching(Fs ...fs) {
    return [=](
      matchable auto m, 
      std::source_location loc = std::source_location::current()
    ) {
      return match(m, loc)(fs...);
    };
  }

}

namespace black::support {
  //
  // `match_cases` and `match_downcast` for `std::variant`
  //
  template<typename ...Cases>
  struct match_cases<std::variant<Cases...>> :
    std::type_identity<std::tuple<Cases...>> { };
    
  template<typename U, typename ...Cases>
    requires (std::same_as<U, Cases> || ...)
  struct match_downcast<std::variant<Cases...>, U> {
    static std::optional<U> downcast(std::variant<Cases...> const& v) {
      if(std::holds_alternative<U>(v))
        return {std::get<U>(v)};
      return {};
    }
  };

  //
  // `common_result` for `std::optional` and `std::expected`
  //
  template<typename T, typename U>
    requires std::common_with<T, U>
  struct common_result<T, std::optional<U>>
    : std::type_identity<std::optional<std::common_type_t<T, U>>> { };
  
  template<typename T, typename U>
    requires std::common_with<U, T>
  struct common_result<std::optional<U>, T>
    : std::type_identity<std::optional<std::common_type_t<U, T>>> { };

  template<typename T, typename U>
    requires std::common_with<T, U>
  struct common_result<std::optional<T>, std::optional<U>>
    : std::type_identity<std::optional<std::common_type_t<T, U>>> { };

  template<typename T, typename U, typename E>
    requires std::common_with<T, U>
  struct common_result<T, std::expected<U, E>>
    : std::type_identity<std::expected<std::common_type_t<T, U>, E>> { };
  
  template<typename T, typename U, typename E>
    requires std::common_with<U, T>
  struct common_result<std::expected<U, E>, T>
    : std::type_identity<std::expected<std::common_type_t<U, T>, E>> { };
  
  template<typename T, typename U, typename E>
    requires std::common_with<T, U>
  struct common_result<std::expected<T, E>, std::expected<U, E>>
    : std::type_identity<std::expected<std::common_type_t<T, U>, E>> { };
  
}


namespace black::support {
  using internal::unpacking;
  using internal::dispatching;
  using internal::otherwise;
  using internal::visit;
  using internal::visitor;
  using internal::match;
  using internal::matching;
}

#endif // BLACK_SUPPORT_FUNCTIONAL_HPP
