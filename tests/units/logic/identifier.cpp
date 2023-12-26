//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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

#include <black/support.hpp>
#include <black/logic.hpp>

#include <format>

using namespace black::logic;
using namespace black::support;

TEST_CASE("labels") {

  label id = "hello";
  label id2 = 42;

  label id3 = id;

  REQUIRE(id3 == id);
  REQUIRE(id3 != id2);

  std::string str1 = std::format("{}", id);
  std::string str2 = std::format("{}", id2);

  REQUIRE(str1 == "hello");
  REQUIRE(str2 == "42");

}

TEST_CASE("identifiers and paths") {

  enable_debug_msgs("test");

  REQUIRE_THROWS_AS(identifier{std::vector<int>{}}, assume_error);

  identifier id1 = {"hello", 42};
  identifier id2 = {"hi", 0};

  identifier id3 = "hello";

  path p = "hello";

  path p2 = {id1, id2};

  path p3{path::root, path::root, id1};

  REQUIRE(p.identifiers().size() == 1);
  REQUIRE(p2.identifiers().size() == 2);
  REQUIRE(p3.identifiers().size() == 2);

  REQUIRE(p.identifiers()[0] == "hello");
  REQUIRE(p2.identifiers()[0] == id1);
  REQUIRE(p2.identifiers()[1] == id2);
  REQUIRE(p3.identifiers()[0].is_root());
  REQUIRE(p3.identifiers()[1] == id1);

  path p4 = p3 / p;
  path p5 = p / "world";

  REQUIRE(p4.identifiers().size() == 3);
  REQUIRE(p5.identifiers().size() == 2);

  REQUIRE(p4.is_absolute());
  REQUIRE(!p5.is_absolute());

}
