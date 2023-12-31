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

using namespace black::logic;
using namespace black::logic::reflect;

struct C {
    int pippo() const { return x * 2; }
    int pluto(int y) { return y * 2; }

    int x = 21;
};

struct test : C, 
    term_field_member_base<
        test,
        term, terms_enum_t<term>::integer,
        term_fields_enum_t<term, terms_enum_t<term>::integer>::value,
        &C::pippo
    >, 
    term_member_base<
        test,
        term, terms_enum_t<term>::integer,
        &C::pluto
    >
    { };


TEST_CASE("Static reflection") {

    REQUIRE(
        test{}.value() == 42
    );
    
    REQUIRE(
        test{}.integer(21) == 42
    );

    STATIC_REQUIRE(
        std::is_same_v<
            term_type_t<term, terms_enum_t<term>::integer>,
            integer
        >
    );

    REQUIRE(
        term_field_name_v<
            term, 
            terms_enum_t<term>::integer, 
            term_fields_enum_t<term, terms_enum_t<term>::integer>::value
        > == "value"
    );

    STATIC_REQUIRE(
        std::is_same_v<
            term_field_type_t<
                term, 
                terms_enum_t<term>::integer, 
                term_fields_enum_t<term, terms_enum_t<term>::integer>::value
            >,
            int64_t
        >
    );

}