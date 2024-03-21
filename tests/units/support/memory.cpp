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

#include <black/support>

#include <memory>

struct test_t : std::enable_shared_from_this<test_t> {
    int x;

    test_t(int x) : x{x} { }

    test_t(test_t const&) = default;
    test_t(test_t &&) = default;
    
    test_t &operator=(test_t const&) = default;
    test_t &operator=(test_t &&) = default;

    bool operator==(test_t const& t) const {
        return x == t.x;
    }
};

TEST_CASE("wrap_ptr") {

    using namespace black::support;

    SECTION("pointer") {
        auto sptr = std::make_shared<test_t>(42);
        
        wrap_ptr<test_t> p = sptr;

        REQUIRE(p.shared() == sptr);
        REQUIRE(*p == *sptr);

        test_t zero = 0;
        *p = zero;
        REQUIRE(*p == zero);

        p->x = 42;

        REQUIRE(p->x == 42);

        p = sptr.get();

        REQUIRE(p.shared() == sptr);
        REQUIRE(*p == *sptr);

    }
    
    SECTION("pointer to const") {
        auto sptr = std::make_shared<test_t const>(42);
        
        wrap_ptr<test_t const> p = sptr;

        REQUIRE(p.shared() == sptr);

        REQUIRE(p->x == 42);

        p = sptr.get();

        REQUIRE(p.shared() == sptr);
        REQUIRE(*p == *sptr);

        wrap_ptr<test_t> p2 = std::make_shared<test_t>(0);

        p = p2;

        REQUIRE(p->x == 0);

    }


}