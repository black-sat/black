//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Luca Geatti
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
#include <black/solver/solver.hpp>

#include <iostream>
#include <functional>
#include <string_view>

using namespace black;

//
// Representation of KDSs, their encoding into LTLfMT, and their verification
//
struct KDS {
  scope xi;
  formula init;
  formula axioms;
  std::vector<formula> trans;
  formula final;
};

static tribool verify_kds(KDS kds, formula property) {
  auto [xi, init, axioms, trans, final] = std::move(kds);
  alphabet &sigma = *xi.sigma();
  
  formula trans_phi = big_or(sigma, trans);
  formula system = 
    init && G(axioms) && G(implies(X(sigma.top()), trans_phi)) && F(final);

  formula to_check = system && !property;

  [[maybe_unused]]
  bool ok = xi.type_check(to_check, [&](auto s) {
    std::cerr << "Type checking error: " << s << "\n";
    std::cerr << "Formula: " << to_string(to_check) << "\n";
  });
  black_assert(ok);

  solver slv;

  slv.set_tracer([](black::solver::trace_t trace) {
    if(trace.type == black::solver::trace_t::stage)
      std::cout << ".";
  });

  return slv.solve(
    xi, to_check, 
    /* finite = */ true, 
    std::numeric_limits<size_t>::max(),
    /* semi_decidable = */ true
  );
}

//
// Modeling of the two example KDSs
//
static auto frame_axiom(scope &xi, relation rel) {
  using namespace std::literals;

  alphabet &sigma = *xi.sigma();
  std::vector<sort> sorts = *xi.signature(rel);
  std::vector<var_decl> decls;
  for(size_t i = 0; i < sorts.size(); i++)
    decls.push_back(
      sigma.var_decl(sigma.variable(std::pair{"x"sv, i}), sorts[i])
    );

  return 
    forall(decls, implies(X(sigma.top()), iff(rel(decls), wX(rel(decls)))));
}

static formula true_unless(formula f, bool b) {
  if(!b)
    return f.sigma()->top();
  return f;
}

static KDS model_kds(alphabet &sigma, bool simple, size_t N) {
  using namespace std::literals;
  scope xi{sigma};

  auto u1 = sigma.variable("u1");
  auto u2 = sigma.variable("u2");
  auto g1 = sigma.variable("g1");
  auto g2 = sigma.variable("g2");

  auto user = sigma.named_sort("user");
  auto state = sigma.named_sort("state");
  auto group = sigma.named_sort("group");

  auto applications = sigma.relation("applications");
  auto winners = sigma.relation("winners");

  xi.declare(applications, {user, group});
  xi.declare(winners, {user, group});

  auto x_state = sigma.variable("x_state");
  auto x_winners = sigma.variable("x_winners");
  auto x_under = sigma.variable("x_under");

  xi.declare(x_state, state);

  if(!simple) {
    xi.declare(x_winners, sigma.integer_sort());
    xi.declare(x_under, sigma.integer_sort());
  }

  auto c_init = sigma.variable("c_init");
  auto c_app = sigma.variable("c_app");
  auto c_eval = sigma.variable("c_eval");
  auto c_final = sigma.variable("c_final");

  std::vector<variable> eval_n;
  for(size_t i = 0; i < N; i++)
    eval_n.push_back(sigma.variable(std::tuple{"c_eval"sv, i}));

  std::vector<variable> states = {c_init, c_app, c_eval, c_final};

  if(simple)
    states.insert(states.end(), eval_n.begin(), eval_n.end());

  xi.declare(state, make_domain(states));

  auto c_majority = sigma.variable("c_majority");
  auto c_under = sigma.variable("c_under");
  xi.declare(group, make_domain({c_majority, c_under}));

  formula winners_primary_key = 
    forall({u1[user], g1[group], g2[group]}, 
      implies(winners(u1, g1) && winners(u1, g2), g1 == g2)
    );
  
  formula applications_primary_key = 
    forall({u1[user], g1[group], g2[group]}, 
      implies(applications(u1, g1) && applications(u1, g2), g1 == g2)
    );

  formula axioms = winners_primary_key && applications_primary_key;

  formula init = x_state == c_init && 
    true_unless(x_winners == 0 && x_under == 0, !simple) &&
    forall({u1[user], g1[group]}, !applications(u1, g1)) && 
    forall({u1[user], g1[group]}, !winners(u1, g1));

  std::vector<formula> trans;

  // from `init` to `app` or the self-loop over `app`
  trans.push_back(
    exists({u1[user], g1[group]}, 
      // guard
      (x_state == c_init || x_state == c_app) && 
      !applications(u1, g1) &&
      // update
      wnext(x_state) == c_app &&
      wX(applications(u1, g1)) &&
      // frames
      true_unless(wnext(x_winners) == x_winners, !simple) &&
      true_unless(wnext(x_under) == x_under, !simple) &&
      forall({u2[user], g2[group]},
        (
          implies(u2 != u1 && g2 != g1, 
            implies(X(sigma.top()), 
              iff(applications(u2, g2), wX(applications(u2, g2)))
            )
          )
        )
      ) && frame_axiom(xi, winners) 
    )
  );

  // from `app` to `eval`
  trans.push_back(
    // guard
    x_state == c_app &&
    // update
    true_unless(wnext(x_state) == c_eval, !simple) &&
    true_unless(wnext(x_state) == eval_n[0], simple) &&
    // frames
    true_unless(wnext(x_winners) == x_winners, !simple) &&
    true_unless(wnext(x_under) == x_under, !simple) &&
    frame_axiom(xi, applications) &&
    frame_axiom(xi, winners)
  );

  // from `eval_i` to `eval_{i+1}` and `eval_{N-1}` to `c_eval`
  if(simple) {
    for(size_t i = 0; i < N; ++i) {
      trans.push_back(
        exists({u1[user], g1[group]}, 
          // guard
          x_state == eval_n[i] &&
          applications(u1, g1) &&
          // update
          wnext(x_state) == (i == N - 1 ? c_eval : eval_n[i + 1]) &&
          wX(winners(u1, g1)) &&
          // frames
          forall({u2[user], g2[group]},
            implies(u2 != u1 && g2 != g1, 
              implies(X(sigma.top()), 
                iff(winners(u2, g2), wX(winners(u2, g2)))
              )
            )
          )
        ) && frame_axiom(xi, applications)
      );
    }

  }

  // self-loop over `eval`
  trans.push_back(
    exists({u1[user], g1[group]}, 
      // guard
      x_state == c_eval &&
      applications(u1, g1) &&
      // update
      wnext(x_state) == c_eval &&
      true_unless(wnext(x_winners) == x_winners + 1, !simple) &&
      true_unless(
        implies(g1 == c_under, wnext(x_under) == x_under + 1), !simple
      ) &&
      true_unless(
        implies(g1 == c_majority, wnext(x_under) == x_under), !simple
      ) &&
      wX(winners(u1, g1)) &&
      // frames
      forall({u2[user], g2[group]},
        implies(u2 != u1 && g2 != g1, 
          implies(X(sigma.top()), 
            iff(winners(u2, g2), wX(winners(u2, g2)))
          )
        )
      )
    ) && frame_axiom(xi, applications)
  );

  // from `eval` to `done`
  trans.push_back(
    // guard
    x_state == c_eval && 
    true_unless(x_winners > N, !simple) &&
    true_unless(3 * x_under > x_winners, !simple) &&
    true_unless(exists({u1[user]}, winners(u1, c_under)), simple) &&
    // update
    wnext(x_state) == c_final &&
    // frames
    true_unless(wnext(x_winners) == x_winners, !simple) &&
    true_unless(wnext(x_under) == x_under, !simple) &&
    frame_axiom(xi, winners) &&
    frame_axiom(xi, applications)
  );

  formula final = x_state == c_final;

  return KDS{std::move(xi), init, axioms, trans, final};
}

//
// Properties to test
//
static formula mk_property(alphabet &sigma, int S, int P) {
  auto u1 = sigma.variable("u1");
  auto u2 = sigma.variable("u2");
  auto x_state = sigma.variable("x_state");
  auto x_under = sigma.variable("x_under");
  auto x_winners = sigma.variable("x_winners");
  auto c_under = sigma.variable("c_under");
  auto c_app = sigma.variable("c_app");
  auto c_final = sigma.variable("c_final");

  auto user = sigma.named_sort("user");
  
  auto winners = sigma.relation("winners");

  if(S == 1) {
    formula gamma = 2 * x_under > x_winners;

    if(P == 1)
      return G(implies(x_state == c_final, gamma));

    if(P == 2)
      return G(implies(x_state == c_app, F(x_state == c_final && gamma)));
  }

  if(S == 2) {
    formula gamma = // exists({u1[user]}, winners(u1, c_under));
      exists({u1[user], u2[user]}, 
        u1 != u2 && winners(u1, c_under) && winners(u2, c_under)
      );

    if(P == 1)
      return G(implies(x_state == c_final, gamma));
    
    if(P == 2)
      return G(implies(x_state == c_app, F(x_state == c_final && gamma)));
  } 

  black_unreachable();
}

//
// Command line handling and set up of the experiment
//

[[noreturn]]
static void usage(const char *argv0) {
  std::cerr << "usage: " << argv0 << " <S> <P> <N>\n";
  std::cerr << " - <S>: either 1 (arithmetic) or 2 (simple)\n";
  std::cerr << 
    " - <P>: property to test, either `s` (safety) or `l` (liveness)\n";
  std::cerr << " - <N>: number of iterations\n";

  exit(1);
}

int main(int argc, char **argv) 
{
  using namespace std::literals;

  if(argc < 4)
    usage(argv[0]);

  int S = atoi(argv[1]);
  int A3 = atoi(argv[3]);

  if(S != 1 && S != 2) {
    std::cerr << argv[0] << ": error: S must be 1 or 2\n";
    usage(argv[0]);
  }

  int P = 0;
  if(argv[2] == "s"sv)
    P = 1;
  if(argv[2] == "l"sv)
    P = 2;

  if(P != 1 && P != 2) {
    std::cerr << argv[0] << ": error: P must be `s` or `l`\n";
    usage(argv[0]);
  }

  if(A3 <= 0) {
    std::cerr << argv[0] << ": error: N must be positive\n";
    usage(argv[0]);
  }

  size_t N = size_t(A3);

  std::cout << 
    "Verifying " << ((S == 2) ? "simple" : "arithmetic") <<
    " property " << P << " for N = " << N << "...";

  bool simple = S == 2;

  alphabet sigma;

  KDS system = model_kds(sigma, simple, N);
  formula property = mk_property(sigma, S, P);
  
  tribool result = verify_kds(std::move(system), property);

  if(result == true) {
    std::cout << " Bug found!\n";
    return 0;
  } else if(result == false)
    std::cout << " System verified!\n";
  else
    std::cout << " UNKNOWN\n";

  return 1;
}
