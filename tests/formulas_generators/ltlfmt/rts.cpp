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

using namespace black;

inline auto app(alphabet &sigma, int i) {
  return sigma.relation(std::tuple{"app", i});
}

int main() {
  alphabet sigma;

  auto user = sigma.relation("user");
  auto team = sigma.relation("team");
  
  std::vector<relation> apps;
  const int M = 3;

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
  auto c_init = sigma.variable("c_init");
  auto c_undef = sigma.variable("c_undef");
  auto c_app_phase = sigma.variable("c_app_phase");
  auto c_apps_rcvd = sigma.variable("c_apps_rcvd");
  auto c_team_rcvd = sigma.variable("c_team_rcvd");
  auto c_team_created = sigma.variable("c_team_created");
  auto c_evaluated = sigma.variable("c_evaluated");
  auto c_programmers = sigma.variable("c_programmers");

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

  formula user_primary_key = 
    forall_block({x1, x2, x3}, 
      implies(user(x1, x2) && user(x1, x3), x2 == x3)
    );

  formula user_rigid = 
    G(forall_block({x1, x2}, iff(user(x1, x2), X(user(x1, x2)))));

  formula team_primary_key =
    forall_block({x1, x2, x3, x4, x5},
      implies(team(x1, x2, x3) && team(x1, x4, x5), 
        x2 == x4 && x3 == x5
      )
    );
  formula team_rigid = 
    G(forall_block({x1, x2, x3}, iff(user(x1, x2, x3), X(user(x1, x2, x3)))));

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
      if(cc1 != cc2)
        axioms = axioms && G(cc1 != cc2);
    }
  }

  formula init =
    x_status == c_init &&
    x_win == c_undef &&
    x_score == 0;

  for(size_t i = 0; i < M; ++i) {
    init = init && forall_block({x1, x2, x3, x4}, !apps[i](x1, x2, x3 /*,x4*/));
  }

  init = init && forall_block({x1, x2, x3, x4}, !app_team(x1, x2, x3 /*,x4*/));

  formula goal =
    x_status == c_evaluated && x_score < 80;

  formula phi_tr = sigma.bottom();

  for(size_t i = 0; i < M; ++i) {
    phi_tr = phi_tr || 
      exists_block({u1, n1, c1, s1}, 
        (x_status == c_init || x_status == c_app_phase) && 
        user(u1, n1) && s1 >= 1 && s1 <= 100 && 
        !apps[i](u1, c1, s1) &&
        wnext(x_status) == c_app_phase &&
        wnext(x_win) == x_win &&
        wnext(x_score) == x_score &&
        forall_block({x1, x2, x3},
          (
            implies(x1 == u1 && x2 == c1 && x3 == s1, X(apps[i](x1, x2, x3))) &&
            implies(x1 != u1 && x2 != c1 && x3 != s1, 
              iff(apps[i](x1, x2, x3), X(apps[i](x1, x2, x3)) 
              /* && frame axiom tutte le apps tranne apps[i] */
            )
          )
        )
      )
    );
  }

  phi_tr = phi_tr || 
    implies(x_status == c_app_phase, 
      wnext(x_status) == c_apps_rcvd &&
      wnext(x_win) == x_win &&
      wnext(x_score) == x_score
      /* && frame axiom tutte le apps[i] */
    );

  
}
