# BLACK's documentation

## Website

Almost all of BLACK's documentation is written using the Sphinx documentation editing system. It's sources are in the `docs/website` subdirectory of BLACK's source tree.

To build the documentation you need the following dependencies:
1. a C++ compiler (the same you use to build BLACK is fine)
2. Python 3.10 or later
3. the `jq` command-line utility for JSON manipulation.
    You can install it easily with Homebrew on macOS or the distribution's package manager on Linux

How to build:

1. enter the `docs/website` directory and create a Python virtual env:
    ```
    $ cd docs/website
    $ python3 -m venv .venv
    ```

2. activate the virtual environment:
    ```
    $ source .venv/bin/activate
    ```
    This step is needed each time you start a new shell.

3. install build dependencies:
    ```
    $ python3 -m pip install -r requirements.txt
    ```

4. build the documentation:
    ```
    make html
    ```

The documentation is ready in the `_build/html/` folder, open `index.html` with a browser to start.

## Code examples

C++ code examples explained in the API guide on the website are available in the `docs/examples/cpp` subdirectory of BLACK's source tree.

They require BLACK to be installed somewhere (probably in a local directory, given that this is a development branch). The examples can be built as any other CMake project as follows:

1. enter the source directory:
    ```
    $ cd docs/examples/cpp
    ```
2. create a build directory and switch to it:
    ```
    $ mkdir build && cd build
    ```
3. Run `cmake`, providing the location of BLACK's installation if it's not the default one:
    ```
    $ cmake -DCMAKE_INSTALL_PREFIX=/black/location -DCMAKE_BUILD_TYPE=Debug ..
    ```
    The debug configuration is suggested to shorten compile times.
4. Build the examples:
    ```
    $ make
    ```
    
Then, one can run any example by running the corresponding executable. For example:
```
$ ./factorial
```

