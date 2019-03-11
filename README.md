# BLACK
Bounded Ltl sAtisfiability ChecKer

## Setup

Setup instructions for macOS (coming soon for other systems).

### Requirements
1. A C++17 compiler, e.g. clang 7 (default on macOS mojave)
2. CMake 3.8 or upper.  
  `brew install cmake`

### Compilation
1. Clone the repository recursively, to get all the submodules  
   ```
   $ git clone --recursive git@github.com:lucageatti/BLACK.git
   ```
   Or, if you already cloned the repository, pull the changes and update
   the submodules:
   ```
   $ git pull
   $ git submodule update --init --recursive
   ```
1. Create a `build` directory inside the source directory  
   `$ cd BLACK && mkdir build`
2. Run `cmake` inside the `build` directory  
   `$ cd build && cmake ..`
3. Build  
   `$ make`
4. Run `./black` to run the built executable, and `tests/tests` to run the test
   suite  
   ```
   $ tests/tests
   =================================================================
     All tests passed (2 assertions in 1 test case)

   $ ./black
   Changing the world, one solver at the time...
   ```
