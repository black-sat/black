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

int main() {
  alphabet sigma;

  auto user = sigma.relation("user");
  auto team = sigma.relation("team");
  
  std::vector<relation> apps;
  const int M = 10;

  for(int i = 0; i < M; ++i) {
    apps.push_back(app(sigma, i));
  }

  auto app_team = sigma.relation("app_team");

  auto x1 = sigma.variable("x1");
  auto x2 = sigma.variable("x2");
  auto x3 = sigma.variable("x3");
  auto x4 = sigma.variable("x4");
  auto x5 = sigma.variable("x5");
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

  auto x_status = sigma.variable("x_status");
  auto x_win = sigma.variable("x_win");
  auto x_score = sigma.variable("x_score");
  auto x_evaluations = sigma.variable("x_evaluations");

  auto c_init = sigma.variable("c_init");
  auto c_undef = sigma.variable("c_undef");
  auto c_app_phase = sigma.variable("c_app_phase");
  auto c_apps_rcvd = sigma.variable("c_apps_rcvd");
  auto c_team_rcvd = sigma.variable("c_team_rcvd");
  auto c_team_created = sigma.variable("c_team_created");
  auto c_evaluated = sigma.variable("c_evaluated");
  auto c_programmers = sigma.variable("c_programmers");

  [[maybe_unused]]
  variable constants[] = {
    c_init,
    c_undef,
    c_app_phase,
    c_apps_rcvd,
    c_team_rcvd,
    c_team_created,
    c_evaluated,
    c_programmers
  };

  [[maybe_unused]]
  formula user_primary_key = 
    forall_block({x1, x2, x3}, 
      implies(user(x1, x2) && user(x1, x3), x2 == x3)
    );

  [[maybe_unused]]
  formula user_rigid = 
    G(forall_block({x1, x2}, 
      implies(user(x1, x2), wX(user(x1, x2))) &&
      implies(X(user(x1, x2)), user(x1, x2))
    ));

  [[maybe_unused]]
  formula team_primary_key =
    forall_block({x1, x2, x3, x4, x5},
      implies(team(x1, x2, x3) && team(x1, x4, x5), 
        x2 == x4 && x3 == x5
      )
    );

  [[maybe_unused]]
  formula team_rigid = 
    G(forall_block({x1, x2, x3}, 
      implies(X(sigma.top()), iff(team(x1, x2, x3), wX(team(x1, x2, x3))))));

  [[maybe_unused]]
  formula user_team_exclusion =
    forall_block({x1, x2, x3, x4}, 
      implies(user(x1, x2), !team(x1, x3, x4))
    );
  
  [[maybe_unused]]
  formula foreign_key = 
    forall_block({x1, x2, x3},
      implies(
        team(x1, x2, x3), 
        exists_block({x4, x5}, user(x2, x4) && user(x3, x5))
      )
    );


  // formula apps_primary_key = sigma.top();

  // for(int i = 0; i < M; ++i) {
  //   auto key = G(forall_block({x1, x2, x3, x4, x5, x6, x7},
  //     implies(apps[i](x1, x2, x3, x4) && apps[i](x1, x5, x6, x7),
  //       x2 == x5 && x3 == x6 && x4 == x7
  //     )
  //   ));

  //   apps_primary_key = apps_primary_key && key;
  // }

  // formula team_primary_key = 
  //   G(forall_block({x1, x2, x3, x4, x5, x6, x7},
  //     implies(team_app(x1, x2, x3, x4) && team_app(x1, x5, x6, x7),
  //       x2 == x5 && x3 == x6 && x4 == x7
  //     )
  //   ));
    

  formula axioms = 
    user_primary_key &&
    user_rigid && 
    team_primary_key &&
    team_rigid;
    // apps_primary_key &&
    //team_primary_key;

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
    x_win == c_undef &&
    x_score == 0;

  for(size_t i = 0; i < M; ++i) {
    reset = reset && 
      forall_block({x1, x2, x3 /*, x4 */}, !apps[i](x1, x2, x3 /*,x4*/));
  }

  reset = reset && 
    forall_block({x1, x2, x3 /*, x4 */}, !app_team(x1, x2, x3 /*,x4*/));

  const int N = 3;

  [[maybe_unused]]
  formula init = reset && x_evaluations == N;

  formula phi_tr = sigma.bottom();

  for(size_t i = 0; i < M; ++i) {
    phi_tr = phi_tr || 
      exists_block({u1, n1, c1, s1}, 
        (x_status == c_init || x_status == c_app_phase) && 
        user(u1, n1) && s1 >= 1 && s1 <= 100 && 
        !apps[i](u1, c1, s1) &&
        wnext(x_evaluations) == x_evaluations &&
        wnext(x_status) == c_app_phase &&
        wnext(x_win) == x_win &&
        wnext(x_score) == x_score &&
        wX(apps[i](u1, c1, s1)) &&
        forall_block({x1, x2, x3},
          (
            implies(x1 != u1 && x2 != c1 && x3 != s1, 
              implies(X(sigma.top()), 
                iff(apps[i](x1, x2, x3), wX(apps[i](x1, x2, x3)))
            )
          )
        )
      ) && frame_axiom(sigma, apps, i, 3) 
        && frame_axiom(sigma, app_team, 3)
    );
  }

  phi_tr = phi_tr || 
    x_status == c_app_phase &&
    wnext(x_evaluations) == x_evaluations && 
    wnext(x_status) == c_apps_rcvd &&
    wnext(x_win) == x_win &&
    wnext(x_score) == x_score &&
    frame_axiom(sigma, apps, std::nullopt, 3) &&
    frame_axiom(sigma, app_team, 3);

  for(size_t i = 0; i < M; ++i) {
    phi_tr = phi_tr ||
      exists_block({u1, c1, s1}, 
        x_status == c_apps_rcvd &&
        apps[i](u1, c1, s1) &&
        wnext(x_evaluations) == x_evaluations &&
        //s1 > 80 &&
        wnext(x_status) == c_evaluated &&
        wnext(x_win) == u1 && 
        wnext(x_score) == s1
      ) && frame_axiom(sigma, apps, std::nullopt, 3) 
        && frame_axiom(sigma, app_team, 3);
  }

  phi_tr = phi_tr || (
    x_status == c_evaluated && 
    wX(reset) &&
    wnext(x_evaluations) == x_evaluations - 1 
  );
  
  formula final = x_status == c_evaluated && x_evaluations == 0;

  formula system = axioms && init && G(phi_tr) && F(final);

  // -------

  formula property1 =
    G(implies(x_status == c_evaluated, x_score > 60));
  
  formula property2 =
    G(implies(x_status == c_app_phase, F(x_status == c_evaluated)));

  sigma.set_default_sort(sigma.integer_sort());
  solver slv; 

  slv.set_sat_backend("z3");
  slv.set_formula(system && !property1, true);

  auto res = slv.solve(std::numeric_limits<size_t>::max(), true);

  if(res == true)
    std::cout << "SAT\n";
  else if(res == tribool::undef)
    std::cout << "UNKNOWN\n";
  else
    std::cout << "UNSAT\n";
  
  slv.set_formula(system && !property2, true);

  res = slv.solve(std::numeric_limits<size_t>::max(), true);

  if(res == true)
    std::cout << "SAT\n";
  else if(res == tribool::undef)
    std::cout << "UNKNOWN\n";
  else
    std::cout << "UNSAT\n";

  return 0;
}
