add_library(mtr
  "${CMAKE_CURRENT_SOURCE_DIR}/mtrBasic.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/mtrGroup.c")

target_include_directories(
  mtr PRIVATE
  "${CUDD_UTIL_DIR}"
)
