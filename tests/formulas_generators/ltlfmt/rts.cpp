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

inline auto frame_axiom(
  scope &xi, std::vector<relation> &rels, std::optional<size_t> except
) {
  formula axiom = xi.sigma()->top();

  for(size_t i = 0; i < rels.size(); ++i) {
    if(i == except)
      continue;
    axiom = axiom && frame_axiom(xi, rels[i]);
  }

  return axiom;
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

  formula winners_primary_key = 
    forall({u1[user], s1[score], s2[score]}, 
      implies(winners(u1, s1) && winners(u1, s2), s1 == s2)
    );

  formula axioms = winners_primary_key;

  formula reset =
    x_status == c_init &&
    forall({u1[user], s1[score]}, !applications(u1, s1));

  formula init = reset && x_evaluations == N &&
    forall({u1[user], s1[score]}, !winners(u1, s1));

  formula phi_tr = sigma.bottom();

  phi_tr = phi_tr || (
    exists({u1[user], s1[score]}, 
      (x_status == c_init || x_status == c_app_phase) && 
      s1 >= 1 && s1 <= 100 && 
      !applications(u1, s1) &&
      wnext(x_evaluations) == x_evaluations &&
      wnext(x_status) == c_app_phase &&
      wX(applications(u1, s1)) &&
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
    x_status == c_app_phase &&
    wnext(x_evaluations) == x_evaluations && 
    wnext(x_status) == c_apps_rcvd &&
    frame_axiom(xi, applications) &&
    frame_axiom(xi, winners)
  );

  phi_tr = phi_tr || (
    exists({u1[user], s1[score]}, 
      x_status == c_apps_rcvd &&
      applications(u1, s1) &&
      wnext(x_evaluations) == x_evaluations &&
      //s1 > 80 &&
      wnext(x_status) == c_evaluated &&
      wX(winners(u1, s1)) &&
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
    x_status == c_evaluated && x_evaluations > 0 &&
    wX(reset) &&
    wnext(x_evaluations) == x_evaluations - 1 &&
    frame_axiom(xi, winners)
  );

  phi_tr = phi_tr || (
    x_status == c_evaluated && x_evaluations == 0 && 
    wnext(x_status) == c_final &&
    exists({u1[user], s1[score]},
      winners(u1, s1) &&
      wnext(x_winner) == u1 &&
      wnext(x_score) == s1
    ) && frame_axiom(xi, winners)
  );

  phi_tr = phi_tr || (
    x_status == c_final && wX(sigma.bottom())
  );

  [[maybe_unused]]
  formula final = x_status == c_final;

  formula system = axioms && init && G(phi_tr) && F(final);

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

  tribool result = slv.solve(xi, to_check, /* finite = */true);

  if(result == true)
    std::cout << "SAT\n";
  else if(result == false)
    std::cout << "UNSAT\n";
  else
    std::cout << "UNKNOWN\n";

  return 0;
}
