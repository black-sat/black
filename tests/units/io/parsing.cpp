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

        success succ{str.c_str(), 42};

        STATIC_REQUIRE(std::same_as<decltype(succ), success<int>>);

        auto const&[p, ans] = succ;

        REQUIRE(p == str.c_str());
        REQUIRE(ans == 42);

        failure fail{str.c_str()};

        REQUIRE(get<0>(fail) == str.c_str());

        result<int> r = succ;

        REQUIRE(r.success() == succ);

        r = fail;

        REQUIRE(r.failure() == fail);
    }

    SECTION("Primitive combinators") {

        std::string s1 = "hello, world!";
        std::string s2 = "42, the answer!";
    
        SECTION("exactly()") {
            auto hello = exactly("hello");

            auto res1 = hello(s1);
            auto res2 = hello(s2);

            REQUIRE(res1.success());
            REQUIRE(res2.failure());
        }

        SECTION("pattern()") {
            auto id = pattern(&isalpha);

            auto res1 = id(s1);
            auto res2 = id(s2);

            REQUIRE(res1.success());
            REQUIRE(res2.failure());
        }

        SECTION("seq()") {

            auto he = seq(exactly("he"), exactly("llo"));

            auto res1 = he(s1);
            auto res2 = he(s2);

            REQUIRE(res1.success());
            REQUIRE(res2.failure());

        }

        SECTION("either()") {
            auto h4 = either(pattern(&isalpha), exactly("42"));

            auto res1 = h4(s1);
            auto res2 = h4(s2);

            REQUIRE(res1.success());
            REQUIRE(
                std::get<std::string_view>(res1.success()->output) == "hello"
            );
            REQUIRE(res2.success());
        }

        SECTION("apply()") {
            auto hello = apply(
                pattern(&isalpha),
                [](auto matched) {
                    return std::ranges::size(matched);
                }
            );

            auto res1 = hello(s1);
            auto res2 = hello(s2);

            REQUIRE(res1.success());
            REQUIRE(res1.success()->output == 5);
            REQUIRE(res2.failure());
        }

        SECTION("optional()") {
            
            std::string s3 = "@ hello, world!";

            auto hello = 
                seq(
                    optional(seq(pattern(&ispunct), exactly(" "))),
                    exactly("hello")
                );

            auto res1 = hello(s1);
            auto res3 = hello(s3);

            REQUIRE(res1.success());
            REQUIRE(!res1.success()->output.has_value());
            REQUIRE(res3.success());
            REQUIRE(res3.success()->output.has_value());
            REQUIRE(res3.success()->output == "@");
        }

        SECTION("many()") {

            std::string s3 = "hello;ciao;hi;halo;hola;";

            auto hello = many(seq(pattern(&isalpha), exactly(";")));

            auto res3 = hello(s3);

            REQUIRE(res3.success());

        }

    }

    SECTION("Complex examples") {

        auto parser =
            apply(
                seq(
                    either(exactly("hello"), exactly("mandi")),
                    exactly(", "), pattern(&isalpha), exactly("!")
                ),
                [&](std::string_view name) { return name == "world"; }
            );

        std::string s = GENERATE("hello, world!", "mandi, world!");

        auto res = parser(s);

        REQUIRE(res.success().has_value());
        REQUIRE(res.success()->output == true);

    }

    SECTION("sep_by()") {

        auto parser = sep_by(pattern(&isalpha), exactly(":"));

        std::string s = "one:two:three:four";

        auto res = parser(s);

        std::deque<std::string_view> expected = {
            "one", "two", "three", "four"
        };

        REQUIRE(res.success().has_value());
        REQUIRE(res.success()->output == expected);

    }

    // SECTION("optional() and spaces()") {

    //     struct func_t {
    //         bool is_static;
    //         std::string_view name;
    //         bool operator==(func_t const&) const = default;
    //     };

    //     auto keyword = [](std::string_view key){
    //         return trying(
    //             ignore(
    //                 token(
    //                     filter(string(&isalpha), [=](auto k){
    //                         return k == key;
    //                     })
    //                 )
    //             )
    //         );
    //     };

    //     auto parser =
    //         apply(
    //             seq(
    //                 optional(keyword("static")),
    //                 keyword("void"), string(&isalpha), exactly("();")
    //             ),
    //             [&](auto opt) {
    //                 return func_t{ st, name };
    //             }
    //         );

    //     SECTION("static void main();") {

    //         std::string s = "static void main();";

    //         auto res = parser(s.begin(), s.end());

    //         REQUIRE(res.success().has_value());
    //         REQUIRE(std::get<0>(res.success()->outputs) == func_t{true, "main"});

    //     }

    //     SECTION("void exit();") {
    //         std::string s = "void exit();";

    //         auto res = parser(s.begin(), s.end());

    //         REQUIRE(res.success().has_value());
    //         REQUIRE(
    //             std::get<0>(res.success()->outputs) == func_t{false, "exit"}
    //         );
    //     }

    // }

    // SECTION("CSV example") {
    //     size_t rows = 0;
    //     size_t fields = 0;

    //     auto field = apply(
    //         string([](char c) { return c != ',' && c != '\n'; }), 
    //         [&](auto...) { fields++; }
    //     );

    //     auto row = apply(
    //         sep_by(field, exactly(",")),
    //         [&](auto...) { rows++; }
    //     );

    //     auto csv = sep_by(row, exactly("\n"));

    //     std::string s = 
    //         "primo,field,del,csv\n"
    //         "secondo,field,del,csv";

    //     auto res = csv(s.begin(), s.end());

    //     REQUIRE(res.success());
    //     REQUIRE(rows == 2);
    //     REQUIRE(fields == 8);
    // }

}