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
#include <string_view>

using namespace black;

[[maybe_unused]]
static void tracer(black::solver::trace_t trace) {
  static size_t k = 0;
  using black::logic::fragments::FO::formula;

  if(trace.type == black::solver::trace_t::stage) {
    k = std::get<size_t>(trace.data);
    std::cerr << "k: " << k << "\n";
  }

  // if(trace.type == black::solver::trace_t::unrav) {
  //   std::cerr << k << "-unrav: " << 
  //     to_string(std::get<formula>(trace.data)) << "\n";
  // }

  // if(trace.type == black::solver::trace_t::empty) {
  //   std::cerr << k << "-empty: " << 
  //     to_string(std::get<formula>(trace.data)) << "\n";
  // }
  
  // if(trace.type == black::solver::trace_t::prune) {
  //   std::cerr << k << "-prune: " << 
  //     to_string(std::get<formula>(trace.data)) << "\n";
  // }
}

inline auto frame_axiom(scope &xi, relation rel) {
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

inline formula true_if(formula f, bool b) {
  if(b)
    return f.sigma()->top();
  return f;
}

[[noreturn]]
static void usage(const char *argv0) {
  std::cerr << "usage: " << argv0 << " <S> <P> <N>\n";
  std::cerr << " - <S>: either 1 (simple) or 2 (arithmetic)\n";
  std::cerr << " - <P>: property to test, either 1 (safety) or 2 (guarantee)\n";
  std::cerr << " - <N>: number of iterations\n";

  exit(1);
}

int main(int argc, char **argv) {

  if(argc < 3) {
    std::cerr << "Please specify number of phases\n";
    return 1;
  }

  int S = atoi(argv[1]);
  int P = atoi(argv[2]);
  int N = atoi(argv[3]);

  if(S != 1 && S != 2) {
    std::cerr << argv[0] << ": error: S must be 1 or 2\n";
    usage(argv[0]);
  }

  if(P != 1 && P != 2) {
    std::cerr << argv[0] << ": error: P must be 1 or 2\n";
    usage(argv[0]);
  }

  if(N <= 0) {
    std::cerr << argv[0] << ": error: N must be positive\n";
    usage(argv[0]);
  }

  std::cout << 
    "Solving " << ((S == 1) ? "simple" : "arithmetic") <<
    " property " << P << " for N = " << N << "...\n";

  alphabet sigma;
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
  auto x_u_winners = sigma.variable("x_u_winners");

  xi.declare(x_state, state);
  xi.declare(x_winners, sigma.integer_sort());
  xi.declare(x_u_winners, sigma.integer_sort());

  auto c_init = sigma.variable("c_init");
  auto c_app = sigma.variable("c_app");
  auto c_eval = sigma.variable("c_eval");
  auto c_final = sigma.variable("c_final");

  xi.declare(state, make_domain({c_init, c_app, c_eval, c_final}));

  auto c_majority = sigma.variable("c_majority");
  auto c_underrepresented = sigma.variable("c_underrepresented");
  xi.declare(group, make_domain({c_majority, c_underrepresented}));

  formula winners_primary_key = 
    forall({u1[user], g1[group], g2[group]}, 
      implies(winners(u1, g1) && winners(u1, g2), g1 == g2)
    );
  
  formula applications_primary_key = 
    forall({u1[user], g1[group], g2[group]}, 
      implies(applications(u1, g1) && applications(u1, g2), g1 == g2)
    );

  formula axioms = winners_primary_key && applications_primary_key;

  formula init = x_state == c_init && x_winners == 0 && x_u_winners == 0 &&
    forall({u1[user], g1[group]}, !applications(u1, g1)) && 
    forall({u1[user], g1[group]}, !winners(u1, g1));

  formula phi_tr = sigma.bottom();

  // from `init` to `app` or the self-loop over `app`
  phi_tr = phi_tr || (
    exists({u1[user], g1[group]}, 
      // guard
      (x_state == c_init || x_state == c_app) && 
      !applications(u1, g1) &&
      // update
      wnext(x_state) == c_app &&
      wX(applications(u1, g1)) &&
      // frames
      wnext(x_winners) == x_winners &&
      wnext(x_u_winners) == x_u_winners &&
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
  phi_tr = phi_tr || (
    // guard
    x_state == c_app &&
    // update
    wnext(x_state) == c_eval &&
    // frames
    wnext(x_winners) == x_winners &&
    wnext(x_u_winners) == x_u_winners &&
    frame_axiom(xi, applications) &&
    frame_axiom(xi, winners)
  );

  // self-loop over `eval`
  phi_tr = phi_tr || (
    exists({u1[user], g1[group]}, 
      // guard
      x_state == c_eval &&
      applications(u1, g1) &&
      // update
      wnext(x_state) == c_eval &&
      wnext(x_winners) == x_winners + 1 &&
      implies(
        g1 == c_underrepresented, wnext(x_u_winners) == x_u_winners + 1
      ) &&
      implies(g1 == c_majority, wnext(x_u_winners) == x_u_winners) &&
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
  phi_tr = phi_tr || (
    // guard
    x_state == c_eval && 
    x_winners > N &&
    //3 * x_u_winners > x_winners && // UNCOMMENT THIS to make it correct
    // update
    wnext(x_state) == c_final &&
    // frames
    wnext(x_winners) == x_winners &&
    wnext(x_u_winners) == x_u_winners &&
    frame_axiom(xi, winners) &&
    frame_axiom(xi, applications)
  );

  [[maybe_unused]]
  formula final = x_state == c_final;

  formula system = 
    init && G(axioms) && G(implies(X(sigma.top()), phi_tr)) && F(final);

  // -------

  formula property = sigma.top();

  if(S == 1) {
    if(P == 1)
      property = G(
        implies(
          x_state == c_final, 
          exists({u1[user], g1[group]}, 
            winners(u1, g1) && g1 == c_underrepresented)
        )
      );
    
    if(P == 2)
      property = G(
        implies(
          x_state == c_app, 
          F(
            x_state == c_final &&
            exists({u1[user], g1[group]}, 
              winners(u1, g1) && g1 == c_underrepresented)
          )
        )
      );
  } 

  if(S == 2) {
    if(P == 1)
      property =
        G(implies(x_state == c_final, 3 * x_u_winners > x_winners));

    if(P == 2)
      property = G(
        implies(x_state == c_app, F(x_state == c_final && x_u_winners > 1))
      );
  }
  

  formula to_check = system && !property;

  bool ok = xi.type_check(to_check, [](auto s) {
    std::cerr << "Type checking error: " << s << "\n";
    exit(1);
  });
  black_assert(ok);

  solver slv;

  slv.set_tracer(tracer);
  //slv.set_sat_backend("cvc5");

  tribool result = slv.solve(
    xi, to_check, 
    /* finite = */true, 
    std::numeric_limits<size_t>::max(),
    /* semi_decidable = */true);

  if(result == true)
    std::cout << "SAT\n";
  else if(result == false)
    std::cout << "UNSAT\n";
  else
    std::cout << "UNKNOWN\n";

  // if(result == true) {
  //   auto m = slv.model();
  //   black_assert(m);

  //   std::cout << m->dump();
  // }

  return 0;
}
