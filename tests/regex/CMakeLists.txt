add_executable(tinyregex_regex_tests
    main.cpp
    search.cpp
    backtracking.cpp
    edge_cases.cpp
    long_inputs.cpp
)

target_link_libraries(tinyregex_regex_tests
    PRIVATE tinyregex::tinyregex
    PRIVATE doctest
)

add_test(NAME regex-tests COMMAND tinyregex_regex_tests)
