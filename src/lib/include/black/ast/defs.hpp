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

#ifndef declare_ast
  #define declare_ast(NS, AST)
#endif

#ifndef declare_ast_node
  #define declare_ast_node(NS, AST, Node)
#endif

#ifndef declare_field
  #define declare_field(NS, AST, Node, Field, Type)
#endif

#ifndef end_ast_node
  #define end_ast_node(NS, AST, Node)
#endif

#ifndef end_ast
  #define end_ast(NS, AST)
#endif

declare_ast(logic, term)

  declare_ast_node(logic, term, integer)
    declare_field(logic, term, integer, value, int64_t)
  end_ast_node(logic, term, integer)

  declare_ast_node(logic, term, symbol)
    declare_field(logic, term, symbol, name, logic::label)
  end_ast_node(logic, term, symbol)

  declare_ast_node(logic, term, boolean)
    declare_field(logic, term, boolean, value, bool)
  end_ast_node(logic, term, boolean)

  declare_ast_node(logic, term, conjunction)
    declare_field(logic, term, conjunction, operands, std::vector<logic::term>)
  end_ast_node(logic, term, conjunction)

end_ast(logic, term)


#undef declare_ast
#undef declare_ast_node
#undef declare_field
#undef end_ast_node
#undef end_ast