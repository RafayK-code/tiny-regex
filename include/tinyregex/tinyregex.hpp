/// @file regex.hpp
/// @brief Public interface for compile-time regex compilation and runtime matching.
///
/// Provides a Regex class and a convenience search function for matching
/// compile-time regex patterns against runtime input strings. Patterns are
/// parsed and validated at compile time, and their resulting syntax trees
/// are used to construct specialized matchers.
/// @code
/// tinyregex::Regex<"a+b"> regex;
/// bool matched = regex.search("aaab");
///
/// bool matched = tinyregex::search<"a+b">("aaab");
/// @endcode

#pragma once
#include "detail/fixed_string.hpp"
#include "detail/parser.hpp"
#include "detail/matcher.hpp"

#include <string_view>

namespace tinyregex
{

/// @brief Represents a regex pattern compiled at compile time.
/// @tparam Pattern The regex pattern to compile.
template <detail::fixed_string Pattern>
class Regex
{
private:
    using Parsed = detail::parser::Parse<Pattern>; ///< The compile-time parse result.
    using AST = typename Parsed::type; ///< The root node type of the parsed syntax tree.

public:
    /// @brief Searches the input for an occurrence of the regex pattern.
    ///
    /// Attempts matching at successive input positions from left to right.
    /// The entire input need not match the pattern.
    ///
    /// @param text The input string to search.
    /// @return True if a match is found, false otherwise.
    bool search(std::string_view text)
    {
        std::array<detail::matcher::Capture, Parsed::capture_group_count> captures{};

        static constexpr auto matcher = detail::matcher::Match<AST, detail::matcher::Success>{ detail::matcher::Success{} };

        // Include the end-of-input position to allow zero-width matches.
        for (size_t i = 0; i <= text.size(); i++)
        {
            if (matcher(text, i, captures))
            {
                return true;
            }
        }

        return false;
    }
};

/// @brief Convenience function for searching an input using a compile-time regex pattern.
/// @tparam Pattern The regex pattern to compile.
/// @param text The input string to search.
/// @return True if a match is found, false otherwise.
template <detail::fixed_string Pattern>
inline bool search(std::string_view text)
{
    return Regex<Pattern>{}.search(text);
}

}