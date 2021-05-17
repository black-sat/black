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

#include <black/support/jobqueue.hpp>

#include <concurrentqueue/blockingconcurrentqueue.h>

#include <functional>
#include <vector>
#include <atomic>
#include <iostream>

namespace black::internal
{
  struct jobqueue::_jobqueue_t {
    moodycamel::BlockingConcurrentQueue<std::unique_ptr<_invokable_t>> queue;
    std::vector<std::thread> threads;
    std::atomic<bool> stop = false;

    void worker();
  };

  jobqueue::jobqueue() : _data{std::make_unique<_jobqueue_t>()} { }
  jobqueue::~jobqueue() {
    stop();
  }

  void jobqueue::start(unsigned int nthreads) {
    for(unsigned i = 0; i < nthreads; ++i) {
      _data->threads.push_back(std::thread{[this] {
        _data->worker();
      }});
    }
  }

  void jobqueue::stop() {
    _data->stop = true;
    for(size_t i = 0; i < _data->threads.size(); ++i) {
      _data->threads[i].join();
    }
    _data->threads.clear();
  }

  void jobqueue::enqueue_impl(std::unique_ptr<_invokable_t> job) {
    _data->queue.enqueue(std::move(job));
  }

  void jobqueue::_jobqueue_t::worker() {
    while(!stop) {
      std::unique_ptr<_invokable_t> job;
      bool dequeued = queue.wait_dequeue_timed(job, 1000 /* µs */);
      if(dequeued)
        job->invoke();
    }
  }

}
