//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2025 Nicola Gigante
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

#include <black/support/range.hpp>
#include <black/logic/logic.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>

using namespace black;

#include <string>
#include <iostream>

enum class pattern {
    D = 0,
    B,
    C1,
    C2,
    C3,
    C4,
    C5
};

formula pattern_D(alphabet &sigma, size_t n);
formula pattern_B(alphabet &sigma, size_t n);
formula pattern_C1(alphabet &sigma, size_t n);
formula pattern_C2(alphabet &sigma, size_t n);
formula pattern_C3(alphabet &sigma, size_t n);
formula pattern_C4(alphabet &sigma, size_t n);
formula pattern_C5(alphabet &sigma, size_t n);

int main(int argc, char **argv) {
    
    size_t n = 0;
    pattern p;
  
    if (argc < 3 || argc > 4) {
      std::cerr << "Please give a pattern name and a size\n";
      return 1;
    }
  
    std::string pstr = argv[1];
    if(pstr == "D")
        p = pattern::D;
    else if(pstr == "B")
        p = pattern::B;
    else if(pstr == "C1")
        p = pattern::C1;
    else if(pstr == "C2")
        p = pattern::C2;
    else if(pstr == "C3")
        p = pattern::C3;
    else if(pstr == "C4")
        p = pattern::C4;
    else if(pstr == "C5")
        p = pattern::C5;
    else {
        std::cerr << "Invalid pattern name: " << pstr << "\n";
        return 1;
    }

    try {
        int in = std::stoi(argv[2]);
        if(in <= 0) {
            std::cerr << "Please provide a positive size\n";
            return 1;
        }
        n = (size_t)in;

    } catch (const std::invalid_argument& e) {
      std::cerr << "Invalid argument: " << e.what() << "\n";
      return 1;
    }
  
    alphabet sigma;

    switch(p) {
        case pattern::D:
            std::cout << to_string(pattern_D(sigma, n)) << "\n";
            break;
        case pattern::B:
            std::cout << to_string(pattern_B(sigma, n)) << "\n";
            break;
        case pattern::C1:
            std::cout << to_string(pattern_C1(sigma, n)) << "\n";
            break;
        case pattern::C2:
            std::cout << to_string(pattern_C2(sigma, n)) << "\n";
            break;
        case pattern::C3:
            std::cout << to_string(pattern_C3(sigma, n)) << "\n";
            break;
        case pattern::C4:
            std::cout << to_string(pattern_C4(sigma, n)) << "\n";
            break;
        case pattern::C5:
            std::cout << to_string(pattern_C5(sigma, n)) << "\n";
            break;
    }

    return 0;
}

static std::string name(std::string s, size_t i) {
    return s + "_" + std::to_string(i);
}

formula pattern_D(alphabet &sigma, size_t n) {

    formula diamonds = big_and(sigma, range(0, n), [&](size_t i) {
        term sp = sigma.variable(name("s", i));
        proposition p = sigma.proposition(name("p", i));
        
        return diamond(sp, p);
    });

    formula sharps = big_and(sigma, range(0,n - 1), [&](size_t i) {
        return sigma.variable(name("s", i)) <= sigma.variable(name("s", i + 1));
    });

    return diamonds && sharps;
}

formula pattern_B(alphabet &sigma, size_t n) {
    
    formula boxes = big_and(sigma, range(0, n), [&](size_t i) {
        term sp = sigma.variable(name("s", i));
        proposition p = sigma.proposition(name("p", i));
        
        return box(sp, p);
    });

    formula sharps = big_and(sigma, range(0,n - 1), [&](size_t i) {
        return sigma.variable(name("s", i)) <= sigma.variable(name("s", i + 1));
    });

    return boxes && sharps;
}

formula pattern_C1(alphabet &sigma, size_t n) {
    proposition p = sigma.proposition("p");
    
    return big_and(sigma, range(0, n), [&](size_t i) {
        term sp = sigma.variable(name("s", i));
        
        return until(diamond(sp, p), X(box(sp, !p)));
    });
}

formula pattern_C2(alphabet &sigma, size_t n) {
    proposition p = sigma.proposition("p");
    proposition q = sigma.proposition("q");

    return big_or(sigma, range(0, n), [&](size_t i) {
        term sp = sigma.variable(name("s", i));

        return F(diamond(sp, p)) && G(box(sp, q));
    });
}

formula pattern_C3(alphabet &sigma, size_t n) {
    term sp = sigma.variable(name("s", n));
    proposition p = sigma.proposition(name("p", n));
    if(n == 1)
        return diamond(sp, p);
    
    return until(pattern_C3(sigma, n - 1), diamond(sp, p));
}

formula pattern_C4(alphabet &sigma, size_t n) {
    term sp = sigma.variable(name("s", n));
    proposition p = sigma.proposition(name("p", n));
    if(n == 1)
        return diamond(sp, p);

    if(n % 2 == 0)
        return until(pattern_C4(sigma, n - 1), diamond(sp, p));
    else
        return release(pattern_C4(sigma, n - 1), diamond(sp, p));
}

static formula pattern_C5(alphabet &sigma, size_t n, formula &sharps) {
    term sp = sigma.variable(name("s", n));
    proposition p = sigma.proposition(name("p", n));
    if(n == 1)
        return box(sp, p);

    sharps = sharps && 
        sigma.variable(name("s", n - 1)) <= sigma.variable(name("s", n));

    return until(pattern_C5(sigma, n - 1, sharps), p);
}

formula pattern_C5(alphabet &sigma, size_t n) {
    formula sharps = sigma.top();
    formula pattern = pattern_C5(sigma, n, sharps);

    return pattern && sharps;
}
