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
#include <tuple>
#include <cctype>


TEST_CASE("Parsing") {
    
    using namespace black::parsing;

    auto s = succeed(42);
    auto f = fail();
    auto e = error("unexpected everything");
    auto q = element('q');
    auto a = element('a');
    
    auto qa = either(q, a);

    std::string msg1 = "question?";
    std::string msg2 = "answer!";
    std::string msg3 = "42!";

    REQUIRE(s(msg1));
    REQUIRE(s(msg1)->value == 42);

    REQUIRE(!f(msg1));

    REQUIRE(!e(msg1));
    REQUIRE(e(msg1).error().log.errors[0] == "unexpected everything");

    REQUIRE(q(msg1));
    REQUIRE(q(msg1)->value == 'q');

    REQUIRE(qa(msg1));
    REQUIRE(qa(msg1)->value == 'q');
    REQUIRE(qa(msg2));
    REQUIRE(qa(msg2)->value == 'a');
    REQUIRE(!qa(msg3));

    auto b = bind(qa, [](auto...vs) { return succeed(std::move(vs)...); });

    REQUIRE(b(msg1));
    REQUIRE(b(msg1)->value == 'q');

    auto Q = apply(q, [](char c) { return (char)toupper(c); } );

    REQUIRE(Q(msg1));
    REQUIRE(Q(msg1)->value == 'Q');

    auto ans = seq(element('a'), element('n'), element('s'));

    REQUIRE(ans(msg2));
    REQUIRE(ans(msg2)->value == 's');

}