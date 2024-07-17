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

#include <catch.hpp>

#include <black/support>
#include <black/logic>

using namespace black::support;
using namespace black::logic;
using namespace black::pipes;

enum class step {
    import,
    adopt,
    adopt_rec,
    state,
    push,
    pop
};

struct debug_t : consumer {
    module *mod;
    std::vector<step> steps;

    debug_t(module *mod) : mod{mod} { }

    virtual void import(module m) override { 
        steps.push_back(step::import);
        return mod->import(std::move(m));
    }
    virtual void adopt(std::shared_ptr<root const> r) override { 
        if(r->mode == recursion::forbidden)
            steps.push_back(step::adopt);
        else
            steps.push_back(step::adopt_rec);
        return mod->adopt(r);
    }
    virtual void state(term t, statement s) override { 
        steps.push_back(step::state);
        return mod->state(t, s);
    }
    virtual void push() override { 
        steps.push_back(step::push);
        mod->push();
    }
    virtual void pop(size_t n) override { 
        steps.push_back(step::pop);
        mod->pop(n);
    }
    
};

TEST_CASE("modules") {

    SECTION("linear definitions") {
        module ours;

        object x = ours.declare("x", types::integer());
        object y = ours.declare("y", types::integer());

        ours.require(x < y);
        ours.push();

        SECTION("theirs prefix of ours") {
            module theirs = ours;

            ours.push();
            REQUIRE(ours != theirs);

            ours.require(y <= x);

            ours.push();
            ours.require(y == 42);

            debug_t debug{&theirs};
            ours.replay(theirs, &debug);

            std::vector<step> expected = {
                step::push, step::state, step::push, step::state
            };
            
            REQUIRE(ours == theirs);
            REQUIRE(debug.steps == expected);
        }

        SECTION("ours prefix of theirs") {
            module theirs = ours;

            theirs.push();
            theirs.require(y <= x);

            debug_t debug{&theirs};
            ours.replay(theirs, &debug);

            std::vector<step> expected = {step::pop};
            
            REQUIRE(ours == theirs);
            REQUIRE(debug.steps == expected);
        }

        SECTION("diverging with different pivot frame") {
            module theirs = ours;

            ours.require(y >= x);
            theirs.require(y <= x);

            ours.push();
            theirs.push();
            ours.require(y == 2);
            theirs.require(y == 3);

            debug_t debug{&theirs};
            ours.replay(theirs, &debug);

            std::vector<step> expected = {
                step::pop, step::pop, step::push, 
                step::state, step::push, step::state
            };
            
            REQUIRE(ours == theirs);
            REQUIRE(debug.steps == expected);
        }

        SECTION("diverging with included pivot frame") {
            module theirs = ours;

            ours.require(y >= x);
            ours.require(y <= x);
            theirs.require(y >= x);

            ours.push();
            theirs.push();
            ours.require(y == 2);
            theirs.require(y == 3);

            debug_t debug{&theirs};
            ours.replay(theirs, &debug);

            std::vector<step> expected = {
                step::pop, step::state, step::push, step::state
            };
            
            REQUIRE(ours == theirs);
            REQUIRE(debug.steps == expected);
        }
    }

    SECTION("Recursive definitions") {

        module rec;

        variable x = "x";
        variable f = "f";
        variable g = "g";

        object fobj = 
            rec.define({f, {{x, types::integer()}}, g(x)}, resolution::delayed);
        object gobj = 
            rec.define({g, {{x, types::integer()}}, f(x)}, resolution::delayed);

        rec.resolve(recursion::allowed);

        module other;

        debug_t debug{&other};
        rec.replay(other, &debug);

        std::vector<step> expected = {
            step::adopt_rec
        };
        REQUIRE(rec == other);
        REQUIRE(debug.steps == expected);

    }

}