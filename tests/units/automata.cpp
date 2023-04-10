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

#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/logic/parser.hpp>
#include <black/automata/automaton.hpp>
#include <black/internal/debug/random_formula.hpp>

#include <catch.hpp>

#include <random>
#include <iostream>

//using namespace black;
namespace ltl = black::logic::fragments::LTL;
namespace sdd = black::sdd;

using vars_t = std::unordered_set<ltl::proposition>;

static void nvars(ltl::formula f, vars_t &set) {
  using namespace ltl;

  f.match(
    [](boolean) { },
    [&](proposition p) { set.insert(p); },
    [&](unary, auto arg) {
      nvars(arg, set);
    },
    [&](binary, auto left, auto right) {
      nvars(left, set);
      nvars(right, set);
    }
  );
}

static size_t nvars(ltl::formula f) {
  vars_t set;
  nvars(f, set);
  return set.size();
}

TEST_CASE("automata") {
  black::alphabet sigma;

  std::mt19937 gen(std::random_device{}());

  // auto f = black::parse_formula(
  //   sigma, 
  //   "(G(p1)) & (F(p2)) & (F(p3)) & (F(p4)) & (F(p5)) & (F(p6)) & (F(p7)) & (F(p8)) & (F(p9)) & (F(p10)) & (F(p11)) & (F(p12)) & (F(p13)) & (F(p14)) & (F(p15)) & (F(p16)) & (F(p17)) & (F(p18)) & (F(p19)) & (F(p20))", 
  //   [](auto) { }
  // ).value().to<black::logic::formula<black::logic::LTL>>().value();
  
  auto f = black::random_ltl_formula(gen, sigma, 15, {"p", "q", "r", "s", "t"});

  std::cerr << "formula: " << to_string(f) << "\n";
  
  sdd::manager mgr{&sigma, nvars(f)};
  
  std::cerr << "Starting encoding... " << std::flush;
  auto aut = black_internal::to_automaton(&mgr, f);
  std::cerr << "done!\n";

  std::cerr << "Starting determinization... " << std::flush;
  aut = determinize(aut);
  std::cerr << "done!\n";
}