cmake_minimum_required(VERSION 3.25)
project(tinyregex VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tinyregex INTERFACE)
add_library(tinyregex::tinyregex ALIAS tinyregex)

target_include_directories(tinyregex INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(tinyregex INTERFACE cxx_std_23)

include(CTest)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()