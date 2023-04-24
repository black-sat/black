
add_library(obj "${CMAKE_CURRENT_SOURCE_DIR}/cuddObj.cc")

target_include_directories(
    obj PRIVATE 
    "${CUDD_EPD_DIR}"
    "${CUDD_MTR_DIR}"
    "${CUDD_ST_DIR}"
    "${CUDD_UTIL_DIR}"
    "${CUDD_SOURCE_DIR}"
)