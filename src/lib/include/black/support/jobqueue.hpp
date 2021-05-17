//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#ifndef BLACK_SUPPORT_JOBQUEUE_HPP
#define BLACK_SUPPORT_JOBQUEUE_HPP

#include <memory>
#include <thread>
#include <future>
#include <exception>

namespace black::internal 
{
  class jobqueue 
  {
    public:
      jobqueue();
      ~jobqueue();

      void start(unsigned int nthreads = std::thread::hardware_concurrency());
      void stop();

      template<typename F, typename R = std::invoke_result_t<F>>
      std::future<R> enqueue(F&& job);

    private:
      struct _invokable_t;
      template<typename R, typename F>
      struct _invoker_t;

      void enqueue_impl(std::unique_ptr<_invokable_t>);
      
      struct _jobqueue_t;
      std::unique_ptr<_jobqueue_t> _data;
  };

}

namespace black {
  using internal::jobqueue;
}

//
// Implementation
//
namespace black::internal {
  struct jobqueue::_invokable_t {
    virtual ~_invokable_t() = default;
    virtual void invoke() = 0;
  protected:
    _invokable_t() = default;
    _invokable_t(_invokable_t&&) = default;
    _invokable_t& operator=(_invokable_t&&) noexcept = default;
  };

  template<typename F, typename R>
  struct jobqueue::_invoker_t : jobqueue::_invokable_t 
  {
    _invoker_t(F&& f) : _func(std::move(f)) { }
    _invoker_t(F const& f) : _func(f) { }
    _invoker_t(_invoker_t&&) = default;
    _invoker_t& operator=(_invoker_t&&) = default;

    std::future<R> get_future() { return _promise.get_future(); }

    void invoke() override {
      try {
        if constexpr(std::is_same_v<R, void>) {
          _func();
          _promise.set_value();
        } else {
          _promise.set_value(_func());
        }
      } catch (...) {
        _promise.set_exception(std::current_exception());
      }
    }
  private:
    F _func;
    std::promise<R> _promise;
  };

  template<typename F, typename R>
  std::future<R> jobqueue::enqueue(F&& job)
  {
    auto invoker = std::make_unique<_invoker_t<F, R>>(std::forward<F>(job));
    
    std::future<R> future = invoker->get_future();

    enqueue_impl(std::move(invoker));

    return future;
  }
}


#endif // BLACK_SUPPORT_JOBQUEUE_HPP
