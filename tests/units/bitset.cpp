//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#include <black/support/bitset.hpp>

#include <iostream>

constexpr size_t N = 70;

static constexpr black::bitset<N> test1() {
  black::bitset<N> b = {10, 42};

  return b;
}

static constexpr black::bitset<N> test2() {
  black::bitset<N> b1 = {10, 42};
  black::bitset<N> b2 = {55};

  return b1 | b2;
}

static_assert(test2().contains(test1()));

TEST_CASE("black::bitset<N>") 
{
  black::bitset<N> b;

  for(size_t i = 0; i < N; ++i) {
    REQUIRE(!b.contains(i));
  }

  REQUIRE(b == black::bitset<N>{});

  SECTION("Set") {
    b.set(42);

    for(size_t i = 0; i < N; ++i) {
      if(i == 42)
        REQUIRE(b.contains(i));
      else
        REQUIRE(!b.contains(i));
    }
  }

  SECTION("Reset") {
    b.set(42);
    b.set(10);

    b.reset(10);

    for(size_t i = 0; i < N; ++i) {
      if(i == 42)
        REQUIRE(b.contains(i));
      else
        REQUIRE(!b.contains(i));
    }
  }
  
  SECTION("contains") {
    black::bitset<N> b2;

    b.set(42);
    b.set(10);
    b2.set(10);

    REQUIRE(b.contains(b2));
    REQUIRE(!b2.contains(b));
  }

  SECTION("bitwise ops") {
    black::bitset<N> b2;
    black::bitset<N> b3;

    b.set(42);
    b.set(10);
    b2.set(42);
    b3.set(10);

    REQUIRE(b == (b2 | b3));
    REQUIRE(black::bitset<N>{} == (b2 & b3));
  }

  SECTION("negation") {
    black::bitset<N> b2;
    black::bitset<N> b3;

    for(size_t i = 0; i < N; ++i) {
      b2.set(i);
    }
    b2.reset(42);
    b3.set(42);

    REQUIRE(~b2 == b3);
  }

  SECTION("Initializer list constructor") {
    black::bitset<N> b2 = {1, 2, 3, 5, 7};

    REQUIRE(b2.contains(1));
    REQUIRE(b2.contains(2));
    REQUIRE(b2.contains(3));
    REQUIRE(b2.contains(5));
    REQUIRE(b2.contains(7));
    REQUIRE(!b2.contains(8));
  }

}
