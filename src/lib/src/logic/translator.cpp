//
// Created by gabriele on 16/07/2020.
//

#include <black/logic/translator.hpp>

// Implementation of translator class
namespace black::internal {
  /*formula translator::translate() {
    return substitute_past(_frm);
  }

  formula translator::substitute_past(formula f) {
    return f.match(
      [&](yesterday, formula op) {
        // return _alpha.var( std::pair(Y( substitute_past(op) ), "Y") );
        substitute_past(op);
        return _alpha.var( "Y" );
      },
      [&](since, formula left, formula right) {
        substitute_past(left);
        substitute_past(right);
        return _alpha.var( "S" );
      },
      [&](triggered, formula left, formula right) {
        return substitute_past( ! S( !left, !right ) );
      },
      [&](past, formula op) {
        return substitute_past( S( _alpha.top(), op) );
      },
      [&](historically, formula op) {
        return substitute_past( ! P( ! op ));
      },
      [](boolean b) { return b; },
      [](atom a)    { return a; },
      [&](unary u) {
        return unary(u.formula_type(), substitute_past(u.operand()));
      },
      [&](binary b) {
        return binary(b.formula_type(), substitute_past(b.left()), substitute_past(b.right()));
      },
      [](otherwise) {
        black_unreachable();
      }
    );
  }

  std::vector<formula> translator::gen_semantics(formula) {
    return std::vector<formula>();
  }*/
  
  formula remove_past(alphabet &alpha, formula f) {
    return f.match(
      [&](yesterday, formula op) {
        // return alpha.var( std::pair(Y( remove_past(alpha, op) ), "Y") );
        remove_past(alpha, op);
        return alpha.var( "Y" );
      },
      [&](since, formula left, formula right) {
        remove_past(alpha, left);
        remove_past(alpha, right);
        return alpha.var( "S" );
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
      [](boolean b) { return b; },
      [](atom a)    { return a; },
      [&](unary u) {
        return unary(u.formula_type(), remove_past(alpha, u.operand()));
      },
      [&](binary b) {
        return binary(b.formula_type(), remove_past(alpha, b.left()), remove_past(alpha, b.right()));
      },
      [](otherwise) {
        black_unreachable();
      }
    );
  }
}