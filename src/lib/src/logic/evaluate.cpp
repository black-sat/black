//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <black/logic>
#include <black/ast/algorithms>

namespace black::logic {

  std::vector<term> evaluate(std::vector<term> const& ts) {
    std::vector<term> result;
    for(term t : ts)
      result.push_back(evaluate(t));
    return result;
  }

  term evaluate(term t) {
    using support::match;
    using ast::map;

    return map(t)(
      [&](integer v)       { return v; },
      [&](real v)          { return v; },
      [&](boolean v)       { return v; },
      [&](lambda v)        { return v; },
      [&](variable x)      { return x; },
      [&](object x, auto e) -> term {
        if(e->value)
          return evaluate(*e->value);
        return x;
      },
      [&](atom, term head, auto const& args) -> term {

        auto f = cast<lambda>(head);
        if(!f || f->vars().size() != args.size())
          return atom(head, args);
        
        module env;

        for(size_t i = 0; i < args.size(); i++)
          env.define({f->vars()[i].name, f->vars()[i].type, args[i]});

        return evaluate(env.resolved(f->body()));
      },
      [&](negation, term arg) -> term {
        if(auto v = cast<boolean>(arg); v)
          return boolean(!v->value());
        
        return negation(arg);
      },
      [&](conjunction, auto const& args) -> term {
        bool result = true;
        
        for(auto arg : args) {
          if(auto v = cast<boolean>(arg); v)
            result = result && v->value();
          else
            return conjunction(args);
        }

        return boolean(result);
      },
      [&](disjunction, auto const& args) -> term {
        bool result = false;
        
        for(auto arg : args) {
          if(auto v = cast<boolean>(arg); v)
            result = result || v->value();
          else
            return disjunction(args);
        }

        return boolean(result);
      },
      [&](implication, term left, term right) -> term {
        auto vl = cast<boolean>(left);
        auto vr = cast<boolean>(right);

        if(!vl || !vr)
          return implication(left, right);
        
        return boolean(!vl->value() || vr->value());
      },
      [&](ite, term guard, term iftrue, term iffalse) -> term {
        auto guardv = cast<boolean>(guard);
        if(!guardv)
          return ite(guard, iftrue, iffalse);

        if(guardv->value())
          return iftrue;
        return iffalse;
      },
      [&](minus, term arg) -> term {
        return match(arg)(
          [&](integer, auto value) {
            return integer(-value);
          },
          [&](real, auto value) {
            return real(-value);
          },
          [&](auto) {
            return minus(arg);
          }
        );
      },
      [&]<arithmetic T>(T op, term left, term right) -> term {
        auto ilv = cast<integer>(left);
        auto ilr = cast<integer>(right);
        if(ilv && ilr)
          return integer(
            match(op)(
              [&](sum) { return ilv->value() + ilr->value(); },
              [&](product) { return ilv->value() * ilr->value(); },
              [&](difference) { return ilv->value() - ilr->value(); },
              [&](division) { return ilv->value() / ilr->value(); }
            )
          );
        
        auto rlv = cast<real>(left);
        auto rlr = cast<real>(right);
        if(rlv && rlr)
          return real(
            match(op)(
              [&](sum) { return rlv->value() + rlr->value(); },
              [&](product) { return rlv->value() * rlr->value(); },
              [&](difference) { return rlv->value() - rlr->value(); },
              [&](division) { return rlv->value() / rlr->value(); }
            )
          );

        return T(left, right);
      },
      [&]<relational T>(T op, term left, term right) -> term {
        auto ilv = cast<integer>(left);
        auto ilr = cast<integer>(right);
        if(ilv && ilr)
          return boolean(
            match(op)(
              [&](less_than) { return ilv->value() < ilr->value(); },
              [&](less_than_eq) { return ilv->value() <= ilr->value(); },
              [&](greater_than) { return ilv->value() > ilr->value(); },
              [&](greater_than_eq) { return ilv->value() >= ilr->value(); }
            )
          );
        
        auto rlv = cast<real>(left);
        auto rlr = cast<real>(right);
        if(rlv && rlr)
          return boolean(
            match(op)(
              [&](less_than) { return rlv->value() < rlr->value(); },
              [&](less_than_eq) { return rlv->value() <= rlr->value(); },
              [&](greater_than) { return rlv->value() > rlr->value(); },
              [&](greater_than_eq) { return rlv->value() >= rlr->value(); }
            )
          );

        return T(left, right);
      }
    );
  }

}