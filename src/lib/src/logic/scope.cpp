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

  type_result<term> scope::type_of(term t) const {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](type_type)     { return sigma->type_type(); },
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
      [&](symbol s)      -> type_result<term> { 
        if(auto type = type_of(s); type)
          return type;
        return 
          type_error(
            "Use of undeclared symbol"
          );
      },
      [&](atom, term head, auto args) -> type_result<term> {
        auto fty = cast<function_type>(type_of(head));
        if(!fty)
          return type_error("Calling a non-function");
          
        auto types = type_of(args);
        if(!types)
          return types.error();

        if(*types != fty->parameters())
          return type_error("Type mismatch in function call");

        return fty->range();
      },
      [&](negation, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;

        if(!cast<boolean_type>(argty))
          return type_error("Negation can be applied only to boolean terms");

        return sigma->boolean_type();
      },
      [&](conjunction, std::vector<term> const& arguments) -> type_result<term> 
      {
        for(term arg : arguments) { 
          auto argty = type_of(arg);
          if(!argty)
            return argty;
          
          if(!cast<boolean_type>(argty))
            return 
              type_error("Conjunction can be applied only to boolean terms");
        }

        return sigma->boolean_type();          
      },
      [&](disjunction, std::vector<term> const& arguments) -> type_result<term> 
      {
        for(term arg : arguments) { 
          auto argty = type_of(arg);
          if(!argty)
            return argty;
          
          if(!cast<boolean_type>(argty))
            return 
              type_error("Disjunction can be applied only to boolean terms");
        }

        return sigma->boolean_type();          
      },
      [&](implication, std::vector<term> const& arguments) -> type_result<term> 
      {
        for(term arg : arguments) { 
          auto argty = type_of(arg);
          if(!argty)
            return argty;
          
          if(!cast<boolean_type>(argty))
            return 
              type_error("Implication can be applied only to boolean terms");
        }

        return sigma->boolean_type();          
      },
      [&](ite, term guard, term iftrue, term iffalse) -> type_result<term> {
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
          return type_error("The guard of an `ite` expression must be boolean");
        if(*truety != *falsety)
          return 
            type_error(
              "The two cases of an `ite` expression must have the same type"
            );

        return *truety;
      },
      [&](lambda, std::vector<decl> const& decls, term body)  
        -> type_result<term> 
      {
        std::vector<term> argtypes;
        for(decl d : decls)
          argtypes.push_back(d.type);
        
        auto bodyty = type_of(body);
        if(!bodyty)
          return bodyty;
        return function_type(std::move(argtypes), *bodyty);
      },
      // case_of...
      [&](tomorrow, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `tomorrow` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](w_tomorrow, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `weak tomorrow` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](yesterday, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `yesterday` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](w_yesterday, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `weak yesterday` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](eventually, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `eventually` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](always, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `always` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](once, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `once` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](historically, term argument) -> type_result<term> {
        auto argty = type_of(argument);
        if(!argty)
          return argty;
        if(!cast<boolean_type>(argty))
          return 
            type_error(
              "The `historically` temporal operators can only be applied to "
              "booleans"
            );
        return sigma->boolean_type();
      },
      [&](until, term left, term right) -> type_result<term> {
        auto leftty = type_of(left);
        auto rightty = type_of(right);
        
        if(!leftty)
          return leftty;
        if(!rightty)
          return rightty;
        
        if(!cast<boolean_type>(leftty) || !cast<boolean_type>(rightty))
          return type_error(
            "The `until` temporal operators can only be applied to "
            "booleans"
          );
        
        return sigma->boolean_type();
      },
      [&](release, term left, term right) -> type_result<term> {
        auto leftty = type_of(left);
        auto rightty = type_of(right);
        
        if(!leftty)
          return leftty;
        if(!rightty)
          return rightty;
        
        if(!cast<boolean_type>(leftty) || !cast<boolean_type>(rightty))
          return type_error(
            "The `release` temporal operators can only be applied to "
            "booleans"
          );
        
        return sigma->boolean_type();
      },
      [&](since, term left, term right) -> type_result<term> {
        auto leftty = type_of(left);
        auto rightty = type_of(right);
        
        if(!leftty)
          return leftty;
        if(!rightty)
          return rightty;
        
        if(!cast<boolean_type>(leftty) || !cast<boolean_type>(rightty))
          return type_error(
            "The `since` temporal operators can only be applied to "
            "booleans"
          );
        
        return sigma->boolean_type();
      },
      [&](triggered, term left, term right) -> type_result<term> {
        auto leftty = type_of(left);
        auto rightty = type_of(right);
        
        if(!leftty)
          return leftty;
        if(!rightty)
          return rightty;
        
        if(!cast<boolean_type>(leftty) || !cast<boolean_type>(rightty))
          return type_error(
            "The `triggered` temporal operators can only be applied to "
            "booleans"
          );
        
        return sigma->boolean_type();
      }
    );
  }

  type_result<std::vector<term>> 
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

