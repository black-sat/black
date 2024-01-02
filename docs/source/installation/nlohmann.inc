To install ``nlohmann-json``::

   $ git clone https://github.com/nlohmann/json.git
   $ cd json
   $ mkdir build && cd build
   $ cmake -DJSON_BuildTests=OFF ..
   $ sudo make install