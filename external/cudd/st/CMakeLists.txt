
add_library(st "${CMAKE_CURRENT_SOURCE_DIR}/st.c")
target_include_directories(st PRIVATE "${CUDD_UTIL_DIR}")
