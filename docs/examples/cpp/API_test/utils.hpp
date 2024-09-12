#include <black/logic>

using namespace black;
using namespace black::support;
using namespace black::logic;

std::string term_to_string(term t);

std::string term_to_string(term t) {
    return match(t)(
        /*
            Object terms.
        */
        [](variable var) { return var.name().to_string(); },
        [](prime p)      { return term_to_string(p.object()) + "'"; },
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