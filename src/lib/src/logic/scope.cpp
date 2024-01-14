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
#include <black/logic>

namespace black::logic {

  type_result<type> scope::type_of(term t) const {
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
      [&](symbol s)      -> type_result<type> { 
        if(auto type = type_of(s); type)
          return type;
        return type_error("Use of undeclared symbol");
      },
      [&](atom, term head, auto args) -> type_result<type> {
        auto fty = cast<function_type>(type_of(head));
        if(!fty)
          return type_error("Calling a non-function");
          
        auto sorts = type_of(args);
        if(!sorts)
          return sorts.error();

        if(*sorts != fty->parameters())
          return type_error("Type mismatch in function call");

        return fty->range();
      }
    );
  }

  type_result<std::vector<type>> 
  scope::type_of(std::vector<term> const& vec) const {
    std::vector<type> types;
    for(auto t : vec) {
      auto ty = type_of(t);
      if(!ty)
        return ty.error();
      types.push_back(*ty);
    }
    return types;
  }

}

