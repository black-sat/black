#include <black/logic>
#include <iostream>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>

using namespace black;
using namespace black::support;
using namespace black::logic;

using set_term = std::unordered_set<term, std::hash<term>, black::logic::term_equal_to<>>;

enum Surrogate { XS, WS, YS, ZS };

struct surrogate_t {
   term phi;

   size_t hash() const { return support::hash(phi); }
   bool operator==(surrogate_t const&) const = default;
};


/* std::unordered_map<Surrogate, set_term> surrogates(term t, module Gamma) {
    std::unordered_map<Surrogate, set_term> result;
    result[XS] = { };
    result[WS] = { };
    result[YS] = { };
    result[ZS] = { };

    int i = 0;

    for (term t1 : closure(t)) {
        match(t1)(
            [](tomorrow to){
                result[XS].insert(mod.define(
                    i, {}, types::boolean(),
                    to,
                    resolution::immediate
                ));
            }[](w_tomorrow wto){
                result[WS].insert(mod.define(
                    i, {}, types::boolean(),
                    wto,
                    resolution::immediate
                ));
            }[](yesterday y){
                result[YS].insert(mod.define(
                    i, {}, types::boolean(),
                    y,
                    resolution::immediate
                ));
            }[](w_yesterday z){
                result[ZS].insert(mod.define(
                    i, {}, types::boolean(),
                    z,
                    resolution::immediate
                ));
            }
        );
    }

    return result;
} */


int main() {
    module mod;

    object p = mod.declare("p", types::boolean());
    object q = mod.declare("q", types::boolean());

    term XPHI = X(p && q);
    variable xs_phi = "xs_phi";
    variable x = "x";

    object surr = mod.declare(decl(std::to_string(1), types::function({types::boolean()}, types::boolean())), resolution::delayed);

    std::string s = std::to_string((uintptr_t) XPHI.unique_id());
    std::cout << s << std::endl;
    return 0;
}