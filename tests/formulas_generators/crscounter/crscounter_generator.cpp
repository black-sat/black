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

#include <black/logic//alphabet.hpp>
#include <black/logic//formula.hpp>
#include <black/logic//parser.hpp>
#include <black/logic//prettyprint.hpp>

#include <iostream>
#include <cmath>

using namespace black;

static alphabet sigma;

formula once_chain(int, int);
formula counter(int);
formula next_counter(int);
formula equals(int);
formula equals_d(int, int);
formula digit(int, int);
inline int ilog2(int);

/* This aim to produce the set of parametrized properties of the form:
 *   ! F( O((c = N/2) /\ O((c = N/2+1) /\ ... O(c = N/2+i) ...)) )
 * proposed in
 *   Cimatti, Alessandro, Marco Roveri, e Daniel Sheridan.
 *   «Bounded Verification of Past LTL». In Formal Methods in Computer-Aided
 *   Design, a cura di Alan J. Hu e Andrew K. Martin, 245–59. Lecture Notes
 *   in Computer Science. Berlin, Heidelberg: Springer, 2004.
 *   https://doi.org/10.1007/978-3-540-30494-4_18.
 */
int main(int argc, char **argv) {
  int N,i;

  if (argc < 3 || argc > 4) {
    std::cerr
      << "Generator for parametrized properties of the form:\n"
      << "\t! F( O((c = N/2) /\\ O((c = N/2+1) /\\ ... O(c = N/2+i) ...)) )"
      << "\n\nUsage: past_generator <N> <i> [mode]\n"
      << "\tN,i  : int numbers, N must be even\n"
      << "\tmode : set it to 'next' to use the counter based on X operator\n";
    exit(1);
  }

  try {
    N = std::stoi(argv[1]);
    i = std::stoi(argv[2]);
  } catch (const std::invalid_argument& e) {
    std::cerr << "Invalid argument: " << e.what() << "\n";
    exit(1);
  }

  if (N % 2) {
    std::cerr << "First parameter (N) must be even.";
    exit(1);
  }

  // Retrieve optional modality
  std::string mode;
  if (argc == 4) {
    mode = argv[3];
    if (mode != "next") {
      std::cerr << "Unkown modality.";
      exit(1);
    }
  }

  // Build counter
  formula f_counter = sigma.top();
  if (mode == "next") {
    f_counter = next_counter(N);
  } else {
    f_counter = counter(N);
  }

  formula property = ! F( once_chain(N, i) );
  formula f = ! ( implication( f_counter, property ) );

  std::cout << to_string(f);

  return 0;
}

/* Produces the chain of nested once operators:
 *   O((c = N/2) /\ O((c = N/2+1) /\ ... O(c = N/2+i) ...))
 */
formula once_chain(int N, int i) {
  int d = ilog2(N);
  formula f = O( equals_d(N/2+i, d) );

  for (int j=i-1; j>=0; j--) f = O( equals_d(N/2+j, d) && f );

  return f;
}

/* Produces the counter formula from 0 to N and then loop back to N/2.
 */
formula counter(int N) {
  int d = ilog2(N);
  formula init = equals_d(0, d);
  formula trans = sigma.top();

  for (int i=1; i<=N; i++) {
    formula t = equals_d(i-1, d);
    if (i == N/2) t = t || equals_d(N, d); // create the loop at N/2
    formula step = iff(equals_d(i, d), Y(t));
    trans = (i==1) ? step : trans && step; // accumulate from i>=2
  }

  return init && G(trans);
}

/* Produces the counter formula from 0 to N and then loop back to N/2.
 * It uses Next instead of Yesterday operators.
 */
formula next_counter(int N) {
  int d = ilog2(N);
  formula init = equals_d(0, d);
  formula trans = iff(equals_d(0, d), X(equals_d(1, d)));

  for (int i=1; i<N; i++) {
    formula t = equals_d(i, d);
    if (i == N/2-1) t = t || equals_d(N, d); // create the loop at N/2
    trans = trans && iff(t, X(equals_d(i+1, d)));
  }

  return init && G(trans);
}

/* Produces the single equality 'c = N/2+i'. Where 'num' is the specific N/2+i.
 * This is done producing a conjunction with each digit of the binary
 * representation of 'num'.
 * That is, if 'num' is '100', then to express the equality we produce instead:
 *   c2 /\ !c1 /\ !c0
 */
formula equals(int num) {
  int l = ilog2(num);
  formula f = digit(0, num);

  for (int i = 1; i <= l; i++) {
    num = std::div(num, 2).quot;
    f = digit(i, num) && f;
  }

  return f;
}

/* Produces the same as 'equals(int)' but with a prefixed amount of digits.
 * That is, if num = 4, which is '100' in binary, but d=4, then it will produce:
 *   !c3 /\ c2 /\ !c1 /\ !c0
 */
formula equals_d(int num, int d) {
  formula f = equals(num);

  for (int i = ilog2(num)+1; i <= d; i++) {
    f = digit(i, 0) && f;
  }

  return f;
}

// Produces the i-th binary digit
formula digit(int i, int num) {
  formula c = sigma.prop("(c" + std::to_string(i) + ")");
  return (num % 2) ? c : negation(c);
}

// Calculate the integer log2
inline int ilog2(int num) {
  return (num == 0) ? 0 : (int)floor(log2(num));
}
