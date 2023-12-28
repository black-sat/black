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

#ifndef declare_term_type
  #define declare_term_type(Term)
#endif

#ifndef declare_field
  #define declare_field(Term, Field, Type)
#endif

#ifndef declare_fields
  #define declare_fields(Term, Field, Type)
#endif

#ifndef end_term_type
  #define end_term_type(Term)
#endif

declare_term_type(integer)
  declare_field(integer, value, int64_t)
end_term_type(integer)

declare_term_type(symbol)
  declare_field(symbol, name, label)
end_term_type(symbol)

declare_term_type(boolean)
  declare_field(boolean, value, bool)
end_term_type(boolean)

declare_term_type(conjunction)
  declare_fields(conjunction, operands, term)
end_term_type(conjunction)


#undef declare_term_type
#undef declare_field
#undef declare_fields
#undef end_term_type