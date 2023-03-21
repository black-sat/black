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

#include <black/support/tribool.hpp>

#include <sstream>

using namespace black::support;

TEST_CASE("Testing tribool")
{
  tribool tb0;
  tribool tb1 = tribool::undef;
  tribool tb2 = true;
  tribool tb3 = false;
  
  SECTION("Output stream") {
    std::vector<std::pair<tribool, std::string>> tests = {
      {tb0, "tribool::undef"},
      {tb1, "tribool::undef"},
      {tb2, "true"},
      {tb3, "false"}
    };
    
    for(auto [b, res] : tests) {
      std::stringstream s;
      s << b;

      REQUIRE(s.str() == res);
    }
  }

  SECTION("Equalities") {
    REQUIRE(tb1 == tb1);
    REQUIRE(tb1 != tb2);
    REQUIRE(tb1 != tb3);

    REQUIRE(tb1 != true);
    REQUIRE(true != tb1);
    REQUIRE(tb1 != false);
    REQUIRE(false != tb1);

    REQUIRE(tb2 == true);
    REQUIRE(tb2 != false);
    REQUIRE(tb3 == false);
    REQUIRE(tb3 != true);
  }

  SECTION("operator!") {
    REQUIRE((!tb1) == tb1);
    REQUIRE((!tb2) == tb3);
    REQUIRE((!tb3) == tb2);
  }
}
