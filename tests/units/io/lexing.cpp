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

    using namespace black;

    std::string test = "hello: 12 < 34, hi: 45.6e-2 <= 45.e";

    io::buffer buf{test};
    my_parselet p;

    REQUIRE(io::lex(&buf, p) == io::token::identifier{"hello"});

    std::vector<io::token> toks;
    io::token tok;
    while((tok = io::lex(&buf, p)))
        toks.push_back(tok);

    std::vector<io::token> expected = {
        io::token::punctuation{":"},
        io::token::integer{12},
        io::token::punctuation{"<"},
        io::token::integer{34},
        io::token::punctuation{","},
        io::token::keyword{"hi"},
        io::token::punctuation{":"},
        io::token::real{45.6e-2},
        io::token::punctuation{"<="}
    };

    REQUIRE(toks == expected);

    REQUIRE(tok == io::token::invalid{"45.e"});

    REQUIRE(io::lex(&buf, p) == io::token::eof{});

}