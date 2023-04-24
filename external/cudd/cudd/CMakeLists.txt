file(GLOB CUDD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/cudd*.c")  # unsafe!
add_library(cudd ${CUDD_SRC})
target_link_libraries(cudd PUBLIC epd mtr st cudd_util obj)
target_include_directories(
    cudd PRIVATE
    "${CUDD_EPD_DIR}"
    "${CUDD_MTR_DIR}"
    "${CUDD_ST_DIR}"
    "${CUDD_UTIL_DIR}"
)
