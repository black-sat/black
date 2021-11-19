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
#include <algorithm>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace black::frontend 
{
  using state_t = std::map<std::string, black::tribool>;

  struct trace_t {
    std::optional<std::string> result;
    size_t loop = 0;
    std::vector<state_t> states;
  };

  static bool check(trace_t, formula, size_t);

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
    black_assert(a.label<std::string>().has_value());
    std::string p = *a.label<std::string>();

    std::optional<state_t> state = state_at(trace, t);
    
    // all the states at the end of a non-looping model are
    // full of don't cares so we return true (we may choose false as well)
    if(!state.has_value())
      return true;
      
    auto it = state->find(p);
    if(it == state->end())
      return true;
    
    black::tribool value = it->second;
    if(value == true || value == black::tribool::undef)
      return true;

    return false;
  }

  static
  std::optional<size_t>
  find_one(trace_t trace, formula f, size_t begin, size_t end) {
    for(size_t i = begin; i < end; ++i) {
      if(check(trace, f, i))
        return i;
    }
    return {};
  }

  static
  std::optional<size_t>
  find_one_reverse(trace_t trace, formula f, size_t begin, size_t end) {
    for(ssize_t i = (ssize_t)begin; i >= (ssize_t)end; --i) {
      if(check(trace, f, (size_t)i))
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
  size_t depth(formula f) {
    return f.match(
      [](boolean) -> size_t { return 1; },
      [](proposition) -> size_t { return 1; },
      [](yesterday, formula op) { return 1 + depth(op); },
      [](w_yesterday, formula op) { return 1 + depth(op); },
      [](once, formula op) { return 1 + depth(op); },
      [](historically, formula op) { return 1 + depth(op); },
      [](since, formula l, formula r) {
        return 1 + std::max(depth(l), depth(r));
      },
      [](triggered, formula l, formula r) {
        return 1 + std::max(depth(l), depth(r));
      },
      [](unary, formula op) {
        return depth(op);
      },
      [](binary, formula l, formula r) {
        return std::max(depth(l), depth(r));
      }
    );
  }

  static
  bool check_until(trace_t trace, until u, size_t t) {
    formula l = u.left();
    formula r = u.right();

    size_t period = trace.states.size() - trace.loop;
    size_t d = depth(u);
    black_assert(d >= 1);

    size_t end = std::max(t, trace.states.size()) + period + (period * d);

    // search for 'r'
    std::optional<size_t> rindex = find_one(trace, r, t, end);
    
    if(!rindex.has_value())
      return false; // we didn't find 'r', the formula is false

    // check 'l' in all positions until 'r'
    return check_for_all(trace, l, t, *rindex);
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
  bool state_exists(trace_t trace, size_t t) {
    return !cli::finite || t < trace.states.size();
  }

  static
  bool check(trace_t trace, formula f, size_t t) 
  {
    static std::unordered_map<std::tuple<formula, size_t>, bool> memo;
    if(auto it = memo.find({f, t}); it != memo.end())
      return it->second;

    bool result = f.match(
      [](boolean b) {
        return b.value();
      },
      [&](proposition a) {
        return check_proposition(trace, a, t);
      },
      [&](tomorrow, formula op) {
        return state_exists(trace, t + 1) && check(trace, op, t + 1);
      },
      [&](w_tomorrow, formula op) {
        return !state_exists(trace, t + 1) || check(trace, op, t + 1);
      },
      [&](yesterday, formula op) {
        return t > 0 && check(trace, op, t - 1);
      },
      [&](w_yesterday, formula op) {
        return t == 0 || check(trace, op, t - 1);
      },
      [&](until u) {
        return check_until(trace, u, t);
      },
      [&](since s) {
        return check_since(trace, s, t);
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
        return check(trace, U(op.sigma()->top(), op), t);
      },
      [&](always, formula op) {
        return check(trace, !F(!op), t);
      },
      [&](w_until, formula l, formula r) {
        return check(trace, G(l) || U(l, r), t);
      },
      [&](release, formula l, formula r) {
        return check(trace, !U(!l, !r), t);
      },
      [&](s_release, formula l, formula r) {
        return check(trace, !W(!l, !r), t);
      },
      [&](once, formula op) {
        return check(trace, S(op.sigma()->top(), op), t);
      },
      [&](historically, formula op) {
        return check(trace, !O(!op), t);
      },
      [&](triggered, formula l, formula r) {
        return check(trace, !S(!l, !r), t);
      }
    );

    if(cli::verbose)
      io::println("{} at t = {} is {}", to_string(f), t, result);

    memo.insert({{f,t}, result});

    return result;
  }

  static
  int check(trace_t trace, formula f) {
    size_t initial_state = 0;
    if(cli::initial_state)
      initial_state = *cli::initial_state;

    bool result = check(trace, f, initial_state);
    if(result)
      io::println("TRUE");
    else {
      io::println("FALSE");
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

      if(!cli::finite)
        trace.loop = model["loop"];
      else
        trace.loop = model["size"];

      if(cli::finite && !model["loop"].is_null())
        io::fatal(
          status_code::syntax_error, 
          "expected a finite model, but a \"loop\" field is present"
        );
      
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

      if(trace.loop > trace.states.size()) {
        io::fatal(
          status_code::syntax_error, 
          "{}: \"loop\" field greater than model size",
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
        io::println("MISMATCH");
        quit(status_code::failed_check);
      }
    }

    if((!trace.result || trace.result != "SAT") && trace.states.size() == 0) {
      io::println("MATCH");
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

    if(*cli::filename == "-" && cli::trace == "-") {
      command_line_error(
        "cannot read from stdin both the formula file and the trace file"
      );
      quit(status_code::command_line_error);
    }

    if(cli::formula) {
      std::istringstream str{*cli::formula};

      if(cli::trace == "-")
        return trace_check(std::nullopt, str, std::nullopt, std::cin);

      std::ifstream tracefile = open_file(cli::trace);
      return trace_check(std::nullopt, str, cli::trace, tracefile);
    }

    if(*cli::filename == "-") {
      std::ifstream tracefile = open_file(cli::trace);
      return trace_check(std::nullopt, std::cin, cli::trace, tracefile);
    }

    if(cli::trace == "-") {
      std::ifstream file = open_file(*cli::filename);
      return trace_check(cli::filename, file, std::nullopt, std::cin);
    }

    std::ifstream file = open_file(*cli::filename);
    std::ifstream tracefile = open_file(cli::trace);
    return trace_check(cli::filename, file, cli::trace, tracefile);
  }
}
