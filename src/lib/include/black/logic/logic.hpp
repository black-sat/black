//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_LOGIC_FORMULA_HPP
#define BLACK_LOGIC_FORMULA_HPP

//
// The files included here define BLACK's logic API: handling of logic formulas
// and everything around formulas.
//
// WARNING: the following files are thoroughly commented, but please *do not*
// read them to understand how to *use* BLACK's logic API. Refer to the
// documentation instead for that (and ping me if the documentation does not yet
// exists when you are reading this comment). After that, come here to
// understand how the API works under the hood.
//
// Read the comments of the files in this order:
// - hierarchy.hpp (which is not included from here)
// - core.hpp
// - generation.hpp
// - interface.hpp (interface-fwd.hpp are just needed forward declarations)
// - fragments.hpp
//
#include <black/internal/logic/core.hpp>
#include <black/internal/logic/interface-fwd.hpp>
#include <black/internal/logic/generation.hpp>
#include <black/internal/logic/interface.hpp>

//
// All things semantics
//
#include <black/internal/logic/semantics.hpp>

#endif // BLACK_LOGIC_FORMULA_HPP
