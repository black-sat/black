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

enum class step {
    import,
    adopt,
    adopt_rec,
    require,
    push,
    pop
};

struct debug_t {
    module &mod;
    std::vector<step> steps;

    debug_t(module &mod) : mod{mod} { }

    void import(module m) { 
        steps.push_back(step::import);
        return mod.import(std::move(m));
    }
    void adopt(std::vector<object> const& objs, scope s) { 
        if(s == scope::linear)
            steps.push_back(step::adopt);
        else
            steps.push_back(step::adopt_rec);
        return mod.adopt(objs, s);
    }
    void require(term r) { 
        steps.push_back(step::require);
        return mod.require(r);
    }
    void push() { 
        steps.push_back(step::push);
        mod.push();
    }
    void pop(size_t n) { 
        steps.push_back(step::pop);
        mod.pop(n);
    }
    
};

TEST_CASE("modules") {

    SECTION("linear definitions") {
        module ours;

        object x = ours.declare({"x", integer_type()});
        object y = ours.declare({"y", integer_type()});

        ours.require(x < y);
        ours.push();

        SECTION("theirs prefix of ours") {
            module theirs = ours;

            ours.push();
            ours.require(y <= x);

            ours.push();
            ours.require(y == 42);

            debug_t debug{theirs};
            ours.replay(theirs, debug);

            std::vector<step> expected = {
                step::push, step::require, step::push, step::require
            };
            
            REQUIRE(ours == theirs);
            REQUIRE(debug.steps == expected);
        }

        SECTION("ours prefix of theirs") {
            module theirs = ours;

            theirs.push();
            theirs.require(y <= x);

            debug_t debug{theirs};
            ours.replay(theirs, debug);

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

            debug_t debug{theirs};
            ours.replay(theirs, debug);

            std::vector<step> expected = {
                step::pop, step::pop, step::push, 
                step::require, step::push, step::require
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

            debug_t debug{theirs};
            ours.replay(theirs, debug);

            std::vector<step> expected = {
                step::pop, step::require, step::push, step::require
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
            rec.define({f, {{x, integer_type()}}, g(x)}, resolution::delayed);
        object gobj = 
            rec.define({g, {{x, integer_type()}}, f(x)}, resolution::delayed);

        rec.resolve(scope::recursive);

        module other;

        debug_t debug{other};
        rec.replay(other, debug);

        std::vector<step> expected = {
            step::adopt_rec
        };
        REQUIRE(rec == other);
        REQUIRE(debug.steps == expected);

    }

}