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

namespace black::logic {

  std::vector<term> evaluate(std::vector<term> const& ts) {
    std::vector<term> result;
    for(term t : ts)
      result.push_back(evaluate(t));
    return result;
  }

  term evaluate(term t) {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](type_type v)     { return v; },
      [&](inferred_type v) { return v; },
      [&](integer_type v)  { return v; },
      [&](real_type v)     { return v; },
      [&](boolean_type v)  { return v; },
      [&](function_type v) { return v; },
      [&](integer v)       { return v; },
      [&](real v)          { return v; },
      [&](boolean v)       { return v; },
      [&](lambda v)        { return v; },
      [&](variable x)      { return x; },
      [&](object x, auto lookup) -> term {
        if(lookup->value)
          return evaluate(*lookup->value);
        return x;
      },
      //[&](type_cast c)   { return c.target(); },
      [&](atom, term head, auto const& args) -> term {

        term ehead = evaluate(head);
        std::vector<term> eargs = evaluate(args);

        auto f = cast<lambda>(ehead);
        if(!f || f->vars().size() != args.size())
          return atom(ehead, eargs);
        
        module env(sigma);

        for(size_t i = 0; i < args.size(); i++)
          env.define({f->vars()[i].name, f->vars()[i].type, eargs[i]});

        return evaluate(env.resolved(f->body()));
      },
      // equal, distinct
      [&]<quantifier T>(T, auto const& binds, term body) {
        return T(binds, evaluate(body));
      },
      [&]<temporal T>(T, auto ...args) {
        return T(evaluate(args)...);
      },
      [&](negation, term arg) -> term {
        term earg = evaluate(arg);
        if(auto v = cast<boolean>(earg); v)
          return sigma->boolean(!v->value());
        
        return negation(earg);
      },
      [&](conjunction, auto const& args) -> term {
        std::vector<term> eargs = evaluate(args);
        bool result = true;
        
        for(auto earg : eargs) {
          if(auto v = cast<boolean>(earg); v)
            result = result && v->value();
          return conjunction(eargs);
        }

        return sigma->boolean(result);
      },
      [&](disjunction, auto const& args) -> term {
        std::vector<term> eargs = evaluate(args);
        bool result = false;
        
        for(auto earg : eargs) {
          if(auto v = cast<boolean>(earg); v)
            result = result || v->value();
          return conjunction(eargs);
        }

        return sigma->boolean(result);
      },
      [&](implication, term left, term right) -> term {
        term eleft = evaluate(left);
        term eright = evaluate(right);
        
        auto vl = cast<boolean>(eleft);
        auto vr = cast<boolean>(eright);

        if(!vl || !vr)
          return implication(eleft, eright);
        
        return sigma->boolean(!vl->value() || vr->value());
      },
      [&](ite, term guard, term iftrue, term iffalse) -> term {
        term eguard = evaluate(guard);
        term etrue = evaluate(iftrue);
        term efalse = evaluate(iffalse);

        auto guardv = cast<boolean>(eguard);
        if(!guardv)
          return ite(eguard, etrue, efalse);

        if(guardv->value())
          return etrue;
        return efalse;
      },
      [&](minus, term arg) -> term {
        term earg = evaluate(arg);
        
        return match(earg)(
          [&](integer, auto value) {
            return sigma->integer(-value);
          },
          [&](real, auto value) {
            return sigma->real(-value);
          },
          [&](auto) {
            return minus(earg);
          }
        );
      },
      [&]<arithmetic T>(T op, term left, term right) -> term {
        term eleft = evaluate(left);
        term eright = evaluate(right);

        auto ilv = cast<integer>(eleft);
        auto ilr = cast<integer>(eright);
        if(ilv && ilr)
          return sigma->integer(
            match(op)(
              [&](sum) { return ilv->value() + ilr->value(); },
              [&](product) { return ilv->value() * ilr->value(); },
              [&](difference) { return ilv->value() - ilr->value(); },
              [&](division) { return ilv->value() / ilr->value(); }
            )
          );
        
        auto rlv = cast<real>(eleft);
        auto rlr = cast<real>(eright);
        if(rlv && rlr)
          return sigma->real(
            match(op)(
              [&](sum) { return rlv->value() + rlr->value(); },
              [&](product) { return rlv->value() * rlr->value(); },
              [&](difference) { return rlv->value() - rlr->value(); },
              [&](division) { return rlv->value() / rlr->value(); }
            )
          );

        return T(eleft, eright);
      },
      [&]<relational T>(T op, term left, term right) -> term {
        term eleft = evaluate(left);
        term eright = evaluate(right);

        auto ilv = cast<integer>(eleft);
        auto ilr = cast<integer>(eright);
        if(ilv && ilr)
          return sigma->boolean(
            match(op)(
              [&](less_than) { return ilv->value() < ilr->value(); },
              [&](less_than_eq) { return ilv->value() <= ilr->value(); },
              [&](greater_than) { return ilv->value() > ilr->value(); },
              [&](greater_than_eq) { return ilv->value() >= ilr->value(); }
            )
          );
        
        auto rlv = cast<real>(eleft);
        auto rlr = cast<real>(eright);
        if(rlv && rlr)
          return sigma->boolean(
            match(op)(
              [&](less_than) { return rlv->value() < rlr->value(); },
              [&](less_than_eq) { return rlv->value() <= rlr->value(); },
              [&](greater_than) { return rlv->value() > rlr->value(); },
              [&](greater_than_eq) { return rlv->value() >= rlr->value(); }
            )
          );

        return T(eleft, eright);
      }
    );
  }

}