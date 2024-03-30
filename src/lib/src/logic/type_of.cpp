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

  std::vector<type> type_of(std::vector<term> const& ts) {
    std::vector<type> result;
    for(term t : ts)
      result.push_back(type_of(t));
    return result;
  }

  type type_of(term t) {
    using support::match;

    return match(t)(
      [&](error e, term, auto err) { 
        return types::error(e, err); 
      },
      [&](integer)       { return types::integer(); },
      [&](real)          { return types::real(); },
      [&](boolean)       { return types::boolean(); },
      [&](equal)         { return types::boolean(); },
      [&](distinct)      { return types::boolean(); },
      [&](variable x) -> type {
        return types::error(
          x, std::format("use of unbound free variable: {}", x.name())
        );
      },
      [&](object, auto e) {
        return e->type;
      },
      [&](atom a, term head, auto const& args) -> type {
        auto fty = cast<types::function>(type_of(head));
        if(!fty)
          return types::error(a, "calling a non-function");
          
        if(args.size() != fty->parameters().size()) 
          return types::error(a, "argument number mismatch in function call");

        for(size_t i = 0; i < args.size(); i++) {
          auto type = type_of(args[i]);

          if(type != fty->parameters()[i])
            return types::error(a, "type mismatch in function call");
        }

        return fty->range();
      },
      [&](quantifier auto, auto const& decls, term body) -> type {
        module env;
        for(decl d : decls)
          env.declare(d);

        type bodyty = type_of(env.resolved(body));
        if(!cast<types::boolean>(bodyty))
          return types::error(body, "quantified terms must be boolean");
        
        return types::boolean();
      },
      [&](any_of<negation, implication> auto c, auto ...args) -> type {
        type argtypes[] = {type_of(args)...};
        for(auto ty : argtypes) {
          if(!cast<types::boolean>(ty))
            return types::error(
              c, "connectives can be applied only to boolean terms"
            );
        }

        return types::boolean();
      },
      [&](any_of<conjunction, disjunction> auto c, auto const& args) -> type {
        for(term arg : args) 
          if(!cast<types::boolean>(type_of(arg)))
            return types::error(
              c, "connectives can be applied only to boolean terms"
            );

        return types::boolean();
      },
      [&](ite f, term guard, term iftrue, term iffalse) -> type {
        type ty = type_of(guard);
        if(!cast<types::boolean>(ty))
          return 
            types::error(f, "the guard of an `ite` expression must be boolean");

        auto truety = type_of(iftrue);
        auto falsety = type_of(iffalse);
        if(truety != falsety)
          return 
            types::error(f,
              "the two cases of an `ite` expression must have the same type"
            );

        return truety;
      },
      [&](lambda, auto const& decls, term body) -> type {
        std::vector<type> argtypes;
        for(decl d : decls)
          argtypes.push_back(d.type);
        
        module env;
        for(decl d : decls)
          env.declare(d);

        auto bodyty = type_of(env.resolved(body));
        return types::function(std::move(argtypes), bodyty);
      },
      // case_of...
      [&](temporal auto tm, auto ...args) -> type {
        type argtypes[] = {type_of(args)...};
        for(auto ty : argtypes)
          if(!cast<types::boolean>(ty))
            return 
              types::error(tm,
                "temporal operators can be applied only to boolean terms"
              );

        return types::boolean();
      },
      [&](minus m, term arg) -> type {
        type ty = type_of(arg);
        
        if(
          bool(ty != types::integer()) && 
          bool(ty != types::real())
        )
          return types::error(
            m, "arithmetic operators only work on integers or reals"
          );
        
        return ty;
      },
      [&](arithmetic auto a, term left, term right) -> type {
        type type1 = type_of(left);
        type type2 = type_of(right);
        
        if(
          bool(type1 != types::integer()) && 
          bool(type1 != types::real())
        )
          return 
            types::error(a, 
              "left side of arithmetic operator must be integer or real"
            );
        
        if(
          bool(type2 != types::integer()) && 
          bool(type2 != types::real())
        )
          return 
            types::error(a, 
              "right side of arithmetic operator must be integer or real"
            );

        if(type1 != type2)
          return types::error(
            a, "arithmetic operators can only be applied to equal types"
          );
        
        return type1;
      },
      [&](relational auto r, term left, term right) -> type {
        type type1 = type_of(left);
        type type2 = type_of(right);

        if(
          bool(type1 != types::integer()) && 
          bool(type1 != types::real())
        )
          return 
            types::error(r, 
              "left side of relational operator must be integer or real"
            );
        
        if(
          bool(type2 != types::integer()) && 
          bool(type2 != types::real())
        )
          return 
            types::error(r, 
              "right side of relational operator must be integer or real"
            );

        if(type1 != type2)
          return types::error(
            r, "relational operators can only be applied to equal types"
          );
        
        return types::boolean();
      }
    );
  }

}