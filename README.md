# TinyRegex

A header-only C++ 23 compile-time regex library.

Patterns are parsed into a type-based syntax tree during compilation. Template specializations implement the matching operations, producing code specialized for each pattern. Matching uses greedy, regex-directed backtracking.

TinyRegex is a learning project focused on regex engines and metaprogramming.

## Usage

TinyRegex provides a class and free function that search for a match anywhere in the input. TinyRegex does not provide a separate match function, instead, anchors can be used to achieve the same functionality.

Both interfaces accept `std::string_view` and return a boolean.

```cpp
#include <tinyregex/tinyregex.hpp>

// free function
bool found = tinyregex::search<"cat">("the cat sat"); // true

// object based
tinyregex::Regex<"cat"> regex;
bool first = regex.search("concatenate"); // true
bool second = regex.search("dog"); // false
```

## Supported syntax

| Syntax | Meaning |
| --- | --- |
| `a` | Literal character |
| `ab` | Concatenation |
| `a\|b` | Alternation |
| `a*` | Zero or more occurrences |
| `a+` | One or more occurrences |
| `a?` | Zero or one occurrence |
| `.` | Any single character except `\n` |
| `(ab)` | Capturing group |
| `(?:ab)` | Noncapturing group |
| `\1`–`\9` | Match the text captured by the referenced group |
| `^` | Start of the original input |
| `$` | End of the original input |
| `\.` | Escaped metacharacter, matched literally |

Repetition is greedy. Capturing groups are numbered from 1 in opening-parenthesis order; noncapturing groups do not receive a number.

A backreference to a group that did not participate fails that matching path. A group that successfully captured an empty string can be referenced and matches zero characters.

```cpp
tinyregex::search<R"(^(a+)\1$)">("aaaa"); // true: "aa" followed by "aa"
tinyregex::search<R"(^(a)?b\1$)">("b"); // false: group 1 did not participate
tinyregex::search<R"(^(a*)b\1$)">("b"); // true: group 1 captured an empty string
```

Invalid patterns and references to nonexistent capture groups are rejected during C++ compilation. Empty patterns, empty groups, and empty alternatives are currently rejected.

Character classes, bounded repetition, lazy quantifiers, and lookaround are outside the current feature set. Matching is character-by-character over `char` input, without Unicode decoding. Backtracking can take exponential time on ambiguous patterns.

## Build

Requires a C++23 compiler and CMake 3.25 or later.

To use the library from a CMake project:

```cmake
add_subdirectory(path/to/tinyregex)
target_link_libraries(your_target PRIVATE tinyregex::tinyregex)
```

## Tests

Configure, build the runtime tests, then run the suite:

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
```

The tests cover three layers:

- `tests/parser/`: expected syntax trees and patterns that must fail compilation.
- `tests/matcher/`: matching individual syntax types and their combinations.
- `tests/regex/`: public API searches, backtracking, edge cases, and long inputs.

Parser tests invoke the compiler through a CMake script. Runtime tests use doctest. The current parser test runner uses GCC/Clang-style compiler flags.