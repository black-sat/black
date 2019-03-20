#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <optional>
#include <fmt/format.h>

#include <black/support/common.hpp>

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>

using namespace black;

TEST_CASE("Handles")
{
  alphabet sigma;

  boolean top = sigma.top();
  boolean bottom = sigma.bottom();
  atom p = sigma.var("p");
  atom q = sigma.var("q");

  SECTION("Basic booleans and atoms allocation") {
    REQUIRE(top.value());
    REQUIRE(!bottom.value());
    REQUIRE(p.name() == "p");
    REQUIRE(q.name() == "q");
    REQUIRE(p == p);
    REQUIRE(p != q);

    atom another = sigma.var("p");

    REQUIRE(another == p);
  }

  formula ftop = top;
  formula fbottom = bottom;
  formula fp = p;
  formula fq = q;

  REQUIRE(ftop == ftop);
  REQUIRE(ftop != fbottom);

  SECTION("Formula casting and type checking") {
    REQUIRE(ftop.is<boolean>());
    REQUIRE(fbottom.is<boolean>());
    REQUIRE(fp.is<atom>());
    REQUIRE(fq.is<atom>());

    std::optional<boolean> otop = ftop.to<boolean>();
    std::optional<atom> op = fp.to<atom>();
    std::optional<boolean> opwrong = fp.to<boolean>();

    REQUIRE(otop.has_value());
    REQUIRE(op.has_value());
    REQUIRE(!opwrong.has_value());

    boolean top2 = *otop;
    atom p2 = *op;

    REQUIRE(top2.value());
    REQUIRE(top2 == top);
    REQUIRE(p2 == p);

    REQUIRE(p2.name() == "p");
  }

  SECTION("Unary and binary formulas construction") {
    formula fnotp  = unary(unary::operator_type::negation, p);
    formula fnotp2 = unary(unary::operator_type::negation, fp);
    formula fnotq  = unary(unary::operator_type::negation, fq);

    REQUIRE(fnotp != fnotq);
    REQUIRE(fnotp == fnotp2); // not implemented yet

    REQUIRE(fnotp.is<unary>());
    REQUIRE(fnotp2.is<unary>());
    REQUIRE(fnotp.is<negation>());
    REQUIRE(!fnotp.is<tomorrow>());

    std::optional<unary> onotp = fnotp.to<unary>();

    REQUIRE(onotp.has_value());
    unary notp2 = *onotp;

    REQUIRE(notp2.operand() == p);
    REQUIRE(notp2.operand() == fp);
  }

  SECTION("Type-specific handles") {
    auto neg = negation(p);
    auto next = tomorrow(p);
    formula fneg = neg;
    formula fnext = next;
    formula fland = conjunction(neg, fnext);

    REQUIRE(fneg.is<negation>());
    REQUIRE(fnext.is<tomorrow>());
    REQUIRE(!fnext.is<negation>());
    REQUIRE(fland.is<conjunction>());
  }

  SECTION("Formula pattern matching")
  {
    auto match = [&](formula f) {
      return f.match(
        [](negation)    { return "negation";    },
        [](unary)       { return "unary";       },
        [](conjunction) { return "conjunction"; },
        [](binary)      { return "binary";      },
        [](otherwise)   { return "<unknown>";   }
      );
    };

    formula t = XF(p && GF(!q));

    REQUIRE(t.is<tomorrow>());
    REQUIRE(!t.is<negation>());

    REQUIRE(match(!p)          == "negation");
    REQUIRE(match(tomorrow(p)) == "unary");
    REQUIRE(match(p && q)      == "conjunction");
    REQUIRE(match(p || q)      == "binary");
  }
}
