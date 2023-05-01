Exception types
=================

.. cpp:namespace:: black::support

.. cpp:class:: exception : public std::logic_error

   Base class for all the exception types thrown by BLACK.

.. cpp:class:: unreachable_error : public exception

   Exception type thrown by :c:macro:`black_unreachable` in case of failure.

   :constructor:
      .. cpp:function:: unreachable_error(const char *filename, size_t line) 
         
         :param filename: the name of the file of the 
            :c:macro:`black_unreachable` call.
         :param line: the line of the :c:macro:`black_unreachable` call.

   :members:
      .. cpp:function:: const char* filename() const

         :returns: the name of the file of the :c:macro:`black_unreachable` 
            call.

      .. cpp:function:: size_t line() const

         :returns: the line of the :c:macro:`black_unreachable` call.

.. cpp:class:: assert_error : public exception

   Exception type thrown by :c:macro:`black_assert` in case of failure.

   :constructor:
      .. cpp:function:: assert_error( \
            const char *filename, size_t line, const char *expression)

         :param filename: the name of the file of the failed   
            :c:macro:`black_assert` call.

         :param line: the line of the failed :c:macro:`black_assert` call.

         :param expression: a string representation of the asserted expression 
            of the failed :c:macro:`black_assert` call.

   :members:
      .. cpp:function:: const char* filename() const

         :returns: the name of the file of the failed :c:macro:`black_assert` 
            call.

      .. cpp:function:: size_t line() const

         :returns: the line of the failed :c:macro:`black_assert` call.

      .. cpp:function:: const char* expression() const

         :returns: a string representation of the asserted expression of the 
            failed :c:macro:`black_assert` call.

.. cpp:type:: source_location

   `using` declaration for `std::source_location` if available (with a  
   compliant C++20 compiler), or a dummy type otherwise.

.. cpp:class:: assume_error : public assert_error

   Exception type thrown by :c:macro:`black_assume` in case of failure.

   :constructor:
      .. cpp:function:: assume_error( \
               const char *function,  \
               const char *filename, size_t line, \
               source_location const& loc, \
               const char *expression, const char *message \
            )
         
         :param function: the name of the function that contains the failed
            :c:macro:`black_assume` call.

         :param filename: the name of the file of the failed   
            :c:macro:`black_assert` call.

         :param line: the line of the failed :c:macro:`black_assert` call.

         :param loc: :cpp:type:`source_location` of the failed `black_assume` 
            call.
         
         :param expression: a string representation of the asserted expression 
            of the failed :c:macro:`black_assert` call.
         
         :param message: the message associated with the failed
            :c:macro:`black_assume` call.

   :members:
      .. cpp:function:: const char* function() const

         :returns: the name of the function that contains the failed
            :c:macro:`black_assume` call.
      
      .. cpp:function:: const char* message() const

         :returns: the message associated with the failed
            :c:macro:`black_assume` call.

.. cpp:class:: template<typename Error> bad_result_access : public exception

   Exception type thrown by value access members of a :cpp:class:`result` 
   object that contains an error.

   :constructor: 
      .. cpp:function:: bad_result_access(Error err)

         :param err: the error contained in the :cpp:class:`result` object.

   :members:
      .. cpp:function:: Error const& error() const
         
         :returns: the error contained in the :cpp:class:`result` object.