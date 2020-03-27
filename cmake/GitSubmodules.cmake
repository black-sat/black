#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2020 Nicola Gigante
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

find_package(Git QUIET)
option(GIT_UPDATE_SUBMODULES "Automatically update git submodules" ON)

function(update_git_submodules)
  cmake_parse_arguments(PARSE_ARGV 0 SUBMOD_ "FATAL" "RESULT" "")

  if(GIT_FOUND AND 
     EXISTS "${PROJECT_SOURCE_DIR}/.git" AND GIT_UPDATE_SUBMODULES
  )
    message(STATUS "Git submodule update")
    execute_process(COMMAND 
      ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_QUIET
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
      if(NOT "${SUBM_RESULT}" STREQUAL "")
        set(${SUBMOD_RESULT} ${GIT_SUBMOD_RESULT} PARENT_SCOPE)
      endif()
    if(SUBM_FATAL)
      if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR 
        "git submodule update --init failed with ${GIT_SUBMOD_RESULT} error code")
      endif()
    endif()
  endif()
endfunction()