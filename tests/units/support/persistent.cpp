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

#include <black/support/private>

#include <string>

TEST_CASE("persistent vector") {

    using namespace black::support::persistent;

    vector<int> v = { 1, 2, 3 };

    v.push_back(4);

    REQUIRE(v == vector<int>{1, 2, 3, 4});

    v.update(v.size() - 1, [](int x) {
        return x + 1;
    });

    REQUIRE(v == vector<int>{1, 2, 3, 5});

    v.take(3);

    REQUIRE(v == vector<int>{1, 2, 3});

}

TEST_CASE("persistent map") {

    using namespace black::support::persistent;

    map<std::string, int> v = { {"hello", 1}, {"ciao", 2}, {"hola", 3} };

    v.insert({"halo", 4});

    REQUIRE(
        v == map<std::string, int>{
            {"hello", 1}, {"ciao", 2}, {"hola", 3}, {"halo", 4}
        }
    );

    v.update("halo", [](int x) {
        return x + 1;
    });

    REQUIRE(
        v == map<std::string, int>{
            {"hello", 1}, {"ciao", 2}, {"hola", 3}, {"halo", 5}
        }
    );

    v.erase("halo");

    REQUIRE(
        v == map<std::string, int>{{"hello", 1}, {"ciao", 2}, {"hola", 3}}
    );

}

TEST_CASE("persistent set") {

    using namespace black::support::persistent;

    set<int> v = { 1, 2, 3 };

    v.insert(4);

    REQUIRE(v == set<int>{1, 2, 3, 4});

    v.erase(4);

    REQUIRE(v == set<int>{1, 2, 3});

}
