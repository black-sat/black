# BLACK
Bounded Ltl sAtisfiability ChecKer

## Setup

Setup instructions for macOS (coming soon for other systems).

### Requirements
1. A C++17 compiler, e.g. clang 7 (default on macOS mojave)
2. CMake 3.8 or upper.  
  `brew install cmake`

### Compilation
1. Create a `build` directory inside the source directory  
   `$ cd black && mkdir build`
2. Run `cmake` inside the `build` directory  
   `$ cd build && cmake ..`
3. Build  
   `$ make`
4. Run `./black` to run the built executable, and `tests/test` to run the test
   suite  
   ```
   $ tests/tests
   =================================================================
     All tests passed (2 assertions in 1 test case)

   $ ./black
   Changing the world, one solver at the time...
   ```
