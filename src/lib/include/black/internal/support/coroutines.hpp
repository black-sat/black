//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2025 Nicola Gigante
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

#ifndef BLACK_SUPPORT_COROUTINES_HPP
#define BLACK_SUPPORT_COROUTINES_HPP

#include <expected>
#include <coroutine>
#include <print>

namespace black::support::internal 
{
  template<typename T>
  struct return_wrapper_t {
    std::optional<T> v;

    return_wrapper_t(std::optional<T> **out) {
      *out = &v;
    }

    operator T() const {
      black_assert(v.has_value());
      return *v;
    }
  };

  template<typename T>
  struct awaiter_t {
    std::optional<T> _value = std::nullopt;

    bool await_ready() { return _value.has_value(); }

    void await_suspend(std::coroutine_handle<> h) { h.destroy(); }
    
    T await_resume() { return std::move(*_value); }
  };

  template<typename T, typename E>
  struct expected_promise_t 
  {
    std::optional<std::expected<T, E>> *_value;
    
    auto get_return_object() { 
      return return_wrapper_t { &_value }; 
    }

    std::suspend_never initial_suspend() { return {}; }

    void return_value(T v) {
      _value->emplace(std::move(v));
    }

    template<std::convertible_to<E> E2>
    void return_value(std::unexpected<E2> e) {
      _value->emplace(std::unexpected<E>{std::move(e.error())});
    }

    void unhandled_exception() {}
    
    std::suspend_never final_suspend() noexcept { return {}; }

    template<typename U, std::convertible_to<E> E2>
    auto await_transform(std::expected<U, E2> e) {
      if(e.has_value())
        return awaiter_t<U> { std::move(*e) }; 
      
      _value->emplace(std::unexpected<E>(std::move(e.error())));
      return awaiter_t<U> { };
    }

  };
  
  template<typename T>
  struct optional_promise_t 
  {
    std::optional<std::optional<T>> *_value;
    
    auto get_return_object() { 
      return return_wrapper_t { &_value }; 
    }

    std::suspend_never initial_suspend() { return {}; }

    void return_value(T v) {
      _value->emplace(std::move(v));
    }

    void return_value(std::nullopt_t) {
      _value->emplace(std::nullopt);
    }

    void unhandled_exception() {}
    
    std::suspend_never final_suspend() noexcept { return {}; }

    template<typename U>
    auto await_transform(std::optional<U> opt) {
      if(opt.has_value())
        return awaiter_t<U> { std::move(*opt) }; 
      
      _value->emplace(std::nullopt);
      return awaiter_t<U> { };
    }

  };
}

template<typename T, typename E, typename... Args>
struct std::coroutine_traits<std::expected<T, E>, Args...> {
  using promise_type = black::support::internal::expected_promise_t<T, E>;
};

template<typename T, typename... Args>
struct std::coroutine_traits<std::optional<T>, Args...> {
  using promise_type = black::support::internal::optional_promise_t<T>;
};

#endif // BLACK_SUPPORT_COROUTINES_HPP
