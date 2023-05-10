//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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

#include <black/logic.hpp>

using namespace black::logic::internal;
using namespace black::support;

struct Bool : make_fragment_t<
  syntax_list<
    syntax_element::proposition,
    syntax_element::conjunction,
    syntax_element::negation
  >
> { };

TEST_CASE("Terms hierarchy") {
  
  alphabet sigma;

  proposition p = sigma.proposition("p");
  proposition q = sigma.proposition("q");

  formula<Bool> f = p && !q;

  STATIC_REQUIRE(matchable<formula<Bool>>);
  STATIC_REQUIRE(matchable<formula<Bool>::type>);

  std::string s = f.match(
    [](conjunction<Bool>) {
      return "Ok";
    },
    [](otherwise) {
      return "Not Ok";
    }
  );

  REQUIRE(s == "Ok");

}

TEST_CASE("New syntax checking") {

  using namespace black::logic::internal;

  using large = syntax_list<
    syntax_element::proposition, 
    syntax_element::negation, 
    syntax_element::conjunction, 
    syntax_element::disjunction,
    syntax_element::implication,
    syntax_element::iff
  >;

  using small = syntax_list<
    syntax_element::proposition, 
    syntax_element::conjunction
  >;

  using small2 = syntax_list<
    syntax_element::disjunction,
    syntax_element::implication
  >;

  static_assert(
    rule_implies_v<
      tree<small, node<small>>, 
        unite<
          unite<node<small>, node<small2>>
        >
    >
  );

  // using list3 = syntax_list<
  //   syntax_element::proposition
  // >;

  // static_assert(
  //   std::is_same_v<
  //     syntax_list_subtract_t<test1, test2>,
  //     syntax_list<syntax_element::negation>
  //   >
  // );

  // static_assert(is_new_subfragment_of_v<Literal, Boolean>);
  // static_assert(!is_new_subfragment_of_v<Boolean, Literal>);

  // static_assert(is_new_subfragment_of_v<NNF, Boolean>);
  // static_assert(!is_new_subfragment_of_v<Boolean, NNF>);

    
}