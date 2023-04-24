include_directories("${CUDD_UTIL_DIR}")

add_library(mtr
  "${CMAKE_CURRENT_SOURCE_DIR}/mtrBasic.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/mtrGroup.c")
