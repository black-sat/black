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

#include <black/support>
#include <black/io>
#include <black/logic>

namespace black::logic {

  scope::result<term> scope::type_of(term t) const {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](type_type)     { return sigma->type_type(); },
      [&](integer_type)  { return sigma->type_type(); },
      [&](real_type)     { return sigma->type_type(); },
      [&](boolean_type)  { return sigma->type_type(); },
      [&](integer)       { return sigma->integer_type(); },
      [&](real)          { return sigma->real_type(); },
      [&](boolean)       { return sigma->boolean_type(); },
      [&](equal)         { return sigma->boolean_type(); },
      [&](distinct)      { return sigma->boolean_type(); },
      [&](type_cast c)   { return c.target(); },
      [&](function_type, std::vector<term> const& params, term range) 
        -> result<term>
      { 
        for(term p : params) {
          result<bool> ist = is_type(p);
          if(!ist)
            return ist.error();
          if(!*ist)
            return type_error("type of function parameter is not a type");
        }
        result<bool> ist = is_type(range);
        if(!ist)
          return ist.error();
        if(!*ist)
          return type_error("function range is not a type");

        return sigma->type_type(); 
      },
      [&](symbol s) -> result<term> { 
        if(auto type = type_of(s); type)
          return *type;
        return type_error("use of undeclared symbol");
      },
      [&](atom, term head, auto args) -> result<term> {
        result<term> rfty = type_of(head);
        if(!rfty)
          return rfty;

        auto fty = cast<function_type>(rfty);
        if(!fty)
          return type_error("calling a non-function");
          
        auto types = type_of(args);
        if(!types)
          return types.error();

        if(*types != fty->parameters())
          return type_error("type mismatch in function call");

        return fty->range();
      },
      [&](any_of<negation, implication> auto, auto ...args) -> result<term> {
        result<term> argtypes[] = {type_of(args)...};
        for(auto ty : argtypes) {
          if(!ty)
            return ty;
          if(!cast<boolean_type>(ty))
            return 
              type_error("connectives can be applied only to boolean terms");
        }

        return sigma->boolean_type();
      },
      [&](any_of<conjunction, disjunction> auto, std::vector<term> const& args) 
        -> result<term> 
      {
        for(term arg : args) { 
          auto argty = type_of(arg);
          if(!argty)
            return argty;
          
          if(!cast<boolean_type>(argty))
            return 
              type_error("connectives can be applied only to boolean terms");
        }

        return sigma->boolean_type();
      },
      [&](ite, term guard, term iftrue, term iffalse) -> result<term> {
        auto guardty = type_of(guard);
        auto truety = type_of(iftrue);
        auto falsety = type_of(iffalse);
        
        if(!guardty)
          return guardty;
        if(!truety)
          return truety;
        if(!falsety)
          return falsety;

        if(!cast<boolean_type>(guardty))
          return type_error("the guard of an `ite` expression must be boolean");
        if(*truety != *falsety)
          return 
            type_error(
              "the two cases of an `ite` expression must have the same type"
            );

        return *truety;
      },
      [&](lambda, std::vector<decl> const& decls, term body) -> result<term> {
        std::vector<term> argtypes;
        for(decl d : decls)
          argtypes.push_back(d.type);
        
        auto bodyty = type_of(body);
        if(!bodyty)
          return bodyty;
        return function_type(std::move(argtypes), *bodyty);
      },
      // case_of...
      [&](temporal auto, auto ...args) -> result<term> {
        result<term> argtypes[] = {type_of(args)...};
        for(auto ty : argtypes) {
          if(!ty)
            return ty;
          if(!cast<boolean_type>(ty))
            return 
              type_error(
                "temporal operators can be applied only to boolean terms"
              );
        }

        return sigma->boolean_type();
      }
    );
  }

  scope::result<bool> scope::is_type(term t) const {
    result<term> type = type_of(t);
    if(!type)
      return type.error();
    
    return bool(cast<type_type>(type));
  }

  scope::result<std::vector<term>> 
  scope::type_of(std::vector<term> const& vec) const {
    std::vector<term> types;
    for(auto t : vec) {
      auto ty = type_of(t);
      if(!ty)
        return ty.error();
      types.push_back(*ty);
    }
    return types;
  }

}

