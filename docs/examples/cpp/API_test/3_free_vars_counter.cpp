#include <black/logic>
#include <iostream>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>
#include <unordered_set>
#include "utils.hpp"

using namespace black;
using namespace black::support;
using namespace black::logic;

using set_var = std::unordered_set<variable, std::hash<variable>, black::logic::term_equal_to<>>;

set_var set_union(set_var set1, set_var set2);
set_var quantify(std::vector<decl> q_vars, set_var vars);
set_var free_vars(term t);

set_var set_union(set_var set1, set_var set2) {
    set1.insert (set2.begin(), set2.end());
    return set1;
}

/*
    Removes any quantified variable (q_vars) from the set of free variables vars.
*/
set_var quantify(std::vector<decl> q_vars, set_var vars) {
    for (decl d : q_vars) {
        vars.erase(vars.find(d.name));
    }
    return vars;
}

set_var free_vars(term t) { 
    return match(t)(
        /*
            Base cases. In case of an object, distinguish between declared and defined objects.
        */
        [](variable var) -> set_var { return { var }; },
        [](object)   -> set_var { return { }; },

        /*
            Quantifiers.
        */
        [](exists, std::vector<decl> decls, term body) -> set_var { return quantify(decls, free_vars(body)); },
        [](forall, std::vector<decl> decls, term body) -> set_var { return quantify(decls, free_vars(body)); },
        
        /*
            Boolean connectives.
        */
        [](negation, term argument)                  -> set_var { return free_vars(argument); },
        [](conjunction, std::vector<term> arguments) -> set_var { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](disjunction, std::vector<term> arguments) -> set_var { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](implication, std::vector<term> arguments) -> set_var { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },

        /*
            Boolean and first-order predicates
        */
        [](equal, std::vector<term> arguments)    -> set_var   { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](distinct, std::vector<term> arguments) -> set_var   { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](atom, std::vector<term> arguments)     -> set_var   { 
                                                                    set_var fv = { };
                                                                    for (term t : arguments){
                                                                        fv = set_union(fv, free_vars(t));
                                                                    }
                                                                    return fv;
                                                                },

        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)        -> set_var { return free_vars(argument); },
        [](w_tomorrow, term argument)      -> set_var { return free_vars(argument); },
        [](eventually, term argument)      -> set_var { return free_vars(argument); },
        [](always, term argument)          -> set_var { return free_vars(argument); },
        [](until, term left, term right)   -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](release, term left, term right) -> set_var { return set_union(free_vars(left), free_vars(right)); },

        /*
            Past LTL operators.
        */
        [](yesterday, term argument)            -> set_var { return free_vars(argument); },
        [](w_yesterday, term argument)          -> set_var { return free_vars(argument); },
        [](once, term argument)                 -> set_var { return free_vars(argument); },
        [](historically, term argument)         -> set_var { return free_vars(argument); },
        [](since, term left, term right)        -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](triggered, term left, term right)    -> set_var { return set_union(free_vars(left), free_vars(right)); },
        
        /*
            Arithmetic operators.
        */
        [](minus, term argument)                -> set_var { return free_vars(argument); },
        [](sum, term left, term right)          -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](product, term left, term right)      -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](difference, term left, term right)   -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](division, term left, term right)     -> set_var { return set_union(free_vars(left), free_vars(right)); },

        /*
            Relational comparisons
        */
        [](less_than, term left, term right)        -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](less_than_eq, term left, term right)     -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than, term left, term right)     -> set_var { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than_eq, term left, term right)  -> set_var { return set_union(free_vars(left), free_vars(right)); }
    );
}

void surrogates(term t, module &Gamma) {
    match(t)(
        [&](tomorrow t, term body) {
            std::vector<decl> freevars;
            std::optional<object> obj
            for ( variable var : free_vars(body)) { 
                obj = Gamma.lookup(var);
                freevars.push_back(decl(obj.value().entity()->name, obj.value().entity()->type));
            }
            object a = Gamma.declare(decl(variable{label{std::pair{"xs", t.unique_id()}}}, types::function(freevars, types::boolean())), resolution::delayed);
        }
    );
}

int main() {
    module mod;

    object c = mod.define("c", types::integer(), 3);
    object w = mod.declare("w", types::integer());
    variable x = "x";
    variable y = "y";
    variable z = "z";

    // Free variables as variable objects: z.
    exists t = exists({{x, types::integer()}}, forall({{y, types::integer()}}, ((x > y) && (X(x) < c)) || (x + z) < (y + w)));
    
    module mod2;
    surrogates(X(t), mod2);
    std::cout << mod2.lookup(z).has_value() << std::endl;
    return 0;
}