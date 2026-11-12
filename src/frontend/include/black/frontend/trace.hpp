//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#ifndef BLACK_FRONTEND_T_HPP
#define BLACK_FRONTEND_T_HPP

#include <black/support/tribool.hpp>

#include <optional>
#include <istream>
#include <string>
#include <map>
//#include <variant>

namespace black::frontend 
{
  using ValueType = std::variant<black::tribool, double, int>;
  using state_t = std::map<std::string, ValueType>;
  
  struct trace_t {
    std::optional<std::string> result;
    size_t loop = 0;
    std::vector<state_t> states;
    std::optional<formula> muc;
  };

  static bool check(trace_t, formula, size_t);

  struct check_lookup {
    formula f;
    size_t t;

    friend bool operator==(check_lookup const&, check_lookup const&) = default;
  };
  } namespace std {

    template<>
    struct hash<black::frontend::check_lookup> {
      size_t operator()(black::frontend::check_lookup l) const {
        using namespace black_internal;
        using namespace black;
        size_t h1 = std::hash<formula>{}(l.f);
        size_t h2 = std::hash<size_t>{}(l.t);

        return hash_combine(h1, h2);
      }
    };

  } namespace black::frontend {

  static
  std::optional<size_t>
  find_one_reverse(trace_t trace, formula f, size_t begin, size_t end) { // controlla in (l S r) se r è valso da oggi (begin) in prima (end=0) 
    for(ssize_t i = (ssize_t)begin; i >= (ssize_t)end; --i) {
      if(check(trace, f, (size_t)i))
        return i; 
    }
    return {};
  }
  
  static
  bool check_for_all(trace_t trace, formula f, size_t begin, size_t end) { // controlla se dall'istante della funzione precedente è sempre valso l
    for(size_t i = begin; i < end; ++i) {
      if(!check(trace, f, i))
        return false;
    }
    return true;
  }

  static
  bool check_since(trace_t trace, since s, size_t t) {
    formula l = s.left();
    formula r = s.right();
    
    // search for 'r'
    std::optional<size_t> rindex = find_one_reverse(trace, r, t, 0);

    if(!rindex.has_value())
      return false; // we didn't find 'r', the formula is false

    // check 'l' in all positions until 'r'
    return check_for_all(trace, l, *rindex + 1, t + 1);
  }


  static
  std::optional<state_t> state_at(trace_t trace, size_t t) {
    if(t < trace.loop)
      return trace.states[t];
    
    size_t period = trace.states.size() - trace.loop;

    if(period)
      return trace.states[((t - trace.loop) % period) + trace.loop];
    
    black_assert(t >= trace.states.size());
    return {};
  }

  static
  bool check_proposition(trace_t trace, proposition a, size_t t) {
    black_assert(a.name().to<std::string>().has_value());
    std::string p = *a.name().to<std::string>();

    std::optional<state_t> state = state_at(trace, t);
   
    // all the states at the end of a non-looping model are
    // full of don't cares so we return true (we may choose false as well)
    if(!state.has_value()){
      return true;
    }
      
    auto it = state->find(p);
    if(it == state->end()){
      return true;
    }

    ValueType value = it->second;  
    try{
      black::tribool trib_val = get<black::tribool>(value);
      if(trib_val == black::tribool::undef){
        return true;
      }
      else if(trib_val == true){
        return true;
      }
      else if(trib_val == false){
        return false;
      } 
    

    }catch(std::exception& e) {
      io::fatal(
        status_code::syntax_error, 
        "invalid proposition value (numeric)"
      );
      // printf("try int\n");
      // try{
      //   if(get<int>(value))
      //     printf("value inttt\n");
      // } catch(std::exception& e){
      //     printf("try double\n");
      //   try{
      //     if(get<double>(value))
      //       printf("value doublettt\n");
      //   } catch(std::exception& e){
      //     io::fatal(
      //       status_code::syntax_error, 
      //       "invalid proposition value"
      //     );
      //   }

      // }
    }

    return false;
  }

    
}


#endif