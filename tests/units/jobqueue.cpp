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

#include <catch2/catch.hpp>

#include <black/support/jobqueue.hpp>

#include <iostream>
#include <atomic>
#include <unistd.h>
using namespace black;

TEST_CASE("Job queue") {
  jobqueue q;

  q.start();

  std::future<int> result0 = q.enqueue([]{ return 0; });
  std::future<int> result1 = q.enqueue([]{ return 1; });
  std::future<int> result2 = q.enqueue([]{ return 2; });
  std::future<int> result3 = q.enqueue([]{ return 3; });
  
  REQUIRE(result0.get() == 0);
  REQUIRE(result1.get() == 1);
  REQUIRE(result2.get() == 2);
  REQUIRE(result3.get() == 3);
}
