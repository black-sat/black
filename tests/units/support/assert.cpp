//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#include <catch.hpp>

#ifdef BLACK_ASSERT_DISABLE
  #undef BLACK_ASSERT_DISABLE
#endif

#include <black/support>

#include <string>

using namespace black::support;

using namespace std::literals;

static int divide(int x) {
  black_assert(x != 0);
  return 42/x;
}

static int divide2(
  int x, std::source_location loc = std::source_location::current()
) {
  black_assume(x != 0, loc, "`x` cannot be zero");
  
  return 42/x;
}

static int divide3(int x) {
  if(x != 0)
    return 42/x;
  black_unreachable();
}

TEST_CASE("Testing assert macros") {
  
  black_assert(true);
  REQUIRE_THROWS_AS(divide(0), bad_assert);
  REQUIRE_THROWS_AS(divide2(0), bad_assumption);
  REQUIRE_THROWS_AS(divide3(0), bad_unreachable);

  try {
    divide(0);
  } catch (bad_assert const& e) {
#ifdef _MSC_VER
  REQUIRE(e.filename() == "tests\\units\\support\\assert.cpp"s);
#else
  REQUIRE(e.filename() == "tests/units/support/assert.cpp"s);
#endif
  }

}
