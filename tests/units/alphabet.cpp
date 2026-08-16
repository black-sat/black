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

#include <catch.hpp>

#include <algorithm>
#include <string>
#include <thread>
#include <vector>

using namespace black;

//
// Builds the very same batch of formulas at every call, touching a fair number
// of different storage kinds on the way: leaf ones (`proposition`, `variable`,
// `relation`), unary and binary ones, and one (`atom`) whose nodes hold a
// vector of children.
//
static std::vector<formula> build_batch(alphabet &sigma, size_t n) {
  std::vector<formula> batch;
  batch.reserve(n);

  auto r = sigma.relation("r");

  for(size_t i = 0; i < n; ++i) {
    auto p = sigma.proposition("p" + std::to_string(i));
    auto q = sigma.proposition("q" + std::to_string(i % 8));
    auto x = sigma.variable("x" + std::to_string(i % 16));

    batch.push_back(implies(U(p, q), G(F(p && !q))) && X(r(x, x)));
  }

  return batch;
}

TEST_CASE("Alphabet modes") {

  SECTION("Sequential mode is the default") {
    alphabet sigma;

    REQUIRE(sigma.mode() == alphabet_mode::sequential);
  }

  SECTION("Concurrent alphabets unique nodes as sequential ones do") {
    alphabet sigma{alphabet_mode::concurrent};

    REQUIRE(sigma.mode() == alphabet_mode::concurrent);

    REQUIRE(sigma.boolean(true) == sigma.boolean(true));
    REQUIRE(sigma.boolean(true) != sigma.boolean(false));
    REQUIRE(sigma.proposition("p") == sigma.proposition("p"));
    REQUIRE(sigma.proposition("p") != sigma.proposition("q"));

    formula p = sigma.proposition("p");
    formula q = sigma.proposition("q");

    REQUIRE(U(p, q) == U(p, q));
    REQUIRE(U(p, q) != U(q, p));
    REQUIRE(G(F(p && !q)) == G(F(p && !q)));
  }

  SECTION("Both modes agree on the formulas they build") {
    alphabet sequential;
    alphabet concurrent{alphabet_mode::concurrent};

    std::vector<formula> one = build_batch(sequential, 64);
    std::vector<formula> other = build_batch(concurrent, 64);

    REQUIRE(one.size() == other.size());

    // nodes come from two different alphabets, so they cannot be compared by
    // identity, but building the same batch twice must at least have made the
    // same number of distinct nodes out of it.
    auto distinct = [](std::vector<formula> const& batch) {
      std::vector<void const *> nodes;
      for(formula f : batch)
        nodes.push_back(f.node());

      std::sort(nodes.begin(), nodes.end());
      return size_t(std::unique(nodes.begin(), nodes.end()) - nodes.begin());
    };

    REQUIRE(distinct(one) == distinct(other));
    REQUIRE(distinct(one) == one.size());
  }
}

TEST_CASE("Concurrent alphabets can be shared among threads") {

  size_t const nthreads =
    std::max(size_t{4}, size_t{std::thread::hardware_concurrency()});
  size_t const batch_size = 500;

  alphabet sigma{alphabet_mode::concurrent};

  //
  // All the threads build the very same formulas at the same time, so they
  // race on every single node: whoever gets there first inserts it and the
  // others have to find it.
  //
  std::vector<std::vector<formula>> batches(nthreads);
  std::vector<std::thread> threads;

  for(size_t t = 0; t < nthreads; ++t)
    threads.push_back(std::thread([&, t]() {
      batches[t] = build_batch(sigma, batch_size);
    }));

  for(std::thread &thread : threads)
    thread.join();

  // note that Catch2's assertions are not thread-safe, hence all the checking
  // happens here, after all the threads are done.

  std::vector<formula> const& expected = batches[0];
  REQUIRE(expected.size() == batch_size);

  // hash-consing must have handed the same node to every thread, whichever of
  // them won each insertion race. This is what makes `operator==` on formulas
  // meaningful in the first place.
  for(size_t t = 1; t < nthreads; ++t) {
    REQUIRE(batches[t].size() == batch_size);
    for(size_t i = 0; i < batch_size; ++i)
      REQUIRE(batches[t][i] == expected[i]);
  }

  // conversely, formulas that differ must have got different nodes, i.e. no
  // two of them were collapsed into one.
  std::vector<void const *> nodes;
  for(formula f : expected)
    nodes.push_back(f.node());

  std::sort(nodes.begin(), nodes.end());
  REQUIRE(std::unique(nodes.begin(), nodes.end()) == nodes.end());

  // and the alphabet is still consistent with itself afterwards.
  std::vector<formula> again = build_batch(sigma, batch_size);
  for(size_t i = 0; i < batch_size; ++i)
    REQUIRE(again[i] == expected[i]);
}
