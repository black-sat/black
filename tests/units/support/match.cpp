//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#include <variant>
#include <string>

using namespace black::support;

struct test : black_union_type(int, std::tuple<std::string, float>);

struct nil { };

template<typename T>
struct cons;

template<typename T>
struct list : black_rec_union_type(nil, cons<T>);

template<typename T>
struct cons {

  cons(T h) : head{h}, tail{nil{}} { }
  cons(T h, list<T> t) : head{h}, tail{t} { }

  T head;
  list<T> tail;
};

TEST_CASE("Match infrastructure") {

  test t = 21;

  STATIC_REQUIRE(matchable<test>);
  STATIC_REQUIRE(!matchable<int>);

  auto b = t.match(
    [](int x) {
      return x * 2;
    },
    [](std::tuple<std::string, float>, std::string, float) {
      return false;
    }
  );

  STATIC_REQUIRE(std::is_same_v<decltype(b), int>);

  REQUIRE(b == 42);

}

TEST_CASE("Union types") {
  test t = 21;

  test t2 = t; // copy ctor

  t2 = t; // assignment

  REQUIRE(t2 == t); // equality

  // hashing
  size_t h1 = hash(t);
  size_t h2 = hash(21);

  REQUIRE(h1 == h2);
}

TEST_CASE("Recursive union types") {
  list<int> l = cons(21);

  list<int> l2 = cons(42, l);

  std::vector<int> values;
  std::function<void(list<int>)> f = [&](list<int> arg) {
    arg.match(
      [&](cons<int> c) {
        values.push_back(c.head);
        f(c.tail);
      },
      [](nil) { }
    );
  };

  f(l2);

  REQUIRE(values == std::vector{42, 21});
}