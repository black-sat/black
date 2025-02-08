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

#include <catch.hpp>

#include <string>
#include <black/support>

#include <print>

inline std::expected<double, std::string> reciprocal_exp(double x) {
    if(x == 0)
        co_return std::unexpected("division by zero!");

    co_return 1.0 / x;
}

inline std::expected<double, std::string> test_exp(double x) {
    co_return co_await reciprocal_exp(x);
}

TEST_CASE("Coroutines support for std::expected") {

    auto e1 = test_exp(42);

    REQUIRE(e1.has_value());
    REQUIRE(*e1 == 1.0/42);

    auto e2 = test_exp(0);
    REQUIRE(!e2.has_value());
    REQUIRE(e2.error() == "division by zero!");

}

inline std::optional<double> reciprocal_opt(double x) {
    if(x == 0)
        co_return std::nullopt;

    co_return 1.0 / x;
}

inline std::optional<double> test_opt(double x) {
    co_return co_await reciprocal_opt(x);
}

TEST_CASE("Coroutines support for std::optional") {

    auto e1 = test_opt(42);

    REQUIRE(e1.has_value());
    REQUIRE(*e1 == 1.0/42);

    auto e2 = test_opt(0);
    REQUIRE(!e2.has_value());

}