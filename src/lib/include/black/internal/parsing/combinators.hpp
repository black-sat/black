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

#ifndef BLACK_INTERNAL_PARSING_COMBINATORS_HPP
#define BLACK_INTERNAL_PARSING_COMBINATORS_HPP

#include <black/support>

#include <coroutine>
#include <ranges>
#include <memory>
#include <print>

namespace black::parsing {

  using input_t = std::ranges::subrange<const char *>;

  template<typename Out>
  struct success {
    input_t input;
    Out out;
  };

  struct failure {
    std::string error;
  };

  template<typename Out>
  using result = std::expected<success<Out>, failure>;

  template<typename Out>
  using runner_t = std::function< result<Out>(input_t input) >;

  template<typename Out>
  class parser 
  {
  public:
    struct promise_type;

    parser(runner_t<Out> runner) : _runner{std::move(runner)} { }

    parser(parser const&) = delete;
    parser(parser &&) = default;
    ~parser() = default;
    
    parser &operator=(parser const&) = delete;
    parser &operator=(parser &&) = default;

    result<Out> run(input_t input) {
      return _runner(input);
    }

  private:

    runner_t<Out> _runner;
  };

  template<typename T>
  parser<T> succeed(T v) {
    return parser<T>{
      [&](input_t in) { return success{in, v}; }
    };
  }

  inline failure fail(std::string error) {
    return { error };
  }

  // ugly hack until LLVM's libc++ supports `std::move_only_function`.
  template<typename F>
  struct move_only_function 
  {
    move_only_function(F f) : _f{std::move(f)} { }

    move_only_function(move_only_function &&) = default;
    move_only_function &operator=(move_only_function &&) = default;

    move_only_function(move_only_function const& f) : _f{std::move(f._f)} {
      black_unreachable();
    }

    move_only_function &operator=(move_only_function const&) {
      black_unreachable();
    }

    template<typename ...Args>
      requires std::invocable<F, Args...>
    auto operator()(Args&& ...args)  {
      return _f(std::forward<Args>(args)...);
    }

    F _f;
  }; 

  template<typename Out>
  struct awaiter_t 
  {
    result<Out> _result;

    bool await_ready() { return true; }

    bool await_suspend(std::coroutine_handle<>) {
      return !_result.has_value();
    }

    Out await_resume() {
      black_assert(_result.has_value());
      return _result->out;
    }

  };
  
  template<typename Out>
  struct parser<Out>::promise_type 
  {
    struct co_runner_t;
        
    parser get_return_object() { 
      return parser{ co_runner_t { *this } };
    }
    
    std::suspend_always initial_suspend() { return {}; }
    
    void return_value(Out out) {
      _content = result<Out>{success{_in, std::move(out)}};
    }
    
    void unhandled_exception() {}
    
    std::suspend_always final_suspend() noexcept { return {}; }

    awaiter_t<Out> await_transform(failure f) {
      _content = result<Out>{ std::unexpected(f) };
      return awaiter_t<Out>{ *_content };
    }

    template<typename U>
    awaiter_t<U> await_transform(parser<U> p) {
      _content = p.run(_in);
      return awaiter_t<U>{ *_content };
    }

    input_t _in;
    std::optional<result<Out>> _content;
  };

  

  template<typename Out>
  struct parser<Out>::promise_type::co_runner_t {
    struct deleter_t;

    co_runner_t(promise_type &p) 
      : _handle { std::coroutine_handle<promise_type>::from_promise(p)} { }

    co_runner_t(co_runner_t &&) = default;
    co_runner_t &operator=(co_runner_t &&) = default;

    // black_unreachable() here is an hack waiting for LLVM libc++ to support
    // std::move_only_function
    co_runner_t(co_runner_t const&) {
      black_unreachable();
    }
    co_runner_t &operator=(co_runner_t const&) {
      black_unreachable();
    }

    result<Out> operator()(input_t in) const {
      _handle.get().promise()._in = in;
      _handle.get().resume();

      black_assert(_handle.get().promise()._content.has_value());
      return *_handle.get().promise()._content;
    }

    std::unique_ptr<std::coroutine_handle<promise_type>, deleter_t> _handle;
  };

  template<typename Out>
  struct parser<Out>::promise_type::co_runner_t::deleter_t {
    using pointer = std::coroutine_handle<promise_type>;

    void operator()(pointer p) const {
      p.destroy();
    }
  };

}

#endif // BLACK_INTERNAL_PARSING_COMBINATORS_HPP
