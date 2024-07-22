#include <black/logic>
#include <cassert>
#include <iostream>
#include <vector>

using namespace black;
using namespace black::support;
using namespace black::logic;

std::string what_term(term t) {
    return match(t) (
        [](conjunction) { return "conjunction"; },
        [](disjunction) { return "disjunction"; },
        [](negation) { return "negation"; },
        [](object) { return "object"; },
        [](otherwise) { return "something else"; }
    );
}

int depth(term t) {
    return match(t)(
        [](object) { return 1; },
        [](negation n) { return depth(n.argument()) + 1; },
        [](conjunction c) { return std::max(depth(c.arguments()[0]), depth(c.arguments()[1])) + 1; },
        [](disjunction d) { return std::max(depth(d.arguments()[0]), depth(d.arguments()[1])) + 1; }
    );
}

int main(){

    module mod;

    object p = mod.declare("p", types::boolean());
    object q = mod.declare("q", types::boolean());
    object r = mod.declare("r", types::boolean());

    variable x = "x";

    object succ = mod.define(
        "succ", {{x, types::integer()}}, types::integer(),
        x + 1
    );

    term c = (p && q) || r;

    std::cout << "Your term is a " << what_term(c) << "." << std::endl;

    std::cout << "Term depth: " << depth(c) << std::endl;

    return 0;
}


