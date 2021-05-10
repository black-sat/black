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
#include <black/logic/past_remover.hpp>
#include <black/support/tribool.hpp>

#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace black::frontend 
{
  struct trace_t {
    std::optional<std::string> result;
    size_t loop = 0;
    std::vector<std::map<std::string, black::tribool>> states;
  };

  static bool check(trace_t, formula, size_t);

  static
  bool check_atom(trace_t trace, atom a, size_t t) {
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
  }

  static
  bool check_tomorrow(trace_t trace, tomorrow x, size_t t) {
    if(t < trace.states.size() - 1)
      return check(trace, x.operand(), t + 1);
    
    if(t >= trace.loop)
      return check(trace, x.operand(), trace.loop);

    return true;
  }

  // static
  // bool check_yesterday(trace_t trace, yesterday y, size_t t) {
  //   if(t == 0)
  //     return false;
    
  //   if(t == trace.loop)
  //     return check(trace, y.operand(), t - 1) || 
  //             check(trace, y.operand(), trace.states.size() - 1);
    
  //   return check(trace, y.operand(), t - 1);
  // }

  // static 
  // bool check_w_yesterday(trace_t trace, w_yesterday z, size_t t) {
  //   if(t == 0)
  //     return true;
    
  //   if(t == trace.loop)
  //     return check(trace, z.operand(), t - 1) || 
  //             check(trace, z.operand(), trace.states.size() - 1);
    
  //   return check(trace, z.operand(), t - 1);
  // }

  static
  std::optional<size_t>
  check_one(trace_t trace, formula f, size_t begin, size_t end) {
    for(size_t i = begin; i < end; ++i) {
      if(check(trace, f, i))
        return i;
    }
    return {};
  }

  static
  bool check_for_all(trace_t trace, formula f, size_t begin, size_t end) {
    for(size_t i = begin; i < end; ++i) {
      if(!check(trace, f, i))
        return false;
    }
    return true;
  }

  static
  bool check_until(trace_t trace, until u, size_t t) {
    formula l = u.left();
    formula r = u.right();

    // search for 'r'
    std::optional<size_t> rindex = check_one(trace, r, t, trace.states.size());
    
    // if we didn't find 'r' and we are inside the loop, continue from
    // the beginning of the loop
    if(!rindex.has_value())
      rindex = check_one(trace, r, trace.loop, t);

    if(!rindex.has_value())
      return false; // we didn't find 'r', the formula is false

    // check 'l' in all positions until 'r'
    if(*rindex >= t) 
      return check_for_all(trace, l, t, *rindex);
    else
      return check_for_all(trace, l, t, trace.states.size()) &&
             check_for_all(trace, l, trace.loop, *rindex);
  }

  static
  bool check(trace_t trace, formula f, size_t t) {
    return f.match(
      [](boolean b) {
        return b.value();
      },
      [&](atom a) {
        return check_atom(trace, a, t);
      },
      [&](tomorrow x) {
        return check_tomorrow(trace, x, t);
      },
      // [&](yesterday y) {
      //   return check_yesterday(trace, y, t);
      // },
      // [&](w_yesterday z) {
      //   return check_w_yesterday(trace, z, t);
      // },
      [&](until u) {
        return check_until(trace, u, t);
      },
      [&](negation, formula op) {
        return !check(trace, op, t);
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
      [&](eventually, formula op) {
        return check(trace, U(op.alphabet()->top(), op), t);
      },
      [&](always, formula op) {
        return check(trace, !F(!op), t);
      },
      [&](release, formula l, formula r) {
        return check(trace, G(r) || U(r, l && r), t);
      },
      [](past) -> bool { 
        io::fatal(
          status_code::syntax_error, 
          "trace checking is not available for LTL+Past formulas"
        );
      }
    );
  }

  static
  int check(trace_t trace, formula f) {
    bool result = check(trace, f, 0);
    if(result)
      io::message("SATISFIED");
    else {
      io::message("UNSATISFIED");
      quit(status_code::failed_check);
    }
    
    return 0;
  }

  static 
  trace_t
  parse_trace(std::optional<std::string> const&tracepath, std::istream &file) 
  {
    std::string path = tracepath ? *tracepath : "<stdin>";
    json j;
    try {
      j = json::parse(file);

      trace_t trace;
      if(!j["result"].is_null())
        trace.result = j["result"];

      json model = j["model"];
      if(model.is_null())
        return trace;

      trace.loop = model["loop"];
      
      if(model["states"].size() == 0)
        io::fatal(status_code::syntax_error, "{}: empty model", path);

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

      return trace;

    } catch (json::exception& ex) {
      io::fatal(status_code::syntax_error, "{}:{}", path, ex.what());
    }
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

    if(cli::expected_result) {
      if(trace.result != *cli::expected_result) {
        io::message("MISMATCH");
        quit(status_code::failed_check);
      }
    }

    if((!trace.result || trace.result != "SAT") && trace.states.size() == 0) {
      io::message("MATCH");
      quit(status_code::success);
    }

    return check(trace, f);
  }

  int trace_check() {
    if(!cli::filename && !cli::formula) {
      command_line_error("please specify a filename or the --formula option");
      quit(status_code::command_line_error);
    }

    if(cli::filename && cli::formula) {
      command_line_error(
        "please specify only either a filename or the --formula option"
      );
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
