Debugging utilities
====================

.. cpp:namespace:: black::support

.. c:macro:: black_debug(Format, ...)

   Prints a debug message that includes the current source filename and line.
   The message is only printed if debug messages are enabled, *i.e.*, if
   :cpp:any:`are_debug_msgs_enabled` returns `true`. Debug messages can be enabled with
   :cpp:any:`enable_debug_msgs` and disabled with :cpp:any:`disable_debug_msgs`.

   :param Format: a format string for the message, in `{fmt}` syntax
   :param ...: the arguments of the format string

.. cpp:function:: void enable_debug_msgs(std::string const& tag)

   Enables the reporting of messages through the :c:macro:`black_debug` macro.

   :param tag: a string shown to help the user identify the source of the 
      messages (usually, `argv[0]`).

.. cpp:function:: void disable_debug_msgs()

   Disables the reporting of messages through the :c:macro:`black_debug` macro.

.. cpp:function:: bool are_debug_msgs_enabled()

   :returns: whether debug messages reporting is enabled or not.
