//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2026 Nicola Gigante
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
//

#ifndef BLACK_LOGIC_NNF_HPP
#define BLACK_LOGIC_NNF_HPP

#include <black/logic/logic.hpp>

namespace black_internal::logic {
    // Duals for temporal operators used in to_nnf()
    static unary::type dual(unary::type t) {
        switch(t) {
        case unary::type::negation:
            return unary::type::negation;
        case unary::type::tomorrow:
            return unary::type::w_tomorrow;
        case unary::type::w_tomorrow:
            return unary::type::tomorrow;
        case unary::type::yesterday:
            return unary::type::w_yesterday;
        case unary::type::w_yesterday:
            return unary::type::yesterday;
        case unary::type::always:
            return unary::type::eventually;
        case unary::type::eventually:
            return unary::type::always;
        case unary::type::once:
            return unary::type::historically;
        case unary::type::historically:
            return unary::type::once;
        }
        black_unreachable();
    }

    static binary::type dual(binary::type t)
    {
        switch(t) {
        case binary::type::until:
            return binary::type::release;
        case binary::type::release:
            return binary::type::until;
        case binary::type::w_until:
            return binary::type::s_release;
        case binary::type::s_release:
            return binary::type::w_until;
        case binary::type::since:
            return binary::type::triggered;
        case binary::type::triggered:
            return binary::type::since;
        case binary::type::conjunction:
        case binary::type::disjunction:
        case binary::type::implication:
        case binary::type::iff:
            black_unreachable(); // LCOV_EXCL_LINE
        }
        black_unreachable();
    }

    // Turns a formula into negated normal form
    formula to_nnf(formula f);

    // Turns a formula into negated normal form, using the provided `cache` to memoize shared
    // subformulas. The `cache` parameter must be of a type compatible with
    // `std::unordered_map<formula, formula>`.
    formula to_nnf_cached(formula f, auto& cache) {
        if(auto it = cache.find(f); it != cache.end())
            return it->second;

        formula nnf = f.match( // LCOV_EXCL_LINE
            [](boolean b) { return b; },
            [](proposition p) { return p; },
            [](atom a) { return a; },
            [](equality e) { return e; },
            [](comparison c) { return c; },
            [&](quantifier q) {
                return quantifier(
                    q.node_type(), q.variables(), to_nnf_cached(q.matrix(), cache)
                );
            },
            // Push the negation down to literals
            [&](negation n) {
                return n.argument().match(
                    [&](boolean, bool value) { return f.sigma()->boolean(!value); },
                    [&](proposition) { return n; },
                    [&](atom) { return n; },
                    [&](equality) { return n; },
                    [&](comparison) { return n; },
                    [&](quantifier q) {
                        quantifier::type dual = quantifier::type::exists;
                        if(q.node_type() == quantifier::type::exists)
                            dual = quantifier::type::forall;

                        return quantifier(dual, q.variables(), to_nnf_cached(!q.matrix(), cache));
                    },
                    [&](negation, auto op) {
                        // special case for double negation
                        return to_nnf(op);
                    },
                    [&](unary u) {
                        return unary(dual(u.node_type()), to_nnf_cached(!u.argument(), cache));
                    },
                    [&](implication, auto left, auto right) {
                        return to_nnf_cached(left, cache) && to_nnf_cached(!right, cache);
                    },
                    [&](iff, auto left, auto right) {
                        return
                            to_nnf_cached(!implies(left, right), cache) ||
                            to_nnf_cached(!implies(right, left), cache);
                    },
                    [&](conjunction c) {
                        return big_or(*f.sigma(), operands(c), [&](auto op) {
                            return to_nnf_cached(!op, cache);
                        });
                    },
                    [&](disjunction c) {
                        return big_and(*f.sigma(), operands(c), [&](auto op) {
                            return to_nnf_cached(!op, cache);
                        });
                    },
                    [&](binary b, auto left, auto right) {
                        return binary(
                            dual(b.node_type()),
                            to_nnf_cached(!left, cache), to_nnf_cached(!right, cache)
                        );
                    }
                );
            },
            // other cases: just recurse down the formula
            [&](unary u) {
                return unary(u.node_type(), to_nnf_cached(u.argument(), cache));
            },
            [&](implication, auto left, auto right) {
                return to_nnf_cached(!left, cache) || to_nnf_cached(right, cache);
            },
            [&](iff, auto left, auto right) {
                return
                    to_nnf_cached(implies(left, right), cache) &&
                    to_nnf_cached(implies(right, left), cache);
            },
            [&](conjunction c) {
                return big_and(*f.sigma(), operands(c), [&](auto op) {
                    return to_nnf_cached(op, cache);
                });
            },
            [&](disjunction c) {
                return big_or(*f.sigma(), operands(c), [&](auto op) {
                    return to_nnf_cached(op, cache);
                });
            },
            [&](binary b) {
                return binary(
                    b.node_type(), to_nnf_cached(b.left(), cache), to_nnf_cached(b.right(), cache)
                );
            }
        );

        cache.insert({f, nnf});
        return nnf;
    }
}

namespace black::logic {
    using black_internal::logic::to_nnf;
}

#endif //BLACK_LOGIC_NNF_HPP
