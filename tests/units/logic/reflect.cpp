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

#include <black/support.hpp>
#include <black/logic.hpp>

using namespace black::reflect;
using namespace black::logic;

#include <string_view>

TEST_CASE("Static reflection") {

    STATIC_REQUIRE(is_ast_v<term>);
    STATIC_REQUIRE(is_ast_node_of_v<integer, term>);

    STATIC_REQUIRE(
        std::is_same_v<
            ast_nodes_t<term>,
            std::tuple<integer, symbol, boolean, conjunction>
        >
    );

    STATIC_REQUIRE(
        std::to_underlying(ast_node_field_t<term, symbol>::name) == 0
    );

    STATIC_REQUIRE(
        std::is_same_v<
            ast_node_field_list_t<term, symbol>,
            std::tuple<
                std::integral_constant<
                    ast_node_field_t<term, symbol>,
                    ast_node_field_t<term, symbol>::name
                >
            >
        >
    );

    STATIC_REQUIRE(
        std::is_same_v<
            ast_node_field_type_t<
                term, symbol, ast_node_field_t<term, symbol>::name
            >,
            label
        >
    );

    STATIC_REQUIRE(
        ast_node_field_name_v<
            term, symbol, ast_node_field_t<term, symbol>::name
        > == "name"
    );

    STATIC_REQUIRE(
        std::is_same_v<
            ast_node_field_types_t<term, symbol>,
            std::tuple<label>
        >
    );

    struct base {
        int test1(int y) { return y * 2; }
        int test2() const { return x * 2; }

        int x = 21;
    };

    struct test 
        : base,
        ast_node_member_base<test, term, symbol, &base::test1>,
        ast_node_field_member_base<
            test,
            term, symbol, ast_node_field_t<term, symbol>::name, &base::test2
        > { };

    REQUIRE(test{}.symbol(21) == 42);
    REQUIRE(test{}.name() == 42);

}