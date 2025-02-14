//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2025 Nicola Gigante
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

#include <print>
#include <cctype>

using namespace black::parsing;

inline parser<std::string> braced() {
    return [] -> parsed<std::string> {
        co_await chr('{');
        
        std::string result;
        std::optional<char> c;
        while((c = co_await optional(peek(&isalpha)))) {
            result.push_back(*c);
            co_await advance();
        }

        co_await expect('}');

        co_return result;
    };
}

TEST_CASE("Very basic") {
    parser<char> p = choice(peek('c'), pass('0'));

    std::string hello = "d";
    auto result = 
        p.parse(range{hello.c_str(), hello.c_str() + hello.size()});

    REQUIRE(result.has_value());
    REQUIRE(result == '0');
    //REQUIRE(!result->has_value());
}

TEST_CASE("Basic operations on parsers") 
{
    parser<std::string> p = braced();

    SECTION("Found") {
        std::string hello = "{hello}";
        auto result = 
            p.parse(range{hello.c_str(), hello.c_str() + hello.size()});

        REQUIRE(result.has_value());
        REQUIRE(*result == "hello");
    }

    SECTION("Not found") {
        std::string mandi = "{mandi";
        auto result = 
            p.parse(range{mandi.c_str(), mandi.c_str() + mandi.size()});

        REQUIRE(!result.has_value());
    }

}

inline parser<std::string> opt_test() {
    return [] -> parsed<std::string> {
        auto opt = co_await optional(chr('{'));
        if(opt)
            co_return "Ok!";
        
        co_return "Nope!";
    };
}

TEST_CASE("Optional") {

    parser<std::string> p = opt_test();

    SECTION("Ok") {
        std::string hello = "{hello}";
        auto result = 
            p.parse(range{hello.c_str(), hello.c_str() + hello.size()});

        REQUIRE(result == "Ok!");
    }

    SECTION("Nope") {
        std::string mandi = "[mandi]";
        auto result = 
            p.parse(range{mandi.c_str(), mandi.c_str() + mandi.size()});

        REQUIRE(result == "Nope!");
    }

}

TEST_CASE("Lookaehad") {

    SECTION("Skip") {
        parser<std::string> p = braced();

        std::string hello = "[hello]";
        range input{hello.c_str(), hello.c_str() + hello.size()};
        range saved = input;

        auto result = p.parse(input, &input);

        REQUIRE(!result.has_value());
        REQUIRE(std::begin(input) == std::begin(saved));
    }
    
    SECTION("Error") {
        parser<std::string> p = braced();

        std::string hello = "{hello";
        range input{hello.c_str(), hello.c_str() + hello.size()};
        range saved = input;

        auto result = p.parse(input, &input);

        REQUIRE(!result.has_value());
        REQUIRE(std::begin(input) == std::end(saved));
    }

    SECTION("Lookahead") {
        parser<std::string> p = try_(braced());

        std::string hello = "{hello";
        range input{hello.c_str(), hello.c_str() + hello.size()};
        range saved = input;

        auto result = p.parse(input, &input);

        REQUIRE(!result.has_value());
        REQUIRE(std::begin(input) == std::begin(saved));
    }
}

inline parser<char> letters() {
    return chr('a') | chr('b') | chr('c');
}

TEST_CASE("either") {

    std::string abcde = "abcde";
    range input{abcde.c_str(), abcde.c_str() + abcde.size()};

    parser<char> p = letters();

    REQUIRE(p.parse(input, &input).has_value());
    REQUIRE(p.parse(input, &input).has_value());
    REQUIRE(p.parse(input, &input).has_value());
    REQUIRE(!p.parse(input, &input).has_value());

}

TEST_CASE("yielding parsers") {
    
    std::string abcde = "abcde4";
    range input{abcde.c_str(), abcde.c_str() + abcde.size()};

    parser<char[]> p = some(chr(&isalpha));

    auto result = p.parse(input, &input);

    REQUIRE(result.has_value());
    REQUIRE(result == std::vector{'a', 'b', 'c', 'd', 'e'});
    REQUIRE(*std::begin(input) == '4');

}

TEST_CASE("many with empty input") {
    std::string empty = "";
    range input{empty.c_str(), empty.c_str() + empty.size()};

    parser<size_t[]> p = many(integer());

    auto result = p.parse(input, &input);

    REQUIRE(result.has_value());
    REQUIRE(result->empty());
}

TEST_CASE("skip many") {

    parser<void> p = skip( chr('{') + skip_many(chr(&isalpha)) + chr('}') );

    std::string hello = "{hello}";
    range input{hello.c_str(), hello.c_str() + hello.size()};

    auto result = p.parse(input);

    REQUIRE(result.has_value());

}

TEST_CASE("pattern string") {
    std::string num = "answer";
    range input{num.c_str(), num.c_str() + num.size()};

    parser<std::string> p = string("answer");
    
    auto result = p.parse(input, &input);

    REQUIRE(result.has_value());
    REQUIRE(*result == "answer");
}

TEST_CASE("Operations on sequences") {

    std::string any = "42";
    range input{any.c_str(), any.c_str() + any.size()};

    std::vector<int> v = {1, 2, 3, 4};
    
    SECTION("Transform") {
        parser<int[]> p = transform(yield(v), [](auto i) { return i * 2; });
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{2, 4, 6, 8});
    }
    
    SECTION("Filter") {
        parser<int[]> p = filter(yield(v), [](auto i) { return i % 2 == 0; });
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{2, 4});
    }
    
    SECTION("Take") {
        parser<int[]> p = take(yield(v), 2);
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{1, 2});
    }
    
    SECTION("Take while") {
        parser<int[]> p = take_while(yield(v), [](auto i) { return i <= 2; });
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{1, 2});
    }
    
    SECTION("Drop") {
        parser<int[]> p = drop(yield(v), 2);
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{3, 4});
    }
    
    SECTION("Drop while") {
        parser<int[]> p = drop_while(yield(v), [](auto i) { return i <= 2; });
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{3, 4});
    }
    
    SECTION("Concat") {
        std::vector<int> v1 = { 1, 2 };
        std::vector<int> v2 = { 3 };
        std::vector<int> v3 = { 4 };

        parser<int[]> p = concat(yield(v1), yield(v2), yield(v3));
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == v);
    }

    SECTION("Zip") {
        std::vector<int> v1 = { 1, 2 };
        std::vector<int> v2 = { 3, 4 };

        parser<std::tuple<int, int>[]> p = zip(yield(v1), yield(v2));
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{std::tuple{1, 3}, std::tuple{2, 4}});
    }
    
    SECTION("Index") {
        parser<int> p = index<0>(pass(std::tuple{1, 2}));
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == 1);
    }
    
    SECTION("Index on sequences") {
        std::vector<std::tuple<int, int>> v1 = { {1, 2}, {3, 4} };

        parser<int[]> p = index<0>(yield(v1));
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{1, 3});
    }
    
    SECTION("sep_many") {
        parser<size_t[]> p = sep_many(token(integer()), token(chr(',')));

        std::string s = "1, 2 , 3, 4";
        input = range{s.c_str(), s.c_str() + s.size()};

        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result == std::vector{1uz, 2uz, 3uz, 4uz});
    }
    
    SECTION("sep_many with empty input") {
        std::string s = "";
        input = range{s.c_str(), s.c_str() + s.size()};

        parser<size_t[]> p = sep_many(token(integer()), token(chr(',')));
        auto result = p.parse(input);

        REQUIRE(result.has_value());
        REQUIRE(result->empty());
    }

}

TEST_CASE("integers, strings, identifiers") {
    std::string num = "answer: 42";
    range input{num.c_str(), num.c_str() + num.size()};

    parser<size_t> p = 
        identifier("answer") + chr(':') + token(integer());
    
    auto number = p.parse(input, &input);

    REQUIRE(number.has_value());
    REQUIRE(*number == 42);
    REQUIRE(input.empty());
}

TEST_CASE("resumability") {
    parser<size_t> p = [] -> parsed<size_t> {
        co_await identifier("answer");
        co_await chr(':');
        co_return co_await integer();
    };
    context<size_t> c = p.start();
    
    std::string in1 = "answer:";
    range r1{in1.c_str(), in1.c_str() + in1.size()};

    std::string in2 = "42";
    range r2{in2.c_str(), in2.c_str() + in2.size()};
    
    auto number = c.parse(r1, &r1);

    REQUIRE(!number.has_value());
    REQUIRE(number.error() == failure::eof);
    REQUIRE(!r1);

    std::println("Second parse");
    number = c.parse(r2, &r2);

    REQUIRE(number.has_value());
    REQUIRE(*number == 42);
    REQUIRE(!r2);
}