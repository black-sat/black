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

#include <black/support>
#include <black/ast/core>

#include <format>

using namespace black::support;
using namespace black::ast::core;


TEST_CASE("labels") {

  label empty;
  label id = "hello";
  label id2 = label{42};

  label id3 = id;

  REQUIRE(empty == empty);
  REQUIRE(empty != id);

  REQUIRE(id3 == id);
  REQUIRE(id3 != id2);

  std::string stre = std::format("{}", empty);
  std::string str1 = std::format("{}", id);
  std::string str2 = std::format("{}", id2);

  REQUIRE(stre == "<empty label>");
  REQUIRE(str1 == "hello");
  REQUIRE(str2 == "42");

  REQUIRE(empty.hash() == 0);

}