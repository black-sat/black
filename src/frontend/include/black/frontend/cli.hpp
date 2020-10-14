//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#ifndef BLACK_CLI_HPP
#define BLACK_CLI_HPP

#include <string>
#include <optional>
#include <cstdint>

namespace black::frontend
{
  // Levels of output verbosity
  enum class verbosity : int8_t {
    fatal   =  1,
    error   =  2,
    warning =  3,
    message =  4,
    debug   =  5,
    trace   =  6
  };

  // Comparison between verbosity levels
  inline bool operator<(verbosity v1, verbosity v2) {
    return static_cast<int8_t>(v1) < static_cast<int8_t>(v2);
  }

  //
  // Status codes returned from the executable to the environment.
  // It's useful to return meaningful codes and document them to help
  // the usage of `black` in shell scripts
  //
  enum class status_code : uint8_t {
    success = 0,            // success status code
    failure = 1,            // generic failure
    command_line_error = 2, // command line parsing errors
    filesystem_error = 3,   // errors related to file operations
    syntax_error = 4,       // syntax errors at logic level (formulas, etc.)
  };

  //
  // Like std::exit(), but accepts a status_code instead of a plain integer
  //
  [[ noreturn ]]
  void quit(status_code);

  // Global parameters given by the command-line arguments
  namespace cli
  {
    // name of the program as given on the command-line (i.e. argv[0])
    inline std::string command_name;

    // name of the input file, if given
    inline std::optional<std::string> filename;

    // maximum bound for BMC algorithms, if given
    inline std::optional<int> bound;

    // name of the selected SAT backend (nullopt for default)
    inline std::optional<std::string> sat_backend;

    // past removing before executing the SAT-encoding (disabled by default)
    inline bool remove_past = false;

    // verbosity level
    inline verbosity verbosity = verbosity::message;
  }

  // parse the command-line arguments, filling the variables declared above.
  void parse_command_line(int argc, char **argv);

}

#endif // BLACK_CLI_HPP
