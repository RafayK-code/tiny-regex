# Compile-time parser tests.
#
# Required -D arguments:
#   COMPILER, STANDARD, INCLUDE_DIR, PASS_DIR, FAIL_DIR
#
# Compiler flags target GCC and Clang.

foreach(argument IN ITEMS COMPILER STANDARD INCLUDE_DIR PASS_DIR FAIL_DIR)
    if(NOT DEFINED ${argument} OR "${${argument}}" STREQUAL "")
        message(FATAL_ERROR "Missing required argument: ${argument}")
    endif()
endforeach()

function(compile_source source)
    execute_process(
        COMMAND "${COMPILER}"
            "-std=${STANDARD}"
            -fsyntax-only
            "-I${INCLUDE_DIR}"
            "${source}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
        TIMEOUT 60
    )

    if(NOT "${result}" MATCHES "^[0-9]+$")
        message(FATAL_ERROR
            "Compiler could not complete for ${source}:\n"
            "${result}\n${stdout}\n${stderr}"
        )
    endif()

    set(compile_status "${result}" PARENT_SCOPE)
    set(compile_output "${stdout}\n${stderr}" PARENT_SCOPE)
endfunction()

file(GLOB pass_files "${PASS_DIR}/*.cpp")
file(GLOB fail_files "${FAIL_DIR}/*.cpp")

list(SORT pass_files)
list(SORT fail_files)

set(passed 0)
set(failed 0)

foreach(source IN LISTS pass_files)
    compile_source("${source}")

    if("${compile_status}" STREQUAL "0")
        math(EXPR passed "${passed} + 1")
    else()
        math(EXPR failed "${failed} + 1")
        message(STATUS
            "Expected compilation to succeed: ${source}\n${compile_output}"
        )
    endif()
endforeach()

foreach(source IN LISTS fail_files)
    compile_source("${source}")

    if("${compile_status}" STREQUAL "0")
        math(EXPR failed "${failed} + 1")
        message(STATUS "Expected compilation to fail: ${source}")
    else()
        math(EXPR passed "${passed} + 1")
    endif()
endforeach()

if(failed GREATER 0)
    message(FATAL_ERROR
        "Parser compile tests: ${passed} passed, ${failed} failed"
    )
endif()

if(passed EQUAL 0)
    message(FATAL_ERROR "No parser compile tests were found")
endif()

message(STATUS "Parser compile tests: ${passed} passed")