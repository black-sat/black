//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#include <black/logic/formula.hpp>

#include <type_traits>

using namespace black;

//
// TODO: better testing of matching facility
//

template<typename T1, typename T2, typename R>
constexpr bool check = std::is_same_v<std::common_type_t<T1, T2>, R>;

static_assert(check<boolean, atom,         formula>);
static_assert(check<boolean, boolean,      formula>);
static_assert(check<boolean, negation,     formula>);
static_assert(check<boolean, tomorrow,     formula>);
static_assert(check<boolean, yesterday,    formula>);
static_assert(check<boolean, always,       formula>);
static_assert(check<boolean, eventually,   formula>);
static_assert(check<boolean, past,         formula>);
static_assert(check<boolean, historically, formula>);
static_assert(check<boolean, conjunction,  formula>);
static_assert(check<boolean, disjunction,  formula>);
static_assert(check<boolean, then,         formula>);
static_assert(check<boolean, iff,          formula>);
static_assert(check<boolean, until,        formula>);
static_assert(check<boolean, release,      formula>);
static_assert(check<boolean, since,        formula>);
static_assert(check<boolean, triggered,    formula>);

static_assert(check<atom, atom,         formula>);
static_assert(check<atom, boolean,      formula>);
static_assert(check<atom, negation,     formula>);
static_assert(check<atom, tomorrow,     formula>);
static_assert(check<atom, yesterday,    formula>);
static_assert(check<atom, always,       formula>);
static_assert(check<atom, eventually,   formula>);
static_assert(check<atom, past,         formula>);
static_assert(check<atom, historically, formula>);
static_assert(check<atom, conjunction,  formula>);
static_assert(check<atom, disjunction,  formula>);
static_assert(check<atom, then,         formula>);
static_assert(check<atom, iff,          formula>);
static_assert(check<atom, until,        formula>);
static_assert(check<atom, release,      formula>);
static_assert(check<atom, since,        formula>);
static_assert(check<atom, triggered,    formula>);

static_assert(check<tomorrow, boolean,      formula>);
static_assert(check<tomorrow, negation,     unary>);
static_assert(check<tomorrow, tomorrow,     tomorrow>);
static_assert(check<tomorrow, yesterday,    unary>);
static_assert(check<tomorrow, always,       unary>);
static_assert(check<tomorrow, eventually,   unary>);
static_assert(check<tomorrow, past,         unary>);
static_assert(check<tomorrow, historically, unary>);
static_assert(check<tomorrow, conjunction,  formula>);
static_assert(check<tomorrow, disjunction,  formula>);
static_assert(check<tomorrow, then,         formula>);
static_assert(check<tomorrow, iff,          formula>);
static_assert(check<tomorrow, until,        formula>);
static_assert(check<tomorrow, release,      formula>);
static_assert(check<tomorrow, since,        formula>);
static_assert(check<tomorrow, triggered,    formula>);

static_assert(check<yesterday, atom,         formula>);
static_assert(check<yesterday, boolean,      formula>);
static_assert(check<yesterday, negation,     unary>);
static_assert(check<yesterday, tomorrow,     unary>);
static_assert(check<yesterday, yesterday,    yesterday>);
static_assert(check<yesterday, always,       unary>);
static_assert(check<yesterday, eventually,   unary>);
static_assert(check<yesterday, past,         unary>);
static_assert(check<yesterday, historically, unary>);
static_assert(check<yesterday, conjunction,  formula>);
static_assert(check<yesterday, disjunction,  formula>);
static_assert(check<yesterday, then,         formula>);
static_assert(check<yesterday, iff,          formula>);
static_assert(check<yesterday, until,        formula>);
static_assert(check<yesterday, release,      formula>);
static_assert(check<yesterday, since,        formula>);
static_assert(check<yesterday, triggered,    formula>);

static_assert(check<always, atom,         formula>);
static_assert(check<always, boolean,      formula>);
static_assert(check<always, negation,     unary>);
static_assert(check<always, tomorrow,     unary>);
static_assert(check<always, yesterday,    unary>);
static_assert(check<always, always,       always>);
static_assert(check<always, eventually,   unary>);
static_assert(check<always, past,         unary>);
static_assert(check<always, historically, unary>);
static_assert(check<always, conjunction,  formula>);
static_assert(check<always, disjunction,  formula>);
static_assert(check<always, then,         formula>);
static_assert(check<always, iff,          formula>);
static_assert(check<always, until,        formula>);
static_assert(check<always, release,      formula>);
static_assert(check<always, since,        formula>);
static_assert(check<always, triggered,    formula>);

static_assert(check<eventually, atom,         formula>);
static_assert(check<eventually, boolean,      formula>);
static_assert(check<eventually, negation,     unary>);
static_assert(check<eventually, tomorrow,     unary>);
static_assert(check<eventually, yesterday,    unary>);
static_assert(check<eventually, always,       unary>);
static_assert(check<eventually, eventually,   eventually>);
static_assert(check<eventually, past,         unary>);
static_assert(check<eventually, historically, unary>);
static_assert(check<eventually, conjunction,  formula>);
static_assert(check<eventually, disjunction,  formula>);
static_assert(check<eventually, then,         formula>);
static_assert(check<eventually, iff,          formula>);
static_assert(check<eventually, until,        formula>);
static_assert(check<eventually, release,      formula>);
static_assert(check<eventually, since,        formula>);
static_assert(check<eventually, triggered,    formula>);

static_assert(check<past, atom,         formula>);
static_assert(check<past, boolean,      formula>);
static_assert(check<past, negation,     unary>);
static_assert(check<past, tomorrow,     unary>);
static_assert(check<past, yesterday,    unary>);
static_assert(check<past, always,       unary>);
static_assert(check<past, eventually,   unary>);
static_assert(check<past, past,         past>);
static_assert(check<past, historically, unary>);
static_assert(check<past, conjunction,  formula>);
static_assert(check<past, disjunction,  formula>);
static_assert(check<past, then,         formula>);
static_assert(check<past, iff,          formula>);
static_assert(check<past, until,        formula>);
static_assert(check<past, release,      formula>);
static_assert(check<past, since,        formula>);
static_assert(check<past, triggered,    formula>);

static_assert(check<historically, atom,         formula>);
static_assert(check<historically, boolean,      formula>);
static_assert(check<historically, negation,     unary>);
static_assert(check<historically, tomorrow,     unary>);
static_assert(check<historically, yesterday,    unary>);
static_assert(check<historically, always,       unary>);
static_assert(check<historically, eventually,   unary>);
static_assert(check<historically, past,         unary>);
static_assert(check<historically, historically, historically>);
static_assert(check<historically, conjunction,  formula>);
static_assert(check<historically, disjunction,  formula>);
static_assert(check<historically, then,         formula>);
static_assert(check<historically, iff,          formula>);
static_assert(check<historically, until,        formula>);
static_assert(check<historically, release,      formula>);
static_assert(check<historically, since,        formula>);
static_assert(check<historically, triggered,    formula>);

static_assert(check<conjunction, atom,         formula>);
static_assert(check<conjunction, boolean,      formula>);
static_assert(check<conjunction, negation,     formula>);
static_assert(check<conjunction, tomorrow,     formula>);
static_assert(check<conjunction, yesterday,    formula>);
static_assert(check<conjunction, always,       formula>);
static_assert(check<conjunction, eventually,   formula>);
static_assert(check<conjunction, past,         formula>);
static_assert(check<conjunction, historically, formula>);
static_assert(check<conjunction, conjunction,  conjunction>);
static_assert(check<conjunction, disjunction,  binary>);
static_assert(check<conjunction, then,         binary>);
static_assert(check<conjunction, iff,          binary>);
static_assert(check<conjunction, until,        binary>);
static_assert(check<conjunction, release,      binary>);
static_assert(check<conjunction, since,        binary>);
static_assert(check<conjunction, triggered,    binary>);

static_assert(check<disjunction, atom,         formula>);
static_assert(check<disjunction, boolean,      formula>);
static_assert(check<disjunction, negation,     formula>);
static_assert(check<disjunction, tomorrow,     formula>);
static_assert(check<disjunction, yesterday,    formula>);
static_assert(check<disjunction, always,       formula>);
static_assert(check<disjunction, eventually,   formula>);
static_assert(check<disjunction, past,         formula>);
static_assert(check<disjunction, historically, formula>);
static_assert(check<disjunction, conjunction,  binary>);
static_assert(check<disjunction, disjunction,  disjunction>);
static_assert(check<disjunction, then,         binary>);
static_assert(check<disjunction, iff,          binary>);
static_assert(check<disjunction, until,        binary>);
static_assert(check<disjunction, release,      binary>);
static_assert(check<disjunction, since,        binary>);
static_assert(check<disjunction, triggered,    binary>);

static_assert(check<then, atom,         formula>);
static_assert(check<then, boolean,      formula>);
static_assert(check<then, negation,     formula>);
static_assert(check<then, tomorrow,     formula>);
static_assert(check<then, yesterday,    formula>);
static_assert(check<then, always,       formula>);
static_assert(check<then, eventually,   formula>);
static_assert(check<then, past,         formula>);
static_assert(check<then, historically, formula>);
static_assert(check<then, conjunction,  binary>);
static_assert(check<then, disjunction,  binary>);
static_assert(check<then, then,         then>);
static_assert(check<then, iff,          binary>);
static_assert(check<then, until,        binary>);
static_assert(check<then, release,      binary>);
static_assert(check<then, since,        binary>);
static_assert(check<then, triggered,    binary>);

static_assert(check<iff, atom,         formula>);
static_assert(check<iff, boolean,      formula>);
static_assert(check<iff, negation,     formula>);
static_assert(check<iff, tomorrow,     formula>);
static_assert(check<iff, yesterday,    formula>);
static_assert(check<iff, always,       formula>);
static_assert(check<iff, eventually,   formula>);
static_assert(check<iff, past,         formula>);
static_assert(check<iff, historically, formula>);
static_assert(check<iff, conjunction,  binary>);
static_assert(check<iff, disjunction,  binary>);
static_assert(check<iff, then,         binary>);
static_assert(check<iff, iff,          iff>);
static_assert(check<iff, until,        binary>);
static_assert(check<iff, release,      binary>);
static_assert(check<iff, since,        binary>);
static_assert(check<iff, triggered,    binary>);

static_assert(check<until, atom,         formula>);
static_assert(check<until, boolean,      formula>);
static_assert(check<until, negation,     formula>);
static_assert(check<until, tomorrow,     formula>);
static_assert(check<until, yesterday,    formula>);
static_assert(check<until, always,       formula>);
static_assert(check<until, eventually,   formula>);
static_assert(check<until, past,         formula>);
static_assert(check<until, historically, formula>);
static_assert(check<until, conjunction,  binary>);
static_assert(check<until, disjunction,  binary>);
static_assert(check<until, then,         binary>);
static_assert(check<until, iff,          binary>);
static_assert(check<until, until,        until>);
static_assert(check<until, release,      binary>);
static_assert(check<until, since,        binary>);
static_assert(check<until, triggered,    binary>);

static_assert(check<release, atom,         formula>);
static_assert(check<release, boolean,      formula>);
static_assert(check<release, negation,     formula>);
static_assert(check<release, tomorrow,     formula>);
static_assert(check<release, yesterday,    formula>);
static_assert(check<release, always,       formula>);
static_assert(check<release, eventually,   formula>);
static_assert(check<release, past,         formula>);
static_assert(check<release, historically, formula>);
static_assert(check<release, conjunction,  binary>);
static_assert(check<release, disjunction,  binary>);
static_assert(check<release, then,         binary>);
static_assert(check<release, iff,          binary>);
static_assert(check<release, until,        binary>);
static_assert(check<release, release,      release>);
static_assert(check<release, since,        binary>);
static_assert(check<release, triggered,    binary>);

static_assert(check<since, atom,         formula>);
static_assert(check<since, boolean,      formula>);
static_assert(check<since, negation,     formula>);
static_assert(check<since, tomorrow,     formula>);
static_assert(check<since, yesterday,    formula>);
static_assert(check<since, always,       formula>);
static_assert(check<since, eventually,   formula>);
static_assert(check<since, past,         formula>);
static_assert(check<since, historically, formula>);
static_assert(check<since, conjunction,  binary>);
static_assert(check<since, disjunction,  binary>);
static_assert(check<since, then,         binary>);
static_assert(check<since, iff,          binary>);
static_assert(check<since, until,        binary>);
static_assert(check<since, release,      binary>);
static_assert(check<since, since,        since>);
static_assert(check<since, triggered,    binary>);

static_assert(check<triggered, atom,         formula>);
static_assert(check<triggered, boolean,      formula>);
static_assert(check<triggered, negation,     formula>);
static_assert(check<triggered, tomorrow,     formula>);
static_assert(check<triggered, yesterday,    formula>);
static_assert(check<triggered, always,       formula>);
static_assert(check<triggered, eventually,   formula>);
static_assert(check<triggered, past,         formula>);
static_assert(check<triggered, historically, formula>);
static_assert(check<triggered, conjunction,  binary>);
static_assert(check<triggered, disjunction,  binary>);
static_assert(check<triggered, then,         binary>);
static_assert(check<triggered, iff,          binary>);
static_assert(check<triggered, until,        binary>);
static_assert(check<triggered, release,      binary>);
static_assert(check<triggered, since,        binary>);
static_assert(check<triggered, triggered,    triggered>);
