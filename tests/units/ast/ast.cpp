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

#include <black/logic>
#include <black/ast/algorithms>

#include <string_view>

TEST_CASE("Static reflection") {

    using namespace black::support;
    using namespace black::ast::core;
    using namespace black::logic;

    STATIC_REQUIRE(is_ast_v<term>);
    STATIC_REQUIRE(is_ast_node_of_v<integer, term>);

    STATIC_REQUIRE(
        std::to_underlying(ast_node_index_t<term>::error) == 0
    );

    STATIC_REQUIRE(
        std::to_underlying(ast_node_field_index_t<term, variable>::name) == 0
    );

    STATIC_REQUIRE(
        std::to_underlying(ast_node_index_of_v<term, black::logic::error>) == 0
    );

    STATIC_REQUIRE(
        std::is_same_v<
            ast_node_field_list_t<term, variable>,
            std::tuple<
                std::integral_constant<
                    ast_node_field_index_t<term, variable>,
                    ast_node_field_index_t<term, variable>::name
                >
            >
        >
    );

    STATIC_REQUIRE(
        std::is_same_v<
            ast_node_field_type_t<
                term, variable, ast_node_field_index_t<term, variable>::name
            >,
            label
        >
    );

    STATIC_REQUIRE(
        ast_node_field_name_v<
            term, variable, ast_node_field_index_t<term, variable>::name
        > == "name"
    );

    STATIC_REQUIRE(
        std::is_same_v<
            ast_node_field_types_t<term, variable>,
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
        ast_node_member_base<test, term, variable, &base::test1>,
        ast_node_field_member_base<
            test,
            term, variable, ast_node_field_index_t<term, variable>::name, 
            &base::test2
        > { };

    REQUIRE(test{}.variable(21) == 42);
    REQUIRE(test{}.name() == 42);

    integer p = 21;
    integer q = 42;

    auto c = conjunction(std::vector<term>{p, q});

    term t1 = p;
    term t2 = p;

    REQUIRE(t1 == t2);

    REQUIRE(c.arguments()[0] == p);

    difference u = difference(p, q);
    term t = u;

    REQUIRE(cast<difference>(t).has_value());
    REQUIRE(cast<difference>(t) == u);
    REQUIRE(cast<difference>(t)->left() == p);

    auto result = match(t)(
        [&](difference, term left, term) {
            REQUIRE(left == p);
            return cast<integer>(left)->value() * 2;
        }
    );

    REQUIRE(result == 42);

    REQUIRE((t == t));

    term t3 = t == t;

    auto r = match(u)(
        [](difference) { return 42; }
    );

    REQUIRE(r == 42);

    std::unordered_map<term, int> m = {{t, 42}};

    REQUIRE(m[t] == 42);

}

TEST_CASE("Traverse") {

    using namespace black::logic;
    using namespace black::ast;

    tree_ast tree = node(1, {leaf(), node(2, {leaf(), leaf()})});

    size_t sum = traverse<size_t>(tree)(
        [](leaf) { return 0; },
        [](node, size_t value, auto children) { 
            return value + std::reduce(children.begin(), children.end()); 
        }
    );

    REQUIRE(sum == 3);

}