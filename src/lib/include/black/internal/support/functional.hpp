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
  // lazy() combinator
  //
  template<typename F>
  class lazy_t 
  {
  public:
    lazy_t(F f) : _f{std::move(f)} { }

    template<typename T>
      requires std::convertible_to<std::invoke_result_t<F>, T>
    operator T() {
      return _f();
    }

  private:
    F _f;
  };

  template<typename F>
  auto lazy(F f) {
    return lazy_t<F>{std::move(f)};
  }

  //
  // Returns a lambda that ignores an additional dummy first argument
  //
  inline auto ignore1(auto f) {
    return [=]<typename ...Args>(auto&&, Args&& ...args) 
      -> decltype(f(std::forward<Args>(args)...))
    {
      return f(std::forward<Args>(args)...);
    };
  }


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

namespace black::support::internal {

  template<typename F, typename Cases>
  struct visit_result_ { };
  
  template<typename F, typename ...Cases>
  struct visit_result_<F, std::tuple<Cases...>> : std::common_type<
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
  // `match_cases` and `match_downcast` for standard types `std::variant`, 
  // `std::optional` and  `std::expected`
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
  
  
  template<typename T, typename E>
  struct match_cases<std::expected<T, E>> 
    : std::type_identity<std::tuple<T, E>> { };
    
  template<typename R, typename E>
  struct match_downcast<std::expected<R, E>, R> {
    static std::optional<R> downcast(std::expected<R, E> const& v) {
      if(v.has_value())
        return {v.value()};
      return {};
    }
  };
  
  template<typename R, typename T>
  struct match_downcast<std::expected<T, R>, R> {
    static std::optional<R> downcast(std::expected<T, R> const& v) {
      if(!v.has_value())
        return {v.error()};
      return {};
    }
  };
  
  template<typename T>
  struct match_cases<std::optional<T>> 
    : std::type_identity<std::tuple<T, internal::otherwise>> { };
    
  template<typename U>
  struct match_downcast<std::optional<U>, U> {
    static std::optional<U> downcast(std::optional<U> const& v) {
      if(v.has_value())
        return {v.value()};
      return {};
    }
  };
  
  template<typename T>
  struct match_downcast<T, internal::otherwise> {
    std::optional<internal::otherwise> downcast(std::optional<T> const& v) {
      if(!v.has_value())
        return {internal::otherwise{}};
      return {};
    }
  };
}


namespace black::support {
  using internal::lazy;
  using internal::ignore1;
  using internal::unpacking;
  using internal::dispatching;
  using internal::otherwise;
  using internal::visitor;
  using internal::match;
  using internal::matching;
}

#endif // BLACK_SUPPORT_FUNCTIONAL_HPP
