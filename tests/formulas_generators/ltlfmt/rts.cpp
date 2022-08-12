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

inline auto app(alphabet &sigma, int i) {
  using namespace std::literals;
  return sigma.relation(std::pair{"app"sv, i});
}

inline auto frame_axiom(
  alphabet &sigma, relation rel, size_t arity
) {
  using namespace std::literals;

  std::vector<variable> vars;
  for(size_t i = 0; i < arity; ++i) {
    vars.push_back(sigma.variable(std::pair{"x"sv, i}));
  }

  return forall_block(vars, 
    implies(X(sigma.top()), iff(rel(vars), wX(rel(vars)))));
}

inline auto frame_axiom(
  alphabet &sigma, std::vector<relation> &rels, 
  std::optional<size_t> except, size_t arity
) {
  formula axiom = sigma.top();

  for(size_t i = 0; i < rels.size(); ++i) {
    if(i == except)
      continue;
    axiom = axiom && frame_axiom(sigma, rels[i], arity);
  }

  return axiom;
}

int main(int argc, char **argv) {

  if(argc < 2) {
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

  auto user = sigma.relation("user");
  
  auto applications = sigma.relation("applications");
  auto winners = sigma.relation("winners");

  auto x1 = sigma.variable("x1");
  auto x2 = sigma.variable("x2");
  auto x3 = sigma.variable("x3");
  // auto x4 = sigma.variable("x4");
  // auto x5 = sigma.variable("x5");
  //auto x6 = sigma.variable("x6");
  // auto x7 = sigma.variable("x7");
  auto u1 = sigma.variable("u1");
  //auto u2 = sigma.variable("u2");
  //auto u3 = sigma.variable("u3");
  auto s1 = sigma.variable("s1");
  // auto s2 = sigma.variable("s2");
  //auto s3 = sigma.variable("s3");
  auto c1 = sigma.variable("c1");
  auto n1 = sigma.variable("n1");

  auto x_status = sigma.variable("x_status"); [[maybe_unused]]
  auto x_winner = sigma.variable("x_winner");
  auto x_score = sigma.variable("x_score");
  auto x_evaluations = sigma.variable("x_evaluations");

  auto c_init = sigma.variable("c_init");
  auto c_app_phase = sigma.variable("c_app_phase");
  auto c_apps_rcvd = sigma.variable("c_apps_rcvd");
  auto c_evaluated = sigma.variable("c_evaluated");
  auto c_final = sigma.variable("c_final");

  [[maybe_unused]]
  variable constants[] = {
    c_init,
    c_app_phase,
    c_apps_rcvd,
    c_evaluated,
    c_final
  };

  formula user_primary_key = 
    forall_block({x1, x2, x3}, 
      implies(user(x1, x2) && user(x1, x3), x2 == x3)
    );

  formula winners_primary_key = 
    forall_block({x1, x2, x3}, 
      implies(winners(x1, x2) && winners(x1, x3), x2 == x3)
    );

  formula user_rigid = 
    G(forall_block({x1, x2}, 
      implies(user(x1, x2), wX(user(x1, x2))) &&
      implies(X(user(x1, x2)), user(x1, x2))
    ));

  formula axioms = 
    user_primary_key &&
    winners_primary_key &&
    user_rigid;

  for(auto c : constants) {
    axioms = axioms && G(wnext(c) == c);
  }

  for(auto cc1 : constants) {
    for(auto cc2 : constants) {
      if(cc1 != cc2) {
        axioms = axioms && cc1 != cc2;
      }
    }
  }

  formula reset =
    x_status == c_init &&
    forall_block({x1, x2, x3}, !applications(x1, x2, x3));

  formula init = reset && x_evaluations == N &&
    forall_block({x1, x2}, !winners(x1, x2));

  formula phi_tr = sigma.bottom();

  phi_tr = phi_tr || (
    exists_block({u1, n1, c1, s1}, 
      (x_status == c_init || x_status == c_app_phase) && 
      user(u1, n1) && s1 >= 1 && s1 <= 100 && 
      !applications(u1, c1, s1) &&
      wnext(x_evaluations) == x_evaluations &&
      wnext(x_status) == c_app_phase &&
      wX(applications(u1, c1, s1)) &&
      forall_block({x1, x2, x3},
        (
          implies(x1 != u1 && x2 != c1 && x3 != s1, 
            implies(X(sigma.top()), 
              iff(applications(x1, x2, x3), wX(applications(x1, x2, x3)))
            )
          )
        )
      ) && frame_axiom(sigma, winners, 2)
    )
  );

  phi_tr = phi_tr || (
    x_status == c_app_phase &&
    wnext(x_evaluations) == x_evaluations && 
    wnext(x_status) == c_apps_rcvd &&
    frame_axiom(sigma, applications, 3) &&
    frame_axiom(sigma, winners, 2)
  );

  phi_tr = phi_tr || (
    exists_block({u1, c1, s1}, 
      x_status == c_apps_rcvd &&
      applications(u1, c1, s1) &&
      wnext(x_evaluations) == x_evaluations &&
      //s1 > 80 &&
      wnext(x_status) == c_evaluated 
      &&
      wX(winners(u1, s1))
      &&
      forall_block({x1, x2},
        implies(x1 != u1 && x2 != s1, 
          implies(X(sigma.top()), 
            iff(winners(x1, x2), wX(winners(x1, x2)))
          )
        )
      )
    ) && frame_axiom(sigma, applications, 3)
  );

  phi_tr = phi_tr || (
    x_status == c_evaluated && x_evaluations > 0 &&
    wX(reset) &&
    wnext(x_evaluations) == x_evaluations - 1 &&
    frame_axiom(sigma, winners, 2)
  );

  phi_tr = phi_tr || (
    x_status == c_evaluated && x_evaluations == 0 && 
    wnext(x_status) == c_final &&
    exists_block({u1, s1},
      winners(u1, s1) &&
      wnext(x_winner) == u1 &&
      wnext(x_score) == s1
    ) && frame_axiom(sigma, winners, 2)
  );

  phi_tr = phi_tr || (
    x_status == c_final && wX(sigma.bottom())
  );

  [[maybe_unused]]
  formula final = x_status == c_final;

  formula system = axioms && init && G(phi_tr) && F(final);

  // -------

  if(P == 1) {
    formula property =
      G(implies(x_status == c_final, x_score > 60));

    std::cout << to_string(system && !property) << "\n";
  }

  if(P == 2) {
    formula property = G(
      implies(x_status == c_app_phase, F(x_status == c_final && x_score > 60))
    );

    std::cout << to_string(system && !property) << "\n";
  }

  return 0;
}
