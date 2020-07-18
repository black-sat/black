//
// Created by gabriele on 16/07/2020.
//

#include <black/logic/translator.hpp>

namespace black::internal {
  formula substitute_past(alphabet &alpha, formula f) {
    return f.match(
        [&](yesterday, formula op) {
          return alpha.var( std::pair( Y( substitute_past(alpha, op) ), "Y" ) );
        },
        [&](since, formula left, formula right) {
          return alpha.var( std::pair( "S", S( substitute_past(alpha, left), substitute_past(alpha, right) ) ) );
        },
        [&](triggered, formula left, formula right) {
          return substitute_past(alpha,  ! S( !left, !right ) );
        },
        [&](past, formula op) {
          return substitute_past(alpha,  S( alpha.top(), op) );
        },
        [&](historically, formula op) {
          return substitute_past(alpha,  ! P( ! op ));
        },
        [](boolean b) { return b; },
        [](atom a)    { return a; },
        [&](unary u, formula op) {
          return unary( u.formula_type(), substitute_past(alpha, op) );
        },
        [&](binary b, formula left, formula right) {
          return binary( b.formula_type(), substitute_past(alpha, left), substitute_past(alpha, right));
        },
        [](otherwise) {
          black_unreachable();
        }
    );
  }

  /*std::vector<formula> gen_semantics(alphabet &alpha, formula f) {
    return f.match(
      [&](atom a) {
        std::vector<formula> semantics;
        std::optional<std::any> label = a.label();

        if( label ) {
          std
        }

        if( (*label).first == "Y" || (*label).first == "S" ) {
          return label.second.match(
            [&](yesterday, formula op) {
              return semantics;
            },
            [&](since, formula left, formula right) {
              return semantics;
            },
            [](otherwise) {
              balck_unreachable();
            }
          );
        }

        return semantics;
      }
    );
  }*/

  formula ltlpast_to_ltl(alphabet &alpha, formula f) {
    return substitute_past(alpha, f);
  }

  // Old attempt to generate semantics and remove past in one shot
  /*std::pair<formula, std::vector<formula>> remove_past(alphabet &alpha, formula f) {
    return f.match(
      [&](yesterday, formula op) {
        std::pair<formula, std::vector<formula>> fs = remove_past(alpha, op);
        formula p = alpha.var( std::pair( "Y", Y( fs.first ) ) );

        std::vector<formula> semantics = { !p && G( iff( X(p), fs.first ) ) };

        return std::pair( p, semantics );
      },
      [&](since, formula left, formula right) {
        std::pair<formula, std::vector<formula>> fsl = remove_past(alpha, left);
        std::pair<formula, std::vector<formula>> fsr = remove_past(alpha, right);
        formula p = alpha.var( std::pair( "S", S( fsl.first, fsr.first ) ) );

        formula py = alpha.var( std::pair( "Y", Y( S( fsl.first, fsr.first ) ) ) );

        std::vector<formula> semantics = {
            G( iff( p, fsr.first || ( fsl.first && py ) ) ),
            !py && G( iff( X(py), S( fsl.first, fsr.first ) ) )
        };

        return std::pair( p, semantics );
      },
      [&](triggered, formula left, formula right) {
        return remove_past(alpha,  ! S( !left, !right ) );
      },
      [&](past, formula op) {
        return remove_past(alpha,  S( alpha.top(), op) );
      },
      [&](historically, formula op) {
        return remove_past(alpha,  ! P( ! op ));
      },
      [](boolean b) {
        std::vector<formula> semantics;
        return std::pair( b, semantics );
      },
      [](atom a) {
        std::vector<formula> semantics;
        return std::pair( a, semantics );
      },
      [&](unary u) {
        std::pair<formula, std::vector<formula>> fs = remove_past(alpha, u.operand());
        return std::pair( unary(u.formula_type(), fs.first ), fs.second );
      },
      [&](binary b) {
        std::pair<formula, std::vector<formula>> fsl = remove_past(alpha, b.left());
        std::pair<formula, std::vector<formula>> fsr = remove_past(alpha, b.right());

        return std::pair( binary(b.formula_type(), fsl.first, fsr.first), fsl.second.insert( fsl.second.end(), fsr.second.begin(), fsr.second.end() ) );
      },
      [](otherwise) {
        black_unreachable();
      }
    );
  }*/
}