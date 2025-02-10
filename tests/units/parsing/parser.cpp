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

inline parser<std::string> braced() {
    co_await ask('{');

    std::string result;

    std::optional<char> c;
    while((c = co_await optional(peek(&isalpha)))) {
        result.push_back(*c);
        co_await advance();
    }

    co_await expect('}');

    co_return result;
}

TEST_CASE("Basic operations on parsers") 
{
    parser<std::string> p = braced();

    SECTION("Found") {
        std::string hello = "{hello}";
        auto result = 
            p.run(range{hello.c_str(), hello.c_str() + hello.size()});

        REQUIRE(result.has_value());
        REQUIRE(*result == "hello");
    }

    SECTION("Not found") {
        std::string mandi = "{mandi";
        auto result = 
            p.run(range{mandi.c_str(), mandi.c_str() + mandi.size()});

        REQUIRE(!result.has_value());
    }

}

inline parser<std::string> opt_test() {
    auto opt = co_await optional(ask('{'));
    if(opt)
        co_return "Ok!";
    
    co_return "Nope!";
}

TEST_CASE("Optional") {

    parser<std::string> p = opt_test();

    SECTION("Ok") {
        std::string hello = "{hello}";
        auto result = 
            p.run(range{hello.c_str(), hello.c_str() + hello.size()});

        REQUIRE(result == "Ok!");
    }

    SECTION("Nope") {
        std::string mandi = "[mandi]";
        auto result = 
            p.run(range{mandi.c_str(), mandi.c_str() + mandi.size()});

        REQUIRE(result == "Nope!");

        REQUIRE_THROWS(
            p.run(range{mandi.c_str(), mandi.c_str() + mandi.size()})
        );
    }

}

TEST_CASE("Lookaehad") {

    SECTION("Skip") {
        parser<std::string> p = braced();

        std::string hello = "[hello]";
        range input{hello.c_str(), hello.c_str() + hello.size()};
        range saved = input;

        auto result = p.run(input, &input);

        REQUIRE(!result.has_value());
        REQUIRE(std::begin(input) == std::begin(saved));
    }
    
    SECTION("Error") {
        parser<std::string> p = braced();

        std::string hello = "{hello";
        range input{hello.c_str(), hello.c_str() + hello.size()};
        range saved = input;

        auto result = p.run(input, &input);

        REQUIRE(!result.has_value());
        REQUIRE(std::begin(input) == std::end(saved));
    }

    SECTION("Lookahead") {
        parser<std::string> p = try_(braced());

        std::string hello = "{hello";
        range input{hello.c_str(), hello.c_str() + hello.size()};
        range saved = input;

        auto result = p.run(input, &input);

        REQUIRE(!result.has_value());
        REQUIRE(std::begin(input) == std::begin(saved));
    }
}

inline parser<char> delim() {
    return ask('a') || ask('b') || ask('c');
}

TEST_CASE("either") {

    std::string abcde = "abcde";
    range input{abcde.c_str(), abcde.c_str() + abcde.size()};

    REQUIRE(delim().run(input, &input).has_value());
    REQUIRE(delim().run(input, &input).has_value());
    REQUIRE(delim().run(input, &input).has_value());
    REQUIRE(!delim().run(input, &input).has_value());

}