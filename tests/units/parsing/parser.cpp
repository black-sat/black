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

#include <black/parsing>

#include <print>
#include <cctype>

using namespace black::parsing;

inline parser<std::string> half() {
    co_await consume('{');

    std::string result;

    while(! co_await eof()) {
        char c = co_await peek();

        if(c == '}')
            break;

        result.push_back(c);
        co_await consume();
    }

    co_await consume('}');

    co_return result;
}

TEST_CASE("Basic operations on parsers") 
{
    parser<std::string> p = half();

    SECTION("Found") {
        std::string hello = "{hello}";
        auto result = 
            p.run(range<char>{hello.c_str(), hello.c_str() + hello.size()});

        REQUIRE(result.has_value());
        REQUIRE(*result == "hello");
    }

    SECTION("Not found") {
        std::string mandi = "{mandi";
        auto result = 
            p.run(range<char>{mandi.c_str(), mandi.c_str() + mandi.size()});

        REQUIRE(!result.has_value());
    }

}

inline parser<std::string> opt_test() {
    auto opt = co_await optional(consume('{'));
    if(opt)
        co_return "Ok!";
    
    co_return "Nope!";
}

TEST_CASE("Optional") {

    parser<std::string> p = opt_test();

    SECTION("Ok") {
        std::string hello = "{hello}";
        auto result = 
            p.run(range<char>{hello.c_str(), hello.c_str() + hello.size()});

        REQUIRE(result == "Ok!");
    }

    SECTION("Nope") {
        std::string mandi = "[mandi]";
        auto result = 
            p.run(range<char>{mandi.c_str(), mandi.c_str() + mandi.size()});

        REQUIRE(result == "Nope!");

        REQUIRE_THROWS(
            p.run(range<char>{mandi.c_str(), mandi.c_str() + mandi.size()})
        );
    }

}