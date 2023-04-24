include_directories("${CUDD_EPD_DIR}")
include_directories("${CUDD_MTR_DIR}")
include_directories("${CUDD_ST_DIR}")
include_directories("${CUDD_UTIL_DIR}")

file(GLOB CUDD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/cudd*.c")  # unsafe!
add_library(cudd ${CUDD_SRC})
target_link_libraries(cudd epd mtr st cudd_util obj)
