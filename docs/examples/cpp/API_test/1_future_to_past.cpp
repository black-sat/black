#include <black/logic>
#include<iostream>
#include<vector>

using namespace black;
using namespace black::support;
using namespace black::logic;

std::string what_term(term t){
    return match(t)(
        [](conjunction){ return "conjunction"; }, 
        [](tomorrow){ return "tomorrow"; },
        [](yesterday) { return "yesterday"; },  
        [](object) { return "object";}
    );
}

std::optional<term> future_to_past(term t) {
    return match(t)(

        // Future operator: convert to past and keep digging.
        [](tomorrow to) -> std::optional<term> { 
            std::cout << "Tomorrowww!!!" << std::endl;
            try {
                return Y(future_to_past(to.argument()).value());
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },
        [](w_tomorrow wto) -> std::optional<term> { 
            try {
                return Z(future_to_past(wto.argument()).value());
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },
        [](eventually f) -> std::optional<term> { 
            try {
                return O(future_to_past(f.argument()).value());
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },
        [](always g) -> std::optional<term> { 
            try {
                return H(future_to_past(g.argument()).value());
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },
        [](until u) -> std::optional<term> { 
            try {
                return S(
                    future_to_past(u.left()).value(),
                    future_to_past(u.right()).value()
                );
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },
        [](release r) -> std::optional<term> { 
            try {
                return T(
                    future_to_past(r.left()).value(),
                    future_to_past(r.right()).value()
                ); 
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },

        /*
            Catch LTL past operators.
            TODO: fail and return empty object.
        */ 
        [](yesterday) ->    std::optional<term> { return std::nullopt; },
        [](w_yesterday) ->  std::optional<term> { return std::nullopt; },
        [](once) ->         std::optional<term> { return std::nullopt; },
        [](historically) -> std::optional<term> { return std::nullopt; },
        [](since) ->        std::optional<term> { return std::nullopt; },
        [](triggered) ->    std::optional<term> { return std::nullopt; },

        /*
            Catch objects and variables.
            TODO: zoom out.
        */
        [](object obj) ->   std::optional<term> { std::cout << "Object" << std::endl; return obj; },
        [](variable var) -> std::optional<term> { return var; },

        /*
            Catches types equal, distinct, conjunction, disjunction, implication.
        */
        []<typename T>(T, std::vector<term> args) -> std::optional<term> {
            std::cout << "Conjunction detected!!!" << std::endl;
            try{
                args[0] = future_to_past(args[0]).value();
                args[1] = future_to_past(args[1]).value();
                return T(args);
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },

        /*
            Catch type ite to avoid collisions with other following lambdas.
        */ 
        [](ite) -> std::optional<term> { return std::nullopt; },

        /*
            Catch all arithmetic operators (apart from minus) and relational comparisons.
        */
        []<typename T>(T, term left, term right) -> std::optional<term> {
            std::cout << "Relational comparison" << std::endl;
            try {
                return T(future_to_past(left).value(), future_to_past(right).value());
            }
            catch(black::logic::error) {
                return std::nullopt;
            }
        },

        /*
            Catches existential and universal quantifiers.
        */
        []<typename T>(T, std::vector<decl> decls, term body) -> std::optional<term> {
            std::cout << "Quantifier detected!!!" << std::endl;
            try{
                return T(decls, future_to_past(body).value());
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },

        [] (atom, term, std::vector<term>) -> std::optional<term> {
            try{
                return std::nullopt;
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },
        
        [](negation n, term arg) -> std::optional<term> {
            try{
                return n(future_to_past(arg).value());
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        },
        [](minus m, term arg) -> std::optional<term> {
            try{
                return m(future_to_past(arg).value());
            }
            catch(black::logic::error){
                return std::nullopt;
            }
        }
    );
}

int main() {
    module mod;

    object p = mod.declare("p", types::boolean());
    object q = mod.declare("q", types::boolean());
    
    object c = mod.define("c", types::integer(), 42);
    object x = mod.declare("x", types::integer());

    

    future_to_past(p);

    return 0;
}