//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Gabriele Venturato
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

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>

#include <iostream>
#include <fstream>
#include <cmath>

using namespace black;

formula digit(alphabet &sigma, int i, int num) {
  return iff(sigma.var("c" + std::to_string(i)),
             (num % 2) ? sigma.top() : sigma.bottom());
}

formula equals(alphabet &sigma, int k) {
  int l = (int)floor(log2(k)) + 1;
  formula f = digit(sigma, 0, k);

  for (int i=1; i<l; i++) {
    k = std::div(k,2).quot;
    f = f && digit(sigma, i, k);
  }

  return f;
}

formula once_chain(alphabet &sigma, int n, int i) {
  formula f = P( equals(sigma, n/2+i) );

  for (int j=i-1; j>=0; j--) {
    f = P( equals(sigma, n/2+j) && f );
  }

  return f;
}

int main() { // int argc, char **argv
  int i=20,n=32768;
  alphabet sigma;

  black_assert(n%2 == 0);

  formula f = ! F( once_chain(sigma, n, i) );

  std::cout << to_string(f);

  return 0;
}

