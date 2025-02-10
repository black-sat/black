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

#ifndef BLACK_PARSING_PARSER_HPP
#define BLACK_PARSING_PARSER_HPP

#include <black/support>

#include <coroutine>
#include <ranges>

namespace black::parsing 
{
  using range = std::ranges::subrange<const char *>;

  template<typename Out>
  class parser;

  //
  // Types representing the primitive actions available to parsers
  //

  //
  // Type to return to cause a parsing failure
  //
  struct fail_t { };

  //
  // Succeed if we are at EOF, fail otherwise
  //
  struct eof_t { };

  //
  // peek the current token
  //
  struct peek_t { };

  //
  // advance the current position in the input
  //
  struct advance_t { };

  //
  // lift to an std::optional a parser that fails without consuming input
  //
  template<typename Out>
  struct optional_t { 
    parser<Out> inner;
  };

  //
  // Parse ahead without consuming input on failure
  //
  template<typename Out>
  struct try_t {
    parser<Out> inner;
  };

  //
  // Exception thrown if a parser is reused
  //
  class parser_reused : support::exception 
  {
  public:
    parser_reused() : support::exception("parser::run() invoked twice") { }
    virtual ~parser_reused() override = default;
  };

  //
  // Main parser type
  //
  template<typename Out>
  class [[nodiscard("parsers must be awaited")]] parser 
  {
  public:
    struct promise_type;

    parser() = delete;

    parser(parser const&) = delete;
    parser(parser &&) = default;
    
    parser &operator=(parser const&) = delete;
    parser &operator=(parser &&) = default;

    auto run(range input, range *tail = nullptr) {
      if(_coroutine->done())
        throw parser_reused();

      _coroutine->promise().input = input;
      _coroutine->resume();
      if(tail)
        *tail = _coroutine->promise().input;

      return _coroutine->promise().value;
    }

  private:
    parser(std::coroutine_handle<promise_type> h) : _coroutine{ h } { }
    support::coroutine_handle_ptr<promise_type> _coroutine;
  };

  template<typename Pred>
  concept predicate = requires (Pred p, char c) {
    { p(c) } -> std::convertible_to<bool>;
  };

  template<typename P, typename Out>
  struct is_parser_of : std::false_type { };

  template<typename Out>
  struct is_parser_of<parser<Out>, Out> : std::true_type { };

  template<typename P, typename Out>
  inline constexpr bool is_parser_of_v = is_parser_of<P, Out>::value;

  template<typename P, typename Out>
  concept parser_of = is_parser_of_v<P, Out>;

  template<typename T>
  struct awaiter_t {
    std::optional<T> _value = std::nullopt;

    awaiter_t() = default;
    awaiter_t(T v) : _value{v} { }
    awaiter_t(std::optional<T> opt) : _value{opt} { }

    bool await_ready() { return _value.has_value(); }

    void await_suspend(std::coroutine_handle<>) { }
    
    T await_resume() { return std::move(*_value); }
  };
  
  struct void_awaiter_t {
    bool _succeed;

    void_awaiter_t(bool succeed) : _succeed{succeed} { }

    bool await_ready() { return _succeed; }

    void await_suspend(std::coroutine_handle<>) { }
    
    void await_resume() { }
  };

  template<typename Out>
  struct promise_value_holder {
    std::optional<Out> value = std::nullopt;

    promise_value_holder() = default;

    void return_value(Out v) {
      value = std::move(v);
    }
  };

  template<>
  struct promise_value_holder<void> {
    std::optional<std::monostate> value = std::nullopt;

    promise_value_holder() = default;

    void return_void() {
      value = std::monostate{};
    }
  };

  template<typename Out>
  struct parser<Out>::promise_type : promise_value_holder<Out>
  {
    range input;

    // FIXME: Ask on SO why this is needed both on GCC and Clang
    promise_type() = default;

    auto get_return_object() {
      return parser<Out>{ 
        std::coroutine_handle<promise_type>::from_promise(*this) 
      }; 
    }

    auto initial_suspend() { return std::suspend_always{}; }
    
    void unhandled_exception() {}
    
    auto final_suspend() noexcept { return std::suspend_always{}; }

    template<typename U>
    auto await_transform(parser<U> p) {
      return awaiter_t<U>{ p.run(input, &input) };
    }

    auto await_transform(parser<void> p) {
      return void_awaiter_t{ p.run(input, &input).has_value() };
    }

    auto await_transform(fail_t) {
      return void_awaiter_t{ false };
    }

    auto await_transform(eof_t) {
      return void_awaiter_t{ input.empty() };
    }

    auto await_transform(peek_t) {
      if(!input)
        return awaiter_t<char>{ };

      return awaiter_t{ *std::begin(input) };
    }

    auto await_transform(advance_t) {
      if(!input)
        return void_awaiter_t{ false };

      input.advance(1);
      return void_awaiter_t{ true };
    }

    template<typename U>
    auto await_transform(optional_t<U> opt) {
      auto saved = input;
      auto result = opt.inner.run(input, &input);
      
      if(!result && std::begin(saved) != std::begin(input))
        return awaiter_t<std::optional<U>>{ };
      
      return awaiter_t<std::optional<U>>{ std::move(result) };
    }

    template<typename U>
    auto await_transform(try_t<U> opt) {
      auto saved = input;
      auto result = opt.inner.run(input, &input);
      if(result)
        return awaiter_t<U>{ *result };
      
      input = saved;
      return awaiter_t<U>{ };
    }

  };

}

#endif // BLACK_PARSING_PARSER_HPP
