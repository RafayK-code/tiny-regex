add_executable(tinyregex_matcher_tests
    main.cpp
    atoms.cpp
    sequences.cpp
    quantifiers.cpp
    unions.cpp
    backreferences.cpp
)

target_include_directories(tinyregex_matcher_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tinyregex_matcher_tests
    PRIVATE tinyregex::tinyregex
    PRIVATE doctest
)

add_test(NAME matcher-unit-tests COMMAND tinyregex_matcher_tests)
