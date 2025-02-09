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
  template<typename In>
  using range = std::ranges::subrange<const In *>;

  template<typename Out, typename In = char>
  class parser;

  //
  // Type representing the failure of a parser
  //
  struct failure { };

  //
  // Types representing the primitive actions available to parsers
  //

  //
  // test if we reached EOF
  //
  struct eof_t { };

  //
  // peek the current token
  //
  struct peek_t { };

  //
  // consume the current token
  //
  struct consume_t { };

  //
  // lift to an std::optional a parser that fails without consuming input
  //
  template<typename Out, typename In>
  struct optional_t { 
    parser<Out, In> inner;
  };

  //
  // Parse ahead without consuming input on failure
  //
  template<typename Out, typename In>
  struct lookahead_t {
    parser<Out, In> inner;
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
  template<typename Out, typename In>
  class parser 
  {
  public:
    struct promise_type;

    parser() = delete;
    parser(parser const&) = delete;
    parser(parser &&) = default;
    
    parser &operator=(parser const&) = delete;
    parser &operator=(parser &&) = default;

    std::optional<Out> run(range<In> input, range<In> *tail = nullptr) {
      if(_coroutine->done())
        throw parser_reused();

      _coroutine->promise()._input = input;
      _coroutine->resume();
      if(tail)
        *tail = _coroutine->promise()._input;

      return _coroutine->promise()._value;
    }

  private:
    parser(std::coroutine_handle<promise_type> h) : _coroutine{ h } { }
    support::coroutine_handle_ptr<promise_type> _coroutine;
  };

  template<typename P, typename Out, typename In>
  struct is_parser_of : std::false_type { };

  template<typename Out, typename In>
  struct is_parser_of<parser<Out, In>, Out, In> : std::true_type { };

  template<typename P, typename Out, typename In>
  inline constexpr bool is_parser_of_v = is_parser_of<P, Out, In>::value;

  template<typename P, typename Out, typename In = char>
  concept parser_of = is_parser_of_v<P, Out, In>;

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

  template<typename Out, typename In>
  struct parser<Out, In>::promise_type 
  {
    range<In> _input;
    std::optional<Out> _value = std::nullopt;

    // FIXME: Ask on SO why this is needed both on GCC and Clang
    promise_type() = default;

    auto get_return_object() {
      return parser<Out>{ 
        std::coroutine_handle<promise_type>::from_promise(*this) 
      }; 
    }

    auto initial_suspend() { return std::suspend_always{}; }
    
    void return_value(Out v) {
      _value = std::move(v);
    }
    
    void return_value(failure) {
      _value = std::nullopt;
    }
    
    void unhandled_exception() {}
    
    auto final_suspend() noexcept { return std::suspend_always{}; }

    template<typename U>
    auto await_transform(parser<U> p) {
      return awaiter_t<U>{ p.run(_input, &_input) };
    }

    auto await_transform(eof_t) {
      return awaiter_t{ _input.empty() };
    }

    auto await_transform(peek_t) {
      if(!_input)
        return awaiter_t<In>{ };

      return awaiter_t{ *std::begin(_input) };
    }

    auto await_transform(consume_t) {
      if(!_input)
        return awaiter_t<In>{ };

      auto t = *std::begin(_input);
      _input.advance(1);
      return awaiter_t<In>{ t };
    }

    template<typename U>
    auto await_transform(optional_t<U, In> opt) {
      auto saved = _input;
      auto result = opt.inner.run(_input, &_input);
      
      if(!result && std::begin(saved) != std::begin(_input))
        return awaiter_t<std::optional<U>>{ };
      
      return awaiter_t<std::optional<U>>{ std::move(result) };
    }

    template<typename U>
    auto await_transform(lookahead_t<U, In> opt) {
      auto saved = _input;
      auto result = opt.inner.run(_input, &_input);
      if(result)
        return awaiter_t<U>{ *result };
      
      _input = saved;
      return awaiter_t<U>{ };
    }

  };

}

#endif // BLACK_PARSING_PARSER_HPP
