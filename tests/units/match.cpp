//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#include <black/logic/formula.hpp>
#include <black/logic/alphabet.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>

#include <type_traits>

using namespace black;

//
// TODO: better testing of matching facility
//

template<typename T1, typename T2, typename R>
constexpr bool check = std::is_same_v<std::common_type_t<T1, T2>, R>;

static_assert(check<boolean, proposition,  formula>);
static_assert(check<boolean, boolean,      boolean>);
static_assert(check<boolean, negation,     formula>);
static_assert(check<boolean, tomorrow,     formula>);
static_assert(check<boolean, w_tomorrow,   formula>);
static_assert(check<boolean, yesterday,    formula>);
static_assert(check<boolean, w_yesterday,  formula>);
static_assert(check<boolean, always,       formula>);
static_assert(check<boolean, eventually,   formula>);
static_assert(check<boolean, once,         formula>);
static_assert(check<boolean, historically, formula>);
static_assert(check<boolean, conjunction,  formula>);
static_assert(check<boolean, disjunction,  formula>);
static_assert(check<boolean, implication,  formula>);
static_assert(check<boolean, iff,          formula>);
static_assert(check<boolean, until,        formula>);
static_assert(check<boolean, release,      formula>);
static_assert(check<boolean, since,        formula>);
static_assert(check<boolean, triggered,    formula>);

static_assert(check<proposition, proposition,  proposition>);
static_assert(check<proposition, boolean,      formula>);
static_assert(check<proposition, negation,     formula>);
static_assert(check<proposition, tomorrow,     formula>);
static_assert(check<proposition, w_tomorrow,   formula>);
static_assert(check<proposition, yesterday,    formula>);
static_assert(check<proposition, w_yesterday,  formula>);
static_assert(check<proposition, always,       formula>);
static_assert(check<proposition, eventually,   formula>);
static_assert(check<proposition, once,         formula>);
static_assert(check<proposition, historically, formula>);
static_assert(check<proposition, conjunction,  formula>);
static_assert(check<proposition, disjunction,  formula>);
static_assert(check<proposition, implication,  formula>);
static_assert(check<proposition, iff,          formula>);
static_assert(check<proposition, until,        formula>);
static_assert(check<proposition, release,      formula>);
static_assert(check<proposition, since,        formula>);
static_assert(check<proposition, triggered,    formula>);

static_assert(check<tomorrow, boolean,      formula>);
static_assert(check<tomorrow, negation,     unary>);
static_assert(check<tomorrow, tomorrow,     tomorrow>);
static_assert(check<tomorrow, w_tomorrow,   unary>);
static_assert(check<tomorrow, yesterday,    unary>);
static_assert(check<tomorrow, w_yesterday,  unary>);
static_assert(check<tomorrow, always,       unary>);
static_assert(check<tomorrow, eventually,   unary>);
static_assert(check<tomorrow, once,         unary>);
static_assert(check<tomorrow, historically, unary>);
static_assert(check<tomorrow, conjunction,  formula>);
static_assert(check<tomorrow, disjunction,  formula>);
static_assert(check<tomorrow, implication,  formula>);
static_assert(check<tomorrow, iff,          formula>);
static_assert(check<tomorrow, until,        formula>);
static_assert(check<tomorrow, release,      formula>);
static_assert(check<tomorrow, since,        formula>);
static_assert(check<tomorrow, triggered,    formula>);

static_assert(check<w_tomorrow, boolean,      formula>);
static_assert(check<w_tomorrow, negation,     unary>);
static_assert(check<w_tomorrow, tomorrow,     unary>);
static_assert(check<w_tomorrow, w_tomorrow,   w_tomorrow>);
static_assert(check<w_tomorrow, yesterday,    unary>);
static_assert(check<w_tomorrow, w_yesterday,  unary>);
static_assert(check<w_tomorrow, always,       unary>);
static_assert(check<w_tomorrow, eventually,   unary>);
static_assert(check<w_tomorrow, once,         unary>);
static_assert(check<w_tomorrow, historically, unary>);
static_assert(check<w_tomorrow, conjunction,  formula>);
static_assert(check<w_tomorrow, disjunction,  formula>);
static_assert(check<w_tomorrow, implication,  formula>);
static_assert(check<w_tomorrow, iff,          formula>);
static_assert(check<w_tomorrow, until,        formula>);
static_assert(check<w_tomorrow, release,      formula>);
static_assert(check<w_tomorrow, since,        formula>);
static_assert(check<w_tomorrow, triggered,    formula>);

static_assert(check<yesterday, proposition,  formula>);
static_assert(check<yesterday, boolean,      formula>);
static_assert(check<yesterday, negation,     unary>);
static_assert(check<yesterday, tomorrow,     unary>);
static_assert(check<yesterday, w_tomorrow,   unary>);
static_assert(check<yesterday, yesterday,    yesterday>);
static_assert(check<yesterday, w_yesterday,  unary>);
static_assert(check<yesterday, always,       unary>);
static_assert(check<yesterday, eventually,   unary>);
static_assert(check<yesterday, once,         unary>);
static_assert(check<yesterday, historically, unary>);
static_assert(check<yesterday, conjunction,  formula>);
static_assert(check<yesterday, disjunction,  formula>);
static_assert(check<yesterday, implication,  formula>);
static_assert(check<yesterday, iff,          formula>);
static_assert(check<yesterday, until,        formula>);
static_assert(check<yesterday, release,      formula>);
static_assert(check<yesterday, since,        formula>);
static_assert(check<yesterday, triggered,    formula>);

static_assert(check<w_yesterday, proposition,  formula>);
static_assert(check<w_yesterday, boolean,      formula>);
static_assert(check<w_yesterday, negation,     unary>);
static_assert(check<w_yesterday, tomorrow,     unary>);
static_assert(check<w_yesterday, w_tomorrow,   unary>);
static_assert(check<w_yesterday, yesterday,    unary>);
static_assert(check<w_yesterday, w_yesterday,  w_yesterday>);
static_assert(check<w_yesterday, always,       unary>);
static_assert(check<w_yesterday, eventually,   unary>);
static_assert(check<w_yesterday, once,         unary>);
static_assert(check<w_yesterday, historically, unary>);
static_assert(check<w_yesterday, conjunction,  formula>);
static_assert(check<w_yesterday, disjunction,  formula>);
static_assert(check<w_yesterday, implication,  formula>);
static_assert(check<w_yesterday, iff,          formula>);
static_assert(check<w_yesterday, until,        formula>);
static_assert(check<w_yesterday, release,      formula>);
static_assert(check<w_yesterday, since,        formula>);
static_assert(check<w_yesterday, triggered,    formula>);

static_assert(check<always, proposition,  formula>);
static_assert(check<always, boolean,      formula>);
static_assert(check<always, negation,     unary>);
static_assert(check<always, tomorrow,     unary>);
static_assert(check<always, w_tomorrow,   unary>);
static_assert(check<always, yesterday,    unary>);
static_assert(check<always, w_yesterday,  unary>);
static_assert(check<always, always,       always>);
static_assert(check<always, eventually,   unary>);
static_assert(check<always, once,         unary>);
static_assert(check<always, historically, unary>);
static_assert(check<always, conjunction,  formula>);
static_assert(check<always, disjunction,  formula>);
static_assert(check<always, implication,  formula>);
static_assert(check<always, iff,          formula>);
static_assert(check<always, until,        formula>);
static_assert(check<always, release,      formula>);
static_assert(check<always, since,        formula>);
static_assert(check<always, triggered,    formula>);

static_assert(check<eventually, proposition,  formula>);
static_assert(check<eventually, boolean,      formula>);
static_assert(check<eventually, negation,     unary>);
static_assert(check<eventually, tomorrow,     unary>);
static_assert(check<eventually, w_tomorrow,   unary>);
static_assert(check<eventually, yesterday,    unary>);
static_assert(check<eventually, w_yesterday,  unary>);
static_assert(check<eventually, always,       unary>);
static_assert(check<eventually, eventually,   eventually>);
static_assert(check<eventually, once,         unary>);
static_assert(check<eventually, historically, unary>);
static_assert(check<eventually, conjunction,  formula>);
static_assert(check<eventually, disjunction,  formula>);
static_assert(check<eventually, implication,  formula>);
static_assert(check<eventually, iff,          formula>);
static_assert(check<eventually, until,        formula>);
static_assert(check<eventually, release,      formula>);
static_assert(check<eventually, since,        formula>);
static_assert(check<eventually, triggered,    formula>);

static_assert(check<once, proposition,  formula>);
static_assert(check<once, boolean,      formula>);
static_assert(check<once, negation,     unary>);
static_assert(check<once, tomorrow,     unary>);
static_assert(check<once, w_tomorrow,   unary>);
static_assert(check<once, yesterday,    unary>);
static_assert(check<once, w_yesterday,  unary>);
static_assert(check<once, always,       unary>);
static_assert(check<once, eventually,   unary>);
static_assert(check<once, once,         once>);
static_assert(check<once, historically, unary>);
static_assert(check<once, conjunction,  formula>);
static_assert(check<once, disjunction,  formula>);
static_assert(check<once, implication,  formula>);
static_assert(check<once, iff,          formula>);
static_assert(check<once, until,        formula>);
static_assert(check<once, release,      formula>);
static_assert(check<once, since,        formula>);
static_assert(check<once, triggered,    formula>);

static_assert(check<historically, proposition,  formula>);
static_assert(check<historically, boolean,      formula>);
static_assert(check<historically, negation,     unary>);
static_assert(check<historically, tomorrow,     unary>);
static_assert(check<historically, w_tomorrow,   unary>);
static_assert(check<historically, yesterday,    unary>);
static_assert(check<historically, w_yesterday,  unary>);
static_assert(check<historically, always,       unary>);
static_assert(check<historically, eventually,   unary>);
static_assert(check<historically, once,         unary>);
static_assert(check<historically, historically, historically>);
static_assert(check<historically, conjunction,  formula>);
static_assert(check<historically, disjunction,  formula>);
static_assert(check<historically, implication,  formula>);
static_assert(check<historically, iff,          formula>);
static_assert(check<historically, until,        formula>);
static_assert(check<historically, release,      formula>);
static_assert(check<historically, since,        formula>);
static_assert(check<historically, triggered,    formula>);

static_assert(check<conjunction, proposition,  formula>);
static_assert(check<conjunction, boolean,      formula>);
static_assert(check<conjunction, negation,     formula>);
static_assert(check<conjunction, tomorrow,     formula>);
static_assert(check<conjunction, w_tomorrow,   formula>);
static_assert(check<conjunction, yesterday,    formula>);
static_assert(check<conjunction, w_yesterday,  formula>);
static_assert(check<conjunction, always,       formula>);
static_assert(check<conjunction, eventually,   formula>);
static_assert(check<conjunction, once,         formula>);
static_assert(check<conjunction, historically, formula>);
static_assert(check<conjunction, conjunction,  conjunction>);
static_assert(check<conjunction, disjunction,  binary>);
static_assert(check<conjunction, implication,  binary>);
static_assert(check<conjunction, iff,          binary>);
static_assert(check<conjunction, until,        binary>);
static_assert(check<conjunction, release,      binary>);
static_assert(check<conjunction, since,        binary>);
static_assert(check<conjunction, triggered,    binary>);

static_assert(check<disjunction, proposition,  formula>);
static_assert(check<disjunction, boolean,      formula>);
static_assert(check<disjunction, negation,     formula>);
static_assert(check<disjunction, tomorrow,     formula>);
static_assert(check<disjunction, w_tomorrow,   formula>);
static_assert(check<disjunction, yesterday,    formula>);
static_assert(check<disjunction, w_yesterday,  formula>);
static_assert(check<disjunction, always,       formula>);
static_assert(check<disjunction, eventually,   formula>);
static_assert(check<disjunction, once,         formula>);
static_assert(check<disjunction, historically, formula>);
static_assert(check<disjunction, conjunction,  binary>);
static_assert(check<disjunction, disjunction,  disjunction>);
static_assert(check<disjunction, implication,  binary>);
static_assert(check<disjunction, iff,          binary>);
static_assert(check<disjunction, until,        binary>);
static_assert(check<disjunction, release,      binary>);
static_assert(check<disjunction, since,        binary>);
static_assert(check<disjunction, triggered,    binary>);

static_assert(check<implication, proposition,  formula>);
static_assert(check<implication, boolean,      formula>);
static_assert(check<implication, negation,     formula>);
static_assert(check<implication, tomorrow,     formula>);
static_assert(check<implication, w_tomorrow,   formula>);
static_assert(check<implication, yesterday,    formula>);
static_assert(check<implication, w_yesterday,  formula>);
static_assert(check<implication, always,       formula>);
static_assert(check<implication, eventually,   formula>);
static_assert(check<implication, once,         formula>);
static_assert(check<implication, historically, formula>);
static_assert(check<implication, conjunction,  binary>);
static_assert(check<implication, disjunction,  binary>);
static_assert(check<implication, implication,  implication>);
static_assert(check<implication, iff,          binary>);
static_assert(check<implication, until,        binary>);
static_assert(check<implication, release,      binary>);
static_assert(check<implication, since,        binary>);
static_assert(check<implication, triggered,    binary>);

static_assert(check<iff, proposition,  formula>);
static_assert(check<iff, boolean,      formula>);
static_assert(check<iff, negation,     formula>);
static_assert(check<iff, tomorrow,     formula>);
static_assert(check<iff, w_tomorrow,   formula>);
static_assert(check<iff, yesterday,    formula>);
static_assert(check<iff, w_yesterday,  formula>);
static_assert(check<iff, always,       formula>);
static_assert(check<iff, eventually,   formula>);
static_assert(check<iff, once,         formula>);
static_assert(check<iff, historically, formula>);
static_assert(check<iff, conjunction,  binary>);
static_assert(check<iff, disjunction,  binary>);
static_assert(check<iff, implication,  binary>);
static_assert(check<iff, iff,          iff>);
static_assert(check<iff, until,        binary>);
static_assert(check<iff, release,      binary>);
static_assert(check<iff, since,        binary>);
static_assert(check<iff, triggered,    binary>);

static_assert(check<until, proposition,  formula>);
static_assert(check<until, boolean,      formula>);
static_assert(check<until, negation,     formula>);
static_assert(check<until, tomorrow,     formula>);
static_assert(check<until, w_tomorrow,   formula>);
static_assert(check<until, yesterday,    formula>);
static_assert(check<until, w_yesterday,  formula>);
static_assert(check<until, always,       formula>);
static_assert(check<until, eventually,   formula>);
static_assert(check<until, once,         formula>);
static_assert(check<until, historically, formula>);
static_assert(check<until, conjunction,  binary>);
static_assert(check<until, disjunction,  binary>);
static_assert(check<until, implication,  binary>);
static_assert(check<until, iff,          binary>);
static_assert(check<until, until,        until>);
static_assert(check<until, release,      binary>);
static_assert(check<until, since,        binary>);
static_assert(check<until, triggered,    binary>);

static_assert(check<release, proposition,  formula>);
static_assert(check<release, boolean,      formula>);
static_assert(check<release, negation,     formula>);
static_assert(check<release, tomorrow,     formula>);
static_assert(check<release, w_tomorrow,   formula>);
static_assert(check<release, yesterday,    formula>);
static_assert(check<release, w_yesterday,  formula>);
static_assert(check<release, always,       formula>);
static_assert(check<release, eventually,   formula>);
static_assert(check<release, once,         formula>);
static_assert(check<release, historically, formula>);
static_assert(check<release, conjunction,  binary>);
static_assert(check<release, disjunction,  binary>);
static_assert(check<release, implication,  binary>);
static_assert(check<release, iff,          binary>);
static_assert(check<release, until,        binary>);
static_assert(check<release, release,      release>);
static_assert(check<release, since,        binary>);
static_assert(check<release, triggered,    binary>);

static_assert(check<since, proposition,  formula>);
static_assert(check<since, boolean,      formula>);
static_assert(check<since, negation,     formula>);
static_assert(check<since, tomorrow,     formula>);
static_assert(check<since, w_tomorrow,   formula>);
static_assert(check<since, yesterday,    formula>);
static_assert(check<since, w_yesterday,  formula>);
static_assert(check<since, always,       formula>);
static_assert(check<since, eventually,   formula>);
static_assert(check<since, once,         formula>);
static_assert(check<since, historically, formula>);
static_assert(check<since, conjunction,  binary>);
static_assert(check<since, disjunction,  binary>);
static_assert(check<since, implication,  binary>);
static_assert(check<since, iff,          binary>);
static_assert(check<since, until,        binary>);
static_assert(check<since, release,      binary>);
static_assert(check<since, since,        since>);
static_assert(check<since, triggered,    binary>);

static_assert(check<triggered, proposition,  formula>);
static_assert(check<triggered, boolean,      formula>);
static_assert(check<triggered, negation,     formula>);
static_assert(check<triggered, tomorrow,     formula>);
static_assert(check<triggered, w_tomorrow,   formula>);
static_assert(check<triggered, yesterday,    formula>);
static_assert(check<triggered, w_yesterday,  formula>);
static_assert(check<triggered, always,       formula>);
static_assert(check<triggered, eventually,   formula>);
static_assert(check<triggered, once,         formula>);
static_assert(check<triggered, historically, formula>);
static_assert(check<triggered, conjunction,  binary>);
static_assert(check<triggered, disjunction,  binary>);
static_assert(check<triggered, implication,  binary>);
static_assert(check<triggered, iff,          binary>);
static_assert(check<triggered, until,        binary>);
static_assert(check<triggered, release,      binary>);
static_assert(check<triggered, since,        binary>);
static_assert(check<triggered, triggered,    triggered>);


TEST_CASE("Non-formula matchers") 
{
  alphabet sigma;

  proposition p1 = sigma.prop("p1");
  proposition p2 = sigma.prop("p2");
  proposition p3 = sigma.prop("p3");
  proposition p4 = sigma.prop("p4");
  proposition p5 = sigma.prop("p5");

  SECTION("big_conjunction matcher") {
    formula f = (p1 && p2) && (p3 && (p4 && p5));

    std::string result;
    f.match(
      [&](big_conjunction a) {
        for(auto op : a.operands()) {
          result += to_string(op);
        }
      },
      [](otherwise) { }
    );

    REQUIRE(result == "p1p2p3p4p5");
  }

  SECTION("big_disjunction matcher") {
    formula f = (p1 || p2) || (p3 || (p4 || p5));

    std::string result;
    f.match(
      [&](big_disjunction a) {
        for(auto op : a.operands()) {
          result += to_string(op);
        }
      },
      [](otherwise) { }
    );

    REQUIRE(result == "p1p2p3p4p5");
  }
  
  SECTION("past matcher") {
    formula f = S(p1,p2);
    int i = f.match(
      [](past p) {
        return p.match(
          [](yesterday) { return 1; },
          [](w_yesterday) { return 2; },
          [](once) { return 3; },
          [](historically) { return 4; },
          [](since) { return 5; },
          [](triggered) { return 6; }
        );
      },
      [](otherwise) { return 42; }
    );
    REQUIRE(i == 5);
  }

  SECTION("future matcher"){  
    formula f = U(p1,p2);
    int i = f.match(
      [](future p) {
        return p.match(
          [](tomorrow) { return 1; },
          [](w_tomorrow) { return 2; },
          [](eventually) { return 3; },
          [](always) { return 4; },
          [](until) { return 5; },
          [](release) { return 6; },
          [](w_until) { return 7; },
          [](s_release) { return 8; }
        );
      },
      [](otherwise) { return 42; }
    );

    REQUIRE(i == 5);
  }
  
  SECTION("propositional matcher") {
    formula f = implies(p1,p2);
    int i = f.match(
      [](propositional p) {
        return p.match(
          [](boolean) { return 1; },
          [](proposition) { return 2; },
          [](negation) { return 3; },
          [](conjunction) { return 4; },
          [](disjunction) { return 5; },
          [](implication) { return 6; },
          [](iff) { return 7; }
        );
      },
      [](otherwise) -> int { black_unreachable(); }
    );
    REQUIRE(i == 6);
  }

  SECTION("temporal matcher") {
    formula f = G(p1);
    int i = f.match(
      [](temporal p) {
        return p.match(
          [](tomorrow)     { return 1; },
          [](w_tomorrow)   { return 2; },
          [](always)       { return 3; },
          [](eventually)   { return 4; },
          [](until)        { return 5; },
          [](release)      { return 6; },
          [](w_until)      { return 7; },
          [](s_release)    { return 8; },
          [](yesterday)    { return 9; },
          [](w_yesterday)  { return 10; },
          [](once)         { return 11; },
          [](historically) { return 12; },
          [](since)        { return 13; },
          [](triggered)    { return 14; }
        );
      },
      [](otherwise) -> int { black_unreachable(); }
    );
    REQUIRE(i == 3);
  }  

  SECTION("quantifier_block matcher") {
    variable x = sigma.var("x");
    variable y = sigma.var("y");
    variable z = sigma.var("z");

    formula m = forall(x, x == y && y == z);

    formula f = exists({y,z}, m);

    bool executed = false;
    f.match(
      [&](quantifier_block b) {
        executed = true;
        REQUIRE(b.matrix() == m);
      },
      [](otherwise) {}
    );

    REQUIRE(executed);
  }

}
