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

#include <black/parsing>

#include <string>
#include <iostream>
#include <tuple>
#include <cctype>


TEST_CASE("Parsing") {
    
    using namespace black::parsing;


    SECTION("Basics") {
        std::string str = "hello";

        success succ{begin(str), 42, 3.14};

        STATIC_REQUIRE(
            std::same_as<
                decltype(succ), 
                success<std::string::iterator, int, double>
            >
        );

        auto [p, ans, pi] = succ;

        REQUIRE(p == str.begin());
        REQUIRE(ans == 42);
        REQUIRE(pi == 3.14);

        failure fail{str.begin()};

        REQUIRE(get<0>(fail) == str.begin());

        result<std::string::iterator, int, double> r = succ;

        REQUIRE(r.success() == succ);

        r = fail;

        REQUIRE(r.failure() == fail);
    }

    SECTION("Primitive combinators") {

        std::string s1 = "hello, world!";
        std::string s2 = "42, the answer!";

        SECTION("symbol()") {
            auto h = symbol('h');

            auto res1 = h(s1.begin(), s1.end());
            auto res2 = h(s2.begin(), s2.end());

            REQUIRE(res1.success());
            REQUIRE(*res1.success()->current == 'e');
            REQUIRE(res2.failure());
        }
    
        SECTION("pattern()") {
            auto id = pattern(&isalpha);

            auto res1 = id(s1.begin(), s1.end());
            auto res2 = id(s2.begin(), s2.end());

            REQUIRE(res1.success());
            REQUIRE(res2.failure());
        }

        SECTION("seq()") {

            auto he = seq(symbol('h'), symbol('e'));

            auto res1 = he(s1.begin(), s1.end());
            auto res2 = he(s2.begin(), s2.end());

            REQUIRE(res1.success());
            REQUIRE(res2.failure());

        }

        SECTION("either()") {
            auto h4 = either(symbol('h'), symbol('4'));

            auto res1 = h4(s1.begin(), s1.end());
            auto res2 = h4(s2.begin(), s2.end());

            REQUIRE(res1.success());
            REQUIRE(res2.success());
        }

        SECTION("exactly()") {
            auto hello = exactly("hello");

            auto res1 = hello(s1.begin(), s1.end());
            auto res2 = hello(s2.begin(), s2.end());

            REQUIRE(res1.success());
            REQUIRE(res2.failure());
        }

        SECTION("apply()") {
            auto hello = apply(
                pattern(&isalpha),
                [](auto matched) {
                    return std::ranges::size(matched);
                }
            );

            auto res1 = hello(s1.begin(), s1.end());
            auto res2 = hello(s2.begin(), s2.end());

            REQUIRE(res1.success());
            REQUIRE(std::get<0>(res1.success()->values) == 5);
            REQUIRE(res2.failure());
        }

    }

    SECTION("Derived combinators") {
        
        std::string s1 = "hello, world!";
        std::string s2 = "42, the answer!";

        SECTION("string()") {
            auto hello = string(&isalpha);

            auto res1 = hello(s1.begin(), s1.end());
            auto res2 = hello(s2.begin(), s2.end());

            REQUIRE(res1.success());
            REQUIRE(std::get<0>(res1.success()->values) == "hello");
            REQUIRE(res2.failure());
        }

    }


    SECTION("Complex examples") {

        auto parser =
            apply(
                seq(
                    either(exactly("hello"), exactly("mandi")),
                    exactly(", "), string(&isalpha), exactly("!")
                ),
                [&](std::string_view name) { return name == "world"; }
            );

        std::string s = GENERATE("hello, world!", "mandi, world!");

        auto res = parser(s.begin(), s.end());

        REQUIRE(res.success().has_value());
        REQUIRE(std::get<0>(res.success()->values) == true);

    }


    SECTION("sep_by() (and therefore many())") {

        auto parser = sep_by(string(&isalpha), exactly(":"));

        std::string s = "one:two:three:four";

        auto res = parser(s.begin(), s.end());

        std::vector<std::string_view> expected = {
            "one", "two", "three", "four"
        };

        REQUIRE(res.success().has_value());
        REQUIRE(std::get<0>(res.success()->values) == expected);

    }

    SECTION("optional() and spaces()") {

        struct func_t {
            bool is_static;
            std::string_view name;
            bool operator==(func_t const&) const = default;
        };

        auto keyword = [](std::string_view key){
            return trying(
                ignore(
                    token(
                        filter(string(&isalpha), [=](auto k){
                            return k == key;
                        })
                    )
                )
            );
        };

        auto parser =
            apply(
                seq(
                    optional(keyword("static")),
                    keyword("void"), string(&isalpha), exactly("();")
                ),
                [&](bool st, auto name) {
                    return func_t{ st, name };
                }
            );

        SECTION("static void main();") {

            std::string s = "static void main();";

            auto res = parser(s.begin(), s.end());

            REQUIRE(res.success().has_value());
            REQUIRE(std::get<0>(res.success()->values) == func_t{true, "main"});

        }

        SECTION("void exit();") {
            std::string s = "void exit();";

            auto res = parser(s.begin(), s.end());

            REQUIRE(res.success().has_value());
            REQUIRE(
                std::get<0>(res.success()->values) == func_t{false, "exit"}
            );
        }


    }

}