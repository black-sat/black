Error types
============

The following types form a taxonomy of all the possible errors that BLACK can
return *to the user*. These types are **not** thrown as exceptions, but returned
as the error component of a :cpp:class:`result` object. This ensures they are always handled explicitly.

.. cpp:namespace:: black::support

.. cpp:struct:: error 

   A *matchable union type* (see :doc:`match`) representing an user 
   error.

   :alternatives:
      1. :cpp:any:`syntax_error`
      2. :cpp:any:`type_error`
      3. :cpp:any:`backend_error`
      4. :cpp:any:`io_error`

.. cpp:struct:: syntax_error

   An error deriving from some wrong input syntax.

   :constructor:
      .. cpp:function:: syntax_error( \
               std::optional<std::string> filename, size_t line, size_t col, \
               const char *format, Args const& ...args \
            )

         :param filename: the name of the file where the syntax error occurred.
            If `filename` is `std::nullopt`, the error came from some other
            source instead of a file (*.e.g.* the standard input stream.

         :param line: the line where the syntax error occurred.
         :param col: the column where the syntax error occurred.
         :param format: a format string for the error message, in `{fmt}` syntax
         :param ...args: the arguments for the format string

   :members:
      .. cpp:member:: std::optional<std::string> filename

         The name of the file where the syntax error occurred. If `filename` is
         `std::nullopt`, the error came from some other source instead of a file
         (*.e.g.* the standard input stream.

      .. cpp:member:: size_t line

         The line where the syntax error occurred.

      .. cpp:member:: size_t col

         The column where the syntax error occurred.

      .. cpp:member:: std::string message

         The error message.


.. cpp:struct:: type_error

   An error deriving from some semantic type checking error.

.. cpp:struct:: backend_error

   An error coming from a SAT/SMT/QBF/other backend.

.. cpp:struct:: io_error

   An error deriving from some failed I/O operation.

   .. cpp:enum:: operation

      .. cpp:enumerator:: opening
      .. cpp:enumerator:: reading
      .. cpp:enumerator:: writing

   :constructor:

      .. cpp:function:: io_error( \
               std::optional<std::string> filename, operation op, int error, \
               const char *format, Args const& ...args \
            )

         :param filename: the file over which the I/O error occurred, if any
         :param op: the type of failed I/O operation
         :param error: the value of the `errno` variable at the time of the
            error
         :param format: a format string for the error message, in `{fmt}` syntax
         :param ...args: the arguments for the format string

   :members:
      .. cpp:member:: std::optional<std::string> filename

         The file over which the I/O error occurred, if any

      .. cpp:member:: operation op

         The type of failed I/O operation

      .. cpp:member:: int error

         The value of the `errno` variable at the time of the error

      .. cpp:member:: std::string message

         The error message

      