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

#include <black/frontend/tracecheck.hpp>
#include <black/frontend/io.hpp>
#include <black/frontend/support.hpp>

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/support/tribool.hpp>

#include <iostream>
#include <sstream>

#define JSON_DIAGNOSTICS 1
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace black::frontend 
{
  struct trace_t {
    std::optional<size_t> loop;
    std::vector<std::map<std::string, black::tribool>> states;
  };

  static
  bool check(trace_t trace, formula f, size_t t) { 
    return f.match(
      [](boolean b) {
        return b.value();
      },
      [&](atom a) {
        black_assert(a.label<std::string>().has_value());
        std::string p = *a.label<std::string>();

        auto state = trace.states[t];
        auto it = state.find(p);
        if(it == state.end())
          return true;
        
        black::tribool value = it->second;
        if(value == true || value == black::tribool::undef)
          return true;
        return false;
      },
      [&](tomorrow, formula f) {
        if(t < trace.states.size() - 1)
          return check(trace, f, t+1);
        
        if(trace.loop)
          return check(trace, f, *trace.loop);
        
        return true;
      },
      [&](yesterday, formula f) {
        if(t == trace.loop) {
          if(t > 0)
            return check(trace, f, t - 1) || 
                   check(trace, f, trace.states.size() - 1);
          if(t == 0)
            return check(trace, f, trace.states.size() - 1);
        }
        
        if(t == 0)
          return false;
        return check(trace, f, t - 1);
      },
      [&](w_yesterday, formula f) {
        if(t == trace.loop) {
          if(t > 0)
            return check(trace, f, t - 1) || 
                   check(trace, f, trace.states.size() - 1);
          if(t == 0)
            return check(trace, f, trace.states.size() - 1);
        }
        
        if(t == 0)
          return true;
        return check(trace, f, t - 1);
      },
      [&](negation, formula f) {
        return !check(trace, f, t);
      },
      [&](conjunction, formula l, formula r) {
        return check(trace, l, t) && check(trace, r, t);
      },
      [&](disjunction, formula l, formula r) {
        return check(trace, l, t) || check(trace, r, t);
      },
      [&](implication, formula l, formula r) {
        return !check(trace, l, t) || check(trace, r, t);
      },
      [&](iff, formula l, formula r) {
        return check(trace, l, t) == check(trace, r, t);
      },
      [&](eventually) {
        alphabet *sigma = f.alphabet();
        return check(trace, U(sigma->top(), f), t);
      },
      [&](once) {
        alphabet *sigma = f.alphabet();
        return check(trace, S(sigma->top(), f), t);
      },
      [&](always, formula f) {
        return check(trace, !F(!f), t);
      },
      [&](historically) {
        return check(trace, !P(!f), t);
      },
      [&](until) {
        return false;
      },
      [&](release) {
        return false;
      },
      [&](since) {
        return false;
      },
      [&](triggered) {
        return false;
      });
  }

  static
  int check(trace_t trace, formula f) {
    bool result = check(trace, f, 0);
    if(result)
      io::message("SATISFIED");
    else
      io::message("UNSATISFIED");
    
    return 0;
  }

  static 
  trace_t
  parse_trace(std::optional<std::string> const&tracepath, std::istream &trace) 
  {
    std::string path = tracepath ? *tracepath : "<stdin>";
    json j;
    try {
      j = json::parse(trace);

      json model = j["model"];
      if(model.is_null()) 
        io::fatal(status_code::syntax_error, "{}: missing model", path);

      trace_t trace;
      trace.loop = model["loop"];
      
      for(json jstate : model["states"]) {
        std::map<std::string, black::tribool> state;

        for(auto it = jstate.begin(); it != jstate.end(); ++it) {
          black::tribool value = black::tribool::undef;
          if(it.value().get<std::string>() == "undef")
            value = black::tribool::undef;
          else if(it.value().get<std::string>() == "true")
            value = true;
          else if(it.value().get<std::string>() == "false")
            value = false;
          else {
            io::fatal(
              status_code::syntax_error, 
              "{}: invalid proposition value",
              path
            );
          }

          state.insert({it.key(), value});
        }
        trace.states.push_back(state);
      }

      if(model["size"] != trace.states.size()) {
        io::fatal(
          status_code::syntax_error, 
          "{}: \"size\" field and effective model size disagree",
          path
        );
      }

    } catch (json::exception& ex) {
      io::fatal(status_code::syntax_error, "{}:{}", path, ex.what());
    }

    return {};
  }

  static
  int trace_check(
    std::optional<std::string> const&path,
    std::istream &file,
    std::optional<std::string> const&tracepath,
    std::istream &tracefile
  ) {
    black::alphabet sigma;

    black::formula f = 
      *black::parse_formula(sigma, file, formula_syntax_error_handler(path));

    trace_t trace = parse_trace(tracepath, tracefile);

    return check(trace, f);
  }

  int trace_check() {
    if(!cli::filename && !cli::formula) {
      command_line_error("please specify a filename or the --formula option");
      quit(status_code::command_line_error);
    }

    if(*cli::filename == "-" && *cli::trace_check == "-") {
      command_line_error(
        "cannot read from stdin both the formula file and the trace file"
      );
      quit(status_code::command_line_error);
    }

    if(cli::formula) {
      std::istringstream str{*cli::formula};

      if(*cli::trace_check == "-")
        return trace_check(std::nullopt, str, std::nullopt, std::cin);

      std::ifstream tracefile = open_file(*cli::trace_check);
      return trace_check(std::nullopt, str, cli::trace_check, tracefile);
    }

    if(*cli::filename == "-") {
      std::ifstream tracefile = open_file(*cli::trace_check);
      return trace_check(std::nullopt, std::cin, cli::trace_check, tracefile);
    }

    if(*cli::trace_check == "-") {
      std::ifstream file = open_file(*cli::filename);
      return trace_check(cli::filename, file, std::nullopt, std::cin);
    }

    std::ifstream file = open_file(*cli::filename);
    std::ifstream tracefile = open_file(*cli::trace_check);
    return trace_check(cli::filename, file, cli::trace_check, tracefile);
  }
}
