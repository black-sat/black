//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <string>

using namespace black::support;

TEST_CASE("Unpack combinator") {

    using T = std::tuple<int, float, std::string>;

    T v{42, 3.14, "hello world"};

    auto test1 = unpacking([](T t) { return get<0>(t); });
    auto test2 = unpacking([](T, int x) { return x; });
    auto test3 = unpacking([](T, int x, float) { return x; });
    auto test4 = unpacking([](T, int x, float, std::string) { return x; });

    REQUIRE(test1(v) == 42);
    REQUIRE(test2(v) == 42);
    REQUIRE(test3(v) == 42);
    REQUIRE(test4(v) == 42);

}

TEST_CASE("Dispatch combinator") {

    auto d = dispatching(
        [](int x) { return x; },
        [](float f) { return f; },
        [](std::string s) { return s; }
    );

    REQUIRE(d(42) == 42);
}

TEST_CASE("Visiting combinator") 
{
    using T = std::variant<int, float, std::string>;
    T t = 42;

    auto v = visitor(
        [](auto e) {
            return std::format("{}", e);
        }
    );

    REQUIRE(v(t) == "42");
}

TEST_CASE("Match function") {
    using T = std::variant<int, float, std::string>;

    T t = 42;

    auto r = match(t)(
        [](auto e) {
            return std::format("{}", e);
        }
    );

    REQUIRE(r == "42");

    REQUIRE_THROWS(
        match(t)(
            [](std::string x) { return x; }
        )
    );

    auto f = matching(
        [](int x) { return x; }
    );

    REQUIRE(f(t) == 42);
}

TEST_CASE("Match on std::optional") {

    using T = std::variant<int, float, std::string>;

    std::optional<T> t = 42;

    auto r = match(t)(
        [](int x) { return x; }
    );

    REQUIRE(r == 42);

    t = {};

    auto r2 = match(t)(
        [](int x) { return x; }
    );

    REQUIRE(!r2.has_value());

}

TEST_CASE("Match on std::expected") {

    using T = std::variant<int, float, std::string>;

    std::expected<T, std::string> t = 42;

    auto r = match(t)(
        [](int x) { return x; }
    );

    REQUIRE(r == 42);

    t = std::unexpected("Error");

    auto r2 = match(t)(
        [](int x) { return x; }
    );

    REQUIRE(!r2.has_value());

}