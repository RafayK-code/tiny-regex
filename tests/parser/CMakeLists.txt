add_test(
    NAME parser-compile-tests
    COMMAND "${CMAKE_COMMAND}"
        "-DCOMPILER=${CMAKE_CXX_COMPILER}"
        "-DSTANDARD=c++23"
        "-DINCLUDE_DIR=${PROJECT_SOURCE_DIR}/include"
        "-DPASS_DIR=${CMAKE_CURRENT_SOURCE_DIR}/pass"
        "-DFAIL_DIR=${CMAKE_CURRENT_SOURCE_DIR}/fail"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/run_compile_test.cmake"
)