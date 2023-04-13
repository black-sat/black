//
// Synthetico - Pure-past LTL synthesizer based on BLACK
//
// (C) 2023 Nicola Gigante
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
//

#include <black/qbf/backend.hpp>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>

#include <iostream>
#include <fstream>
#include <memory>
#include <random>

namespace black_internal::synth {

  static void dump(qdimacs const& qd, std::string filename) {
    std::ofstream fstr(filename);

    if(fstr.fail())
      throw std::runtime_error(
        "unable to open file '" + filename + "': " + strerror(errno)
      );
    
    fstr << to_string(qd) << "\n";
    std::cerr << to_string(qd) << "\n";
  }

  bool is_sat(qdimacs const& qd) {

    using namespace std::literals;

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(1, 99999);

    std::string input_filename = 
      "synth-" + std::to_string(dist(gen)) + ".dqdimacs";

    dump(qd, input_filename);

    std::string output_filename = 
      "synth-" + std::to_string(dist(gen)) + ".out";

    int fd = open(output_filename.c_str(), O_WRONLY | O_CREAT, 0600);
    int null = open("/dev/null", O_WRONLY);

    if(fd == -1) {
      throw std::runtime_error(
        "unable to open file '" + output_filename + "': " + strerror(errno)
      );
    }
    if(null == -1) {
      throw std::runtime_error(
        "unable to open /dev/null: "s + strerror(errno)
      );
    }

    pid_t pid = fork();

    if(!pid) { // child process
      dup2(fd, 1);
      dup2(null, 2);
      execlp("pedant", "pedant", input_filename.c_str(), nullptr);
      
      exit(errno);
    }

    close(fd);
    int status = 0;
    wait(&status);

    // if(WIFEXITED(status)) { 
    //   int err = WEXITSTATUS(status);
    //   if(err != 0)
    //     throw std::runtime_error("unable to launch backend: "s + strerror(err));
    // }

    std::ifstream fstr(output_filename);
    if(!fstr.good())
      throw std::runtime_error(
        "unable to open backend output file '" + 
        output_filename + "': " + strerror(errno)
      );

    while(fstr.good()) {
      std::string line;
      std::getline(fstr, line);

      if(line == "SATISFIABLE")
        return true;
      if(line == "UNSATISFIABLE")
        return false;
    }

    throw std::runtime_error("unable to parse backend output: missing `SATISFIABLE` or `UNSATISFIABLE` tag");
  }

}
