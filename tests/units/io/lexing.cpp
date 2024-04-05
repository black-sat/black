//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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

struct my_parselet {

    static bool is_keyword(std::string_view str) {
        return str == "hi";
    }

    static bool is_symbol(std::string_view str) {
        return str == "<=" || str == "<" || str == "," || str == ":";
    }
};


TEST_CASE("Lexer") {

    using namespace black::io;

    std::string test = "hello: 12 < 34, hi: 45.6e-2 <= 45.e";

    buffer buf{test};
    my_parselet p;

    REQUIRE(lex(&buf, p) == token::identifier{"hello"});

    std::vector<token> toks;
    token tok;
    while((tok = lex(&buf, p)))
        toks.push_back(tok);

    std::vector<token> expected = {
        token::punctuation{":"},
        token::integer{12},
        token::punctuation{"<"},
        token::integer{34},
        token::punctuation{","},
        token::keyword{"hi"},
        token::punctuation{":"},
        token::real{45.6e-2},
        token::punctuation{"<="}
    };

    REQUIRE(toks == expected);

    REQUIRE(tok == token::invalid{"45.e"});

    REQUIRE(lex(&buf, p) == token::eof{});

}