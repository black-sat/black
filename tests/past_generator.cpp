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
#include <cmath>

using namespace black;

formula once_chain(int, int);
formula equals(int);
formula digit(int, int);

alphabet sigma;

/* This aim to produce the set of parametrized properties of the form:
 *   ! F( P((c = N/2) /\ P((c = N/2+1) /\ ... P(c = N/2+i) ...)) )
 * proposed in
 *   Cimatti, Alessandro, Marco Roveri, e Daniel Sheridan.
 *   «Bounded Verification of Past LTL». In Formal Methods in Computer-Aided
 *   Design, a cura di Alan J. Hu e Andrew K. Martin, 245–59. Lecture Notes
 *   in Computer Science. Berlin, Heidelberg: Springer, 2004.
 *   https://doi.org/10.1007/978-3-540-30494-4_18.
 */
int main(int argc, char **argv) {
  int N,i;

  if (argc != 3) {
    std::cerr << "Usage: past_generator <N> <i>\n"
              << "\tN,i : int numbers; N must be even\n";
    return 1;
  }

  N = atoi(argv[1]);
  i = atoi(argv[2]);

  black_assert(N%2 == 0);

  formula f = ! F( once_chain(N, i) );

  std::cout << to_string(f);

  return 0;
}

/* Produces the chain of nested once operators:
 *   P((c = N/2) /\ P((c = N/2+1) /\ ... P(c = N/2+i) ...))
 */
formula once_chain(int N, int i) {
  formula f = P( equals(N/2+i) );

  for (int j=i-1; j>=0; j--) f = P( equals(N/2+j) && f );

  return f;
}

/* Produces the single equality 'c = N/2+i'. Where 'num' is the specific N/2+i.
 * This is done producing a conjunction with each digit of the binary
 * representation of 'num'.
 * That is, if 'num' is '100', then to express the equality we produce instead:
 *   (c2 <-> True) /\ (c1 <-> False) /\ (c0 <-> False)
 */
formula equals(int num) {
  int l = floor(log2(num));
  formula f = digit(0, num);

  for (int i = 1; i <= l; i++) {
    num = std::div(num, 2).quot;
    f = digit(i, num) && f;
  }

  return f;
}

// Produces the single 'if and only if' for the i-th digit
formula digit(int i, int num) {
  return iff(sigma.var("c" + std::to_string(i)),
             (num % 2) ? sigma.top() : sigma.bottom());
}
