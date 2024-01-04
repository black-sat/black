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

#ifndef BLACK_SUPPORT_DISPATCH_HPP
#define BLACK_SUPPORT_DISPATCH_HPP

#include <source_location>
#include <utility>
#include <tuple>

namespace black::support::internal {

  template<typename Callable, typename Arg>
  struct is_invocable_unpack : std::false_type { };

  template<typename Callable, typename Arg>
  inline constexpr bool is_invocable_unpack_v = 
    is_invocable_unpack<Callable, Arg>::value;

  template<
    typename Callable, typename Arg, 
    typename Indexes = std::make_index_sequence<std::tuple_size_v<Arg>>
  >
  struct is_invocable_unpack_ : std::false_type { };

  template<typename Callable, typename Arg, size_t ...I>
  struct is_invocable_unpack_<Arg, Callable, std::index_sequence<I...>>
    : std::is_invocable<Callable, Arg, std::tuple_element_t<I, Arg>...> { };

  template<typename Callable, typename Arg>
    requires requires { std::tuple_size<Arg>::value; }
  struct is_invocable_unpack<Callable, Arg>
    : is_invocable_unpack_<Callable, Arg> { };

  template<typename Callable, typename Arg, size_t ...I>
  auto unpack(Callable const& callable, Arg arg, std::index_sequence<I...>) {
    return std::invoke(callable, arg, get<I>(arg)...);
  }

  struct missing_case_t {
    template<typename T>
    [[noreturn]] operator T() const { black_unreachable(); }
  };

  template<typename Arg>
  missing_case_t dispatch_impl(std::source_location loc, Arg&& arg) {
    throw bad_pattern(std::forward<Arg>(arg), loc);
  }

  template<typename Arg, typename Callable, typename ...Callables>
  auto dispatch_impl(
    std::source_location loc,
    Arg&& arg, Callable const& callable, Callables const& ...callables
  ) {
    if constexpr(is_invocable_unpack_v<Callable, Arg>) 
      return unpack(
        std::forward<Arg>(arg), callable, 
        std::make_index_sequence<std::tuple_size_v<Arg>>{}
      );
    else if constexpr(std::is_invocable_v<Callable, Arg>)
      return std::invoke(callable, std::forward<Arg>(arg));
    else
      return dispatch_impl(loc, std::forward<Arg>(arg), callables...);
  }

  template<typename ...Callables>
  class dispatch_t 
  {
  public:
    dispatch_t(Callables ...args) : _callables{std::move(args)...} { }

    template<typename Arg>
    auto operator()(
      Arg&& arg, std::source_location loc = std::source_location::current()
    ) const {
      return std::apply([&](auto const& ...callables) {
        return dispatch_impl(loc, std::forward<Arg>(arg), callables...);
      }, _callables);
    }

  private:
    std::tuple<Callables...> _callables;
  };

  struct otherwise {
    otherwise() = default;

    template<typename T>
    otherwise(T const&) { }
  };

  template<typename ...Callables>
  auto dispatch(Callables ...callables) {
    return dispatch_t<Callables...>{std::move(callables)...};
  }
}

namespace black::support {
  using internal::dispatch;
  using internal::otherwise;
}

#endif // BLACK_SUPPORT_DISPATCH_HPP
