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

#include <black/support>

#include <variant>
#include <string>

using namespace black::support;

using test = std::variant<int, std::tuple<std::string, float>>;

TEST_CASE("Match infrastructure") {

  test t = 21;

  STATIC_REQUIRE(matchable<std::variant<int, std::string>>);
  STATIC_REQUIRE(!matchable<int>);

  auto b = match(t)(
    [](int x) {
      return x * 2;
    },
    [](std::tuple<std::string, float>, std::string, float) {
      return false;
    }
  );

  STATIC_REQUIRE(std::is_same_v<decltype(b), int>);

  REQUIRE(b == 42);

  SECTION("Partial pattern matches") {
    REQUIRE_THROWS_AS(
      match(t)(
        [](std::tuple<std::string, float>) { 
          return 42;
        }
      ), bad_pattern
    );

    auto r = match(t)(
      [](int x) {
        return x * 2;
      }
    );

    STATIC_REQUIRE(std::is_same_v<decltype(r), int>);

    REQUIRE(r == 42);
  } 

  SECTION("Match on optional<T>") {
    std::optional<int> v;

    auto r = match(v)(
      [](int x) {
        return x * 2;
      },
      [](otherwise) {
        return 0;
      }
    );

    REQUIRE(r == 0);
  }

  SECTION("Match on expected<T, E>") {
    std::expected<int, std::string> e = 21;

    auto r = match(e)(
      [](int x) {
        return x * 2;
      },
      [](std::string) {
        return 0;
      }
    );

    REQUIRE(r == 42);
  }

}

TEST_CASE("Dispatch") {
  auto d = dispatch(
    [](int x) {
      return x * 2;
    },
    [](float y) { 
      return y / 2;
    }
  );

  REQUIRE(d(21) == 42);

  REQUIRE_THROWS(d("hello"));
}