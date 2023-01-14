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

int main(int argc, char **argv) {

  if(argc < 3) {
    std::cerr << "Please specify number of phases\n";
    return 1;
  }

  int P = atoi(argv[1]);
  int N = atoi(argv[2]);

  if(P != 1 && P != 2) {
    std::cerr << "Property must be 1 or 2\n";
    return 1;
  }

  if(N <= 0) {
    std::cerr << "N must be positive\n";
    return 1;
  }

  std::cout << "Solving property " << P << " for N = " << N << "...\n";

  alphabet sigma;
  scope xi{sigma};

  auto u1 = sigma.variable("u1");
  auto u2 = sigma.variable("u2");
  auto s1 = sigma.variable("s1");
  auto s2 = sigma.variable("s2");

  auto user = sigma.named_sort("user");
  auto state = sigma.named_sort("state");
  auto score = sigma.integer_sort();

  auto applications = sigma.relation("applications");
  auto winners = sigma.relation("winners");

  xi.declare(applications, {user, score});
  xi.declare(winners, {user, score});

  auto x_status = sigma.variable("x_status");
  auto x_winner = sigma.variable("x_winner");
  auto x_score = sigma.variable("x_score");
  auto x_evaluations = sigma.variable("x_evaluations");

  xi.declare(x_status, state);
  xi.declare(x_winner, user);
  xi.declare(x_score, score);
  xi.declare(x_evaluations, sigma.integer_sort());

  auto c_init = sigma.variable("c_init");
  auto c_app_phase = sigma.variable("c_app_phase");
  auto c_apps_rcvd = sigma.variable("c_apps_rcvd");
  auto c_evaluated = sigma.variable("c_evaluated");
  auto c_final = sigma.variable("c_final");

  xi.declare(state, make_domain({
    c_init, c_app_phase, c_apps_rcvd, c_evaluated, c_final
  }));
  
  // auto c_undef_user = sigma.variable("c_undef_user");
  // xi.declare(c_undef_user, user, scope::rigid);

  formula winners_primary_key = 
    forall({u1[user], s1[score], s2[score]}, 
      implies(winners(u1, s1) && winners(u1, s2), s1 == s2)
    );
  
  formula applications_primary_key = 
    forall({u1[user], s1[score], s2[score]}, 
      implies(applications(u1, s1) && applications(u1, s2), s1 == s2)
    );

  formula axioms = winners_primary_key && applications_primary_key;

  formula reset =
    x_status == c_init &&
    forall({u1[user], s1[score]}, !applications(u1, s1));

  formula init = reset && x_evaluations == N - 1 && 
    x_score == 0 && //x_winner == c_undef_user &&
    forall({u1[user], s1[score]}, !winners(u1, s1));

  formula phi_tr = sigma.bottom();

  phi_tr = phi_tr || (
    exists({u1[user], s1[score]}, 
      // guard
      (x_status == c_init || x_status == c_app_phase) && 
      s1 >= 1 && s1 <= 100 && 
      !applications(u1, s1) &&
      // update
      wnext(x_status) == c_app_phase &&
      wnext(x_evaluations) == x_evaluations &&
      wnext(x_score) == x_score &&
      wnext(x_winner) == x_winner &&
      wX(applications(u1, s1)) &&
      // frames
      forall({u2[user], s2[score]},
        (
          implies(u2 != u1 && s2 != s1, 
            implies(X(sigma.top()), 
              iff(applications(u2, s2), wX(applications(u2, s2)))
            )
          )
        )
      ) && frame_axiom(xi, winners) 
    )
  );

  phi_tr = phi_tr || (
    // guard
    x_status == c_app_phase &&
    // update
    wnext(x_evaluations) == x_evaluations && 
    wnext(x_status) == c_apps_rcvd &&
    wnext(x_score) == x_score &&
    wnext(x_winner) == x_winner &&
    // frames
    frame_axiom(xi, applications) &&
    frame_axiom(xi, winners)
  );

  phi_tr = phi_tr || (
    exists({u1[user], s1[score]}, 
      // guard
      x_status == c_apps_rcvd &&
      applications(u1, s1) &&
      //s1 > 80 &&
      // update
      wnext(x_evaluations) == x_evaluations &&
      wnext(x_status) == c_evaluated &&
      wnext(x_score) == x_score &&
      wnext(x_winner) == x_winner &&
      wX(winners(u1, s1)) &&
      // frames
      forall({u2[user], s2[score]},
        implies(u2 != u1 && s2 != s1, 
          implies(X(sigma.top()), 
            iff(winners(u2, s2), wX(winners(u2, s2)))
          )
        )
      )
    ) && frame_axiom(xi, applications)
  );

  phi_tr = phi_tr || (
    // guard
    x_status == c_evaluated && x_evaluations > 0 &&
    // update
    wX(reset) &&
    wnext(x_score) == x_score &&
    wnext(x_winner) == x_winner &&
    wnext(x_evaluations) == x_evaluations - 1 &&
    // frames
    frame_axiom(xi, winners)
  );

  phi_tr = phi_tr || (
    exists({u1[user], s1[score]},
      // guard
      x_status == c_evaluated && x_evaluations == 0 && 
      winners(u1, s1) &&
      // update
      wnext(x_status) == c_final &&
      wnext(x_winner) == u1 &&
      wnext(x_score) == s1
    ) && 
    // frames
    frame_axiom(xi, winners)
  );

  [[maybe_unused]]
  formula final = x_status == c_final;

  formula system = 
    init && G(axioms) && G(implies(X(sigma.top()), phi_tr)) && F(final);

  // -------

  formula property = sigma.top();

  if(P == 1)
    property =
      G(implies(x_status == c_final, x_score > 60));

  if(P == 2)
    property = G(
      implies(x_status == c_app_phase, F(x_status == c_final && x_score > 60))
    );

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
