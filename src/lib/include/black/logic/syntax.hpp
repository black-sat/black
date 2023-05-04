//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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

#ifndef BLACK_LOGIC_NEW_HPP
#define BLACK_LOGIC_NEW_HPP

namespace black::logic::internal {

  // 
  // Requirements:
  // * compile-time definition of the syntax of terms
  //
  // * corresponding runtime representation 
  //
  // * rule-based system
  //
  // * a fragment is a *disjunction* of rules among the following:
  //   - set: a list of admitted types for the node and its children
  //   - head: a list of admitted types for the node
  //     children: a fragment for the children
  //
  // * available operations:
  //   - union of fragments -> the disjunction of fragments is a fragment
  //   - intersection of fragments -> see tablet
  //
  // * example: literal
  //   - set: proposition
  //   - head: negation
  //     children: 
  //     - set: proposition
  //
  // * example: CNF
  //   - head: conjunction
  //     children:
  //     - head: disjunction
  //       children: literal
  //
  // * example: NNF
  //   - set: everything except negation
  //   - head: negation
  //     children:
  //     - set: proposition
  //
  // * example: LTL
  //   - set: every boolean and future temporal operator
  //
  // * example: G(pLTL)
  //   - head: globally
  //     children:
  //     - set: every boolean and past temporal operator
  //
  // * example: SafetyLTL
  //   - intersection of:
  //     - LTL
  //     - NNF
  //     - set: boolean + X, G, R
  // 
  // * needed queries:
  //   - satisfaction of a fragment by a specific node at runtime
  //     - just a recursive descent
  //   - inclusion of a fragment by another fragment at compile-time
  //     - see tablet
  //   - given a fragment and a specific node type, get the fragment of the 
  //     children
  //
  //  

}

#endif // BLACK_LOGIC_NEW_HPP
