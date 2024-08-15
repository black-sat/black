#include <black/logic>
#include<iostream>
#include<vector>

using namespace black;
using namespace black::support;
using namespace black::logic;


std::string term_to_string(term t);
std::string object_to_string(object obj);

std::string term_to_string(term t) {
    return match(t)(
        /*
            Object terms.
        */
        [](object obj)   {            
            if(obj.entity()->value.has_value()) { 
                return match(obj.entity()->value.value())(
                    [](integer i)  { return std::to_string(i.value()); },
                    [](real r)     { return std::to_string(r.value()); },                    
                    [](boolean b)  { return std::to_string(b.value()); },
                    [obj] (lambda) { return obj.entity()->name.name().to_string(); }
                );
            }
            return obj.entity()->name.name().to_string();
        },
        [](variable var) { return var.name().to_string(); },

        /*
            Boolean and first-order predicates.
        */
        [](equal, std::vector<term> arguments)      { return "(" + term_to_string(arguments[0]) + " == " + term_to_string(arguments[1]) + ")"; },
        [](distinct, std::vector<term> arguments)   { return "(" + term_to_string(arguments[0]) + " != " + term_to_string(arguments[1]) + ")"; },
        
        [](atom, term head, std::vector<term> arguments) {
            std::string result = term_to_string(head) + "(";
            for (auto i = arguments.begin(); i != arguments.end(); i ++) {
                if (i != arguments.begin()) result = result + ", ";
                result = result + term_to_string(*i);
            }
            return result + ")";
        }, 

        [](exists, std::vector<decl> decls, term body) {
            std::string result = "exists ";
            for (decl d : decls) {
                result = result + term_to_string(d.name().head()) + " ";
            }
            result = result + ". (" + term_to_string(body) + ")";
            return result;
        },
        [](forall, std::vector<decl> decls, term body) {
            std::string result = "forall ";
            for (decl d : decls) {
                result = result + term_to_string(d.name().head()) + " ";
            }
            result = result + ". (" + term_to_string(body) + ")";
            return result;
        },         

        /*
            Boolean connectives.
        */
        [](negation, term argument) { return "!(" + term_to_string(argument) + ")"; },
        [](conjunction, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " && " + term_to_string(arguments[1]) + ")"; },
        [](disjunction, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " || " + term_to_string(arguments[1]) + ")"; },
        [](implication, std::vector<term> arguments) { return "(" + term_to_string(arguments[0]) + " => " + term_to_string(arguments[1]) + ")"; },

        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)         { return "X("  + term_to_string(argument) + ")"; },
        [](w_tomorrow, term argument)       { return "wX(" + term_to_string(argument) + ")"; },
        [](eventually, term argument)       { return "F("  + term_to_string(argument) + ")"; },
        [](always, term argument)           { return "G("  + term_to_string(argument) + ")"; },
        [](until, term left, term right)    { return "("  + term_to_string(left) + " U " + term_to_string(right) + ")"; },
        [](release, term left, term right)  { return "("  + term_to_string(left) + " R " + term_to_string(right) + ")"; },

        /*
            Future LTL operators.
        */
        [](yesterday, term argument)            { return "Y(" + term_to_string(argument) + ")"; },
        [](w_yesterday, term argument)          { return "Z(" + term_to_string(argument) + ")"; },
        [](once, term argument)                 { return "O(" + term_to_string(argument) + ")"; },
        [](historically, term argument)         { return "H(" + term_to_string(argument) + ")"; },
        [](since, term left, term right)        { return "(" + term_to_string(left) + " S " + term_to_string(right) + ")"; },
        [](triggered, term left, term right)    { return "(" + term_to_string(left) + " T " + term_to_string(right) + ")"; },

        /*
            Arithmetic operators.
        */
        [](minus, term argument)                { return "-(" + term_to_string(argument) + ")"; },
        [](sum, term left, term right)          { return "(" + term_to_string(left) + " + " + term_to_string(right) + ")"; },
        [](product, term left, term right)      { return "(" + term_to_string(left) + " * " + term_to_string(right) + ")"; },
        [](difference, term left, term right)   { return "(" + term_to_string(left) + " - " + term_to_string(right) + ")"; },
        [](division, term left, term right)     { return "(" + term_to_string(left) + " / " + term_to_string(right) + ")"; },

        /*
            Relational comparisons.
        */
        [](less_than, term left, term right)         { return "(" + term_to_string(left) + " < "  + term_to_string(right) + ")"; },
        [](less_than_eq, term left, term right)      { return "(" + term_to_string(left) + " <= " + term_to_string(right) + ")"; },
        [](greater_than, term left, term right)      { return "(" + term_to_string(left) + " > "  + term_to_string(right) + ")"; },
        [](greater_than_eq, term left, term right)   { return "(" + term_to_string(left) + " >= " + term_to_string(right) + ")"; }
    );
}

std::optional<term> future_to_past(term t) {
    return match(t)(
        /*
            Object terms.
        */
        [](object obj)   { return obj; },
        [](variable var) { return var; },

        // Future operator: convert to past and keep digging.
        [](tomorrow to) -> std::optional<term> { 
            try { return Y(future_to_past(to.argument()).value()); }
            catch(black::logic::error){ return std::nullopt; }
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
        [](always, term argument) -> std::optional<term> { 
            try {
                return H(future_to_past(argument).value());
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
        [](object obj) ->   std::optional<term> { return obj; },
        [](variable var) -> std::optional<term> { return var; },

        /*
            Catches types equal, distinct, conjunction, disjunction, implication.
        */
        []<typename T>(T, std::vector<term> args) -> std::optional<term> {
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

    term f = X(p && q) && (c > wX(x));

    future_to_past(f);

    return 0;
}