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

  template<typename P>
  struct co_deleter_t {
    using pointer = std::coroutine_handle<P>;

    void operator()(pointer p) const {
      p.destroy();
    }
  };

  using input_t = std::ranges::subrange<const char *>;

  template<typename Out>
  using result = std::expected<Out, std::string>;

  template<typename Out>
  class parser 
  {
  public:
    struct promise_type;

    parser(parser const&) = delete;
    parser(parser &&) = default;
    ~parser() = default;
    
    parser &operator=(parser const&) = delete;
    parser &operator=(parser &&) = default;

    result<Out> run(input_t input) {
      _handle.get().promise()._in = input;
      _handle.get().resume();

      black_assert(_handle.get().promise()._result.has_value());
      return *_handle.get().promise()._result;
    }

  private:
    parser(std::coroutine_handle<promise_type> h) : _handle{h} { }

    std::unique_ptr<
      std::coroutine_handle<promise_type>, co_deleter_t<promise_type>
    > _handle;
  };

  template<typename T>
  parser<T> succeed(T v) {
    co_return v;
  }

  struct failure {
    std::string error;
  };

  inline failure fail(std::string error) {
    return { error };
  }

  template<typename Out>
  struct awaiter_t {
    result<Out> _result;

    bool await_ready() { return _result.has_value(); }
    
    bool await_suspend(std::coroutine_handle<>) { return false; }
    
    Out await_resume() {
      black_assert(_result.has_value());
      return std::move(*_result);
    }
  };

  template<typename Out>
  struct parser<Out>::promise_type 
  {        
    parser get_return_object() { 
      return parser{ std::coroutine_handle<promise_type>::from_promise(*this) };
    }
    
    std::suspend_always initial_suspend() { return {}; }
    
    void return_value(Out out) {
      _result = result<Out>{std::move(out)};
    }
    
    void unhandled_exception() {}
    
    std::suspend_always final_suspend() noexcept { return {}; }

    awaiter_t<Out> await_transform(failure f) {
      _result = result<Out>{ std::unexpected(f.error) };
      return awaiter_t<Out>{ *_result };
    }

    template<typename U>
    awaiter_t<U> await_transform(parser<U> p) {
      return awaiter_t<U>{ p.run(_in) };
    }

    input_t _in;
    std::optional<result<Out>> _result;
  };

}

#endif // BLACK_INTERNAL_PARSING_COMBINATORS_HPP
