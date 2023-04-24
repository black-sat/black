add_library(epd "${CMAKE_CURRENT_SOURCE_DIR}/epd.c")
target_include_directories(epd PRIVATE "${CUDD_UTIL_DIR}")
