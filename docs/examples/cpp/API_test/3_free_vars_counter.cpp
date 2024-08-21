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

using set_term = std::unordered_set<term, std::hash<term>, black::logic::term_equal_to<>>;

set_term set_union(set_term set1, set_term set2) {
    set1.insert (set2.begin(), set2.end());
    return set1;
}

/*
    Removes any quantified variable (q_vars) from the set of free variables vars.
*/
set_term quantify(std::vector<decl> q_vars, set_term vars) {
    for (decl d : q_vars) {
        vars.erase(vars.find(d.name));
    }
    return vars;
}

set_term free_vars(term t) { 
    return match(t)(
        /*
            Base cases. In case of an object, distinguish between declared and defined objects.
        */
        [](variable var) -> set_term { return { var }; },
        [](object obj)   -> set_term { 
            if(obj.entity()->value.has_value()) { 
                return { };
            }
            return { obj };
        },

        /*
            Quantifiers.
        */
        [](exists, std::vector<decl> decls, term body) -> set_term { return quantify(decls, free_vars(body)); },
        [](forall, std::vector<decl> decls, term body) -> set_term { return quantify(decls, free_vars(body)); },
        
        /*
            Boolean connectives.
        */
        [](negation, term argument)                  -> set_term { return free_vars(argument); },
        [](conjunction, std::vector<term> arguments) -> set_term { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](disjunction, std::vector<term> arguments) -> set_term { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](implication, std::vector<term> arguments) -> set_term { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },

        /*
            Boolean and first-order predicates
        */
        [](equal, std::vector<term> arguments)    -> set_term   { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](distinct, std::vector<term> arguments) -> set_term   { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](atom, std::vector<term> arguments)     -> set_term   { 
                                                                    set_term fv = { };
                                                                    for (term t : arguments){
                                                                        fv = set_union(fv, free_vars(t));
                                                                    }
                                                                    return fv;
                                                                },

        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)        -> set_term { return free_vars(argument); },
        [](w_tomorrow, term argument)      -> set_term { return free_vars(argument); },
        [](eventually, term argument)      -> set_term { return free_vars(argument); },
        [](always, term argument)          -> set_term { return free_vars(argument); },
        [](until, term left, term right)   -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](release, term left, term right) -> set_term { return set_union(free_vars(left), free_vars(right)); },

        /*
            Past LTL operators.
        */
        [](yesterday, term argument)            -> set_term { return free_vars(argument); },
        [](w_yesterday, term argument)          -> set_term { return free_vars(argument); },
        [](once, term argument)                 -> set_term { return free_vars(argument); },
        [](historically, term argument)         -> set_term { return free_vars(argument); },
        [](since, term left, term right)        -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](triggered, term left, term right)    -> set_term { return set_union(free_vars(left), free_vars(right)); },
        
        /*
            Arithmetic operators.
        */
        [](minus, term argument)                -> set_term { return free_vars(argument); },
        [](sum, term left, term right)          -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](product, term left, term right)      -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](difference, term left, term right)   -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](division, term left, term right)     -> set_term { return set_union(free_vars(left), free_vars(right)); },

        /*
            Relational comparisons
        */
        [](less_than, term left, term right)        -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](less_than_eq, term left, term right)     -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than, term left, term right)     -> set_term { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than_eq, term left, term right)  -> set_term { return set_union(free_vars(left), free_vars(right)); }
    );
}



int count_free_vars(term t) {
    return free_vars(t).size();
}

int main() {
    module mod;

    object c = mod.define("c", types::integer(), 3);
    object w = mod.declare("w", types::integer());
    variable x = "x";
    variable y = "y";
    variable z = "z";

    // Free variables as variable objects: z.
    term t = exists({{x, types::integer()}}, forall({{y, types::integer()}}, ((x > y) && (X(x) < c)) || (x + z) < (y + w)));
    std::cout << "Term t: " << term_to_string(t) << std::endl;
    std::cout << "Number of free variables: " << count_free_vars(t) << std::endl;
    std::cout << "The free variables are: ";
    for (term v : free_vars(t)) {
        std::cout << "\t-" << term_to_string(v) << std::endl;
    }

    // Free variable as declared object: w.
    exists f = exists({{x, types::integer()}}, x < w);
    set_term st = free_vars(f);
    std::cout << std::endl << "Term f: " << term_to_string(f) << std::endl;

    std::cout << "The free variables are: ";
    for (term t : free_vars(f)) {
        std::cout << "\t-" << term_to_string(t) << std::endl;
    }

    return 0;
}