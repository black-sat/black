#include <black/logic>
#include <iostream>
#include <vector>
#include <set>
#include <iterator>
#include <algorithm>

using namespace black;
using namespace black::support;
using namespace black::logic;


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

/*
    The following functions have nothing to do with term_to_string. Their use strictly involves the identification of free variables as strings.
*/
std::string var_to_string(variable v) {
    return v.name().to_string();
}

std::string decl_to_string(decl d) {
    return var_to_string(d.name);
}

std::set<std::string> decls_to_string(std::vector<decl> decls) {
    std::set<std::string> result;
    for (decl d : decls) {
        result.insert(decl_to_string(d));
    }
    return result;
}

std::set<std::string> set_union(std::set<std::string> set1, std::set<std::string> set2) {
    set1.insert (set2.begin(), set2.end());
    return set1;
}

std::set<std::string> set_difference(std::set<std::string> set1, std::set<std::string> set2) {
    std::set<std::string> result;
    std::set_difference(set1.begin(), set1.end(), set2.begin(), set2.end(), std::inserter(result, result.end()));
    return result;
}

std::set<std::string> free_vars(term t) {
    return match(t)(
        /*
            Base cases.
        */
        [](variable v)  { return std::set<std::string> { var_to_string(v) }; },
        [](object)      { return std::set<std::string> { }; },

        /*
            Quantifiers.
        */
        [](exists, std::vector<decl> decls, term body) { return set_difference(free_vars(body), decls_to_string(decls)); },
        [](forall, std::vector<decl> decls, term body) { return set_difference(free_vars(body), decls_to_string(decls)); },

        /*
            Boolean and first-order predicates
        */
        [](equal, std::vector<term> arguments)      { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](distinct, std::vector<term> arguments)   { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](atom, std::vector<term> arguments)       { 
                                                        std::set<std::string> fv = { };
                                                        for (term t : arguments){
                                                            fv = set_union(fv, free_vars(t));
                                                        }
                                                        return fv;
                                                    },

        /*
            Boolean connectives.
        */
        [](negation, term argument)                  { return free_vars(argument); },
        [](conjunction, std::vector<term> arguments) { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](disjunction, std::vector<term> arguments) { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },
        [](implication, std::vector<term> arguments) { return set_union(free_vars(arguments[0]), free_vars(arguments[1])); },

        /*
            Future LTL operators.
        */
        [](tomorrow, term argument)         { return free_vars(argument); },
        [](w_tomorrow, term argument)       { return free_vars(argument); },
        [](eventually, term argument)       { return free_vars(argument); },
        [](always, term argument)           { return free_vars(argument); },
        [](until, term left, term right)    { return set_union(free_vars(left), free_vars(right)); },
        [](release, term left, term right)  { return set_union(free_vars(left), free_vars(right)); },

        /*
            Past LTL operators.
        */
        [](yesterday, term argument)            { return free_vars(argument); },
        [](w_yesterday, term argument)          { return free_vars(argument); },
        [](once, term argument)                 { return free_vars(argument); },
        [](historically, term argument)         { return free_vars(argument); },
        [](since, term left, term right)        { return set_union(free_vars(left), free_vars(right)); },
        [](triggered, term left, term right)    { return set_union(free_vars(left), free_vars(right)); },
        
        /*
            Arithmetic operators.
        */
        [](minus, term argument)                { return free_vars(argument); },
        [](sum, term left, term right)          { return set_union(free_vars(left), free_vars(right)); },
        [](product, term left, term right)      { return set_union(free_vars(left), free_vars(right)); },
        [](difference, term left, term right)   { return set_union(free_vars(left), free_vars(right)); },
        [](division, term left, term right)     { return set_union(free_vars(left), free_vars(right)); },

        /*
            Relational comparisons
        */
        [](less_than, term left, term right)        { return set_union(free_vars(left), free_vars(right)); },
        [](less_than_eq, term left, term right)     { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than, term left, term right)     { return set_union(free_vars(left), free_vars(right)); },
        [](greater_than_eq, term left, term right)  { return set_union(free_vars(left), free_vars(right)); }

        
    );
}

int count_free_vars(term t) {
    return free_vars(t).size();
}

int main() {
    module mod;

    object c = mod.define("c", types::integer(), 3);
    variable x = "x";
    variable y = "y";
    variable z = "z";

    term t = exists({{x, types::integer()}}, forall({{y, types::integer()}}, ((x > y) && (X(x) < c)) || (x + z) < y));
    std::cout << "Term t: " << term_to_string(t) << std::endl;
    std::cout << "Number of free variables: " << count_free_vars(t) << std::endl;
    std::cout << "The free variables are: ";
    for (std::string v : free_vars(t)) {
        std::cout << v << " " << std::endl;
    }

    return 0;
}