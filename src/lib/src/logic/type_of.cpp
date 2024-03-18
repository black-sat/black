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

  std::vector<term> type_of(std::vector<term> const& ts) {
    std::vector<term> result;
    for(term t : ts)
      result.push_back(type_of(t));
    return result;
  }

  term type_of(term t) {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](error e)       { return e; },
      [&](type_type)     { return sigma->type_type(); },
      [&](inferred_type) { return sigma->type_type(); },
      [&](integer_type)  { return sigma->type_type(); },
      [&](real_type)     { return sigma->type_type(); },
      [&](boolean_type)  { return sigma->type_type(); },
      [&](function_type) { return sigma->type_type(); },
      [&](integer)       { return sigma->integer_type(); },
      [&](real)          { return sigma->real_type(); },
      [&](boolean)       { return sigma->boolean_type(); },
      [&](equal)         { return sigma->boolean_type(); },
      [&](distinct)      { return sigma->boolean_type(); },
      [&](type_cast c)   { return c.target(); },
      [&](variable x) -> term {
        return error(x, "use of unbound free variable");
      },
      [&](object, auto lookup) {
        return lookup->type;
      },
      [&](atom a, term head, auto const& args) -> term {
        auto fty = cast<function_type>(type_of(head));
        if(!fty)
          return error(a, "calling a non-function");
          
        if(args.size() != fty->parameters().size()) 
          return error(a, "argument number mismatch in function call");

        for(size_t i = 0; i < args.size(); i++) {
          auto type = type_of(args[i]);

          if(type != fty->parameters()[i])
            return error(a, "type mismatch in function call");
        }

        return fty->range();
      },
      [&](quantifier auto, auto const& decls, term body) -> term {
        module env(sigma);
        for(decl d : decls)
          env.declare(d);

        term bodyty = type_of(env.resolved(body));
        if(!cast<boolean_type>(bodyty))
          return error(body, "quantified terms must be boolean");
        
        return sigma->boolean_type();
      },
      [&](any_of<negation, implication> auto c, auto ...args) -> term {
        term argtypes[] = {type_of(args)...};
        for(auto ty : argtypes) {
          if(!cast<boolean_type>(ty))
            return 
              error(c, "connectives can be applied only to boolean terms");
        }

        return sigma->boolean_type();
      },
      [&](any_of<conjunction, disjunction> auto c, auto const& args) -> term {
        for(term arg : args) 
          if(!cast<boolean_type>(type_of(arg)))
            return 
              error(c, "connectives can be applied only to boolean terms");

        return sigma->boolean_type();
      },
      [&](ite f, term guard, term iftrue, term iffalse) -> term {
        if(!cast<boolean_type>(type_of(guard)))
          return error(f, "the guard of an `ite` expression must be boolean");

        auto truety = type_of(iftrue);
        auto falsety = type_of(iffalse);
        if(truety != falsety)
          return 
            error(f,
              "the two cases of an `ite` expression must have the same type"
            );

        return truety;
      },
      [&](lambda, auto const& decls, term body) -> term {
        std::vector<term> argtypes;
        for(decl d : decls)
          argtypes.push_back(d.type);
        
        module env(sigma);
        for(decl d : decls)
          env.declare(d);

        auto bodyty = type_of(env.resolved(body));
        return function_type(std::move(argtypes), bodyty);
      },
      // case_of...
      [&](temporal auto tm, auto ...args) -> term {
        term argtypes[] = {type_of(args)...};
        for(auto ty : argtypes)
          if(!cast<boolean_type>(ty))
            return 
              error(tm,
                "temporal operators can be applied only to boolean terms"
              );

        return sigma->boolean_type();
      },
      [&](minus m, term arg) -> term {
        term type = type_of(arg);
        
        if(
          bool(type != sigma->integer_type()) && 
          bool(type != sigma->real_type())
        )
          return 
            error(m, "arithmetic operators only work on integers or reals");
        
        return type;
      },
      [&](arithmetic auto a, term left, term right) -> term {
        term type1 = type_of(left);
        term type2 = type_of(right);
        
        if(
          bool(type1 != sigma->integer_type()) && 
          bool(type1 != sigma->real_type())
        )
          return 
            error(a, 
              "left side of arithmetic operator must be integer or real"
            );
        
        if(
          bool(type2 != sigma->integer_type()) && 
          bool(type2 != sigma->real_type())
        )
          return 
            error(a, 
              "right side of arithmetic operator must be integer or real"
            );

        if(type1 != type2)
          return 
            error(a, "arithmetic operators can only be applied to equal types");
        
        return type1;
      },
      [&](relational auto r, term left, term right) -> term {
        term type1 = type_of(left);
        term type2 = type_of(right);
        
        if(
          bool(type1 != sigma->integer_type()) && 
          bool(type1 != sigma->real_type())
        )
          return 
            error(r, 
              "left side of relational operator must be integer or real"
            );
        
        if(
          bool(type2 != sigma->integer_type()) && 
          bool(type2 != sigma->real_type())
        )
          return 
            error(r, 
              "right side of relational operator must be integer or real"
            );

        if(type1 != type2)
          return 
            error(r, "relational operators can only be applied to equal types");
        
        return sigma->boolean_type();
      }
    );
  }

}