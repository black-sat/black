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

#include <black/support/common.hpp>

#include <string>
#include <tuple>
#include <unordered_map>

TEST_CASE("Hashing functions for tuples")
{
  using key_type = std::tuple<int, char, char>;

  std::unordered_map<key_type, int> map;

  key_type key = {42, 'a', 'z'};
  map.insert({key, 42});

  SECTION("Lookup of an existent key") {
    auto it = map.find(key);
    REQUIRE(it != map.end());
    REQUIRE(it->second == 42);
  }

  SECTION("Lookup of a non-existent key") {
    key_type nkey = {0, 'A', 'Z'};
    auto it = map.find(nkey);

    REQUIRE(it == map.end());
  }
}
