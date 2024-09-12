#include "pipeline.hpp"
#include "automaton.cpp"
#include "debug.cpp"

using namespace black;
using namespace black::logic;

int main () {
  
  module Sigma;
  variable x = "x";

  object c = Sigma.define("c", types::integer(), 3);
  object q = Sigma.declare("q", types::boolean());

  object p = Sigma.declare("p", types::boolean());

  term phi = exists({{x, types::integer()}}, Y(x + x) > c);
  
  Sigma.require(phi);

  pipes::transform encoding = pipes::automaton() | pipes::debug();
  module result = encoding(Sigma);
}