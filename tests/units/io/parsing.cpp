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

#include <black/io>

#include <string>
#include <iostream>
#include <tuple>
#include <cctype>


TEST_CASE("Parsing") {
    
    using namespace black::io::parsing;


    SECTION("Basics") {

        auto parser =
            act(
                (string("hello") | string("mandi")) + 
                string(", ") + pattern(&isalpha) + string("!"), 
                [&](std::string_view name) { return name == "world"; }
            );

        std::string s = GENERATE("hello, world!", "mandi, world!");

        auto res = parser.parse(s.c_str(), s.c_str() + s.size());

        REQUIRE(res.success().has_value());
        REQUIRE(std::get<0>(res.success()->values) == true);

    }


    SECTION("Many") {

        auto sep_by = [](auto parser, auto sep) { 
            return act(
                parser + many(sep + parser),
                [](auto v, auto vs) {
                    vs.insert(begin(vs), v);
                    return vs;
                }
            );
        };

        auto parser = sep_by(pattern(&isalpha), string(":"));

        std::string s = "one:two:three:four";

        auto res = parser.parse(s.c_str(), s.c_str() + s.size());

        std::vector<std::string_view> expected = {
            "one", "two", "three", "four"
        };

        REQUIRE(res.success().has_value());
        REQUIRE(std::get<0>(res.success()->values) == expected);

    }

    SECTION("Optional and ignore") {

        auto spaces = [] {
            return ignore(pattern(&isspace));
        };

        struct func_t {
            bool is_static;
            std::string_view name;
            bool operator==(func_t const&) const = default;
        };

        auto parser =
            act(
                optional(string("static") + spaces()) + 
                string("void") + spaces() + pattern(&isalpha) + string("();"),
                [&](auto st, auto name) {
                    return func_t{ st, name };
                }
            );

        SECTION("static void main();") {

            std::string s = "static void main();";

            auto res = parser.parse(s.c_str(), s.c_str() + s.size());

            REQUIRE(res.success().has_value());
            REQUIRE(std::get<0>(res.success()->values) == func_t{true, "main"});

        }

        SECTION("void exit();") {
            std::string s = "void exit();";

            auto res = parser.parse(s.c_str(), s.c_str() + s.size());

            REQUIRE(res.success().has_value());
            REQUIRE(
                std::get<0>(res.success()->values) == func_t{false, "exit"}
            );
        }


    }

}