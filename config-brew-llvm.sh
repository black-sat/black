
#!/bin/bash

CC=$(brew --prefix llvm)/bin/clang \
CXX=$(brew --prefix llvm)/bin/clang++ \
LDFLAGS="-L$(brew --prefix llvm)/lib/c++ -Wl,-rpath,$(brew --prefix llvm)/lib/c++" \
CXXFLAGS="-I$(brew --prefix llvm)/include" \
cmake "$@"