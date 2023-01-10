cmake_minimum_required(VERSION 3.1.1)
project(test_jxl)

# Disable JXL features we do not need
set(BUILD_TESTING           false CACHE BOOL "")
set(JPEGXL_ENABLE_DEVTOOLS  false CACHE BOOL "")
set(JPEGXL_ENABLE_TOOLS     false CACHE BOOL "")
set(JPEGXL_ENABLE_DOXYGEN   false CACHE BOOL "")
set(JPEGXL_ENABLE_MANPAGES  false CACHE BOOL "")
set(JPEGXL_ENABLE_BENCHMARK false CACHE BOOL "")
set(JPEGXL_ENABLE_EXAMPLES  false CACHE BOOL "")
set(JPEGXL_ENABLE_VIEWERS   false CACHE BOOL "")

add_subdirectory(libjxl)

add_executable(
    test_jxl
    main.cpp
    jxl_helpers.cpp
    pfm.cpp
)

target_link_libraries(
    test_jxl PRIVATE
    jxl jxl_threads
)
