cmake_minimum_required(VERSION 3.25)
project(tinyregex VERSION 0.1.0 LANGUAGES CXX)

add_library(tinyregex INTERFACE)
add_library(tinyregex::tinyregex ALIAS tinyregex)

target_include_directories(tinyregex INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(tinyregex INTERFACE cxx_std_23)

option(TINYREGEX_BUILD_TESTS "Build TinyRegex tests" ${PROJECT_IS_TOP_LEVEL})

if(TINYREGEX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()