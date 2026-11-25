/// @file fixed_string.hpp
/// @brief Compile-time string representation for regex patterns.
///
/// Allows string literals to be passed as non-type template
/// parameters and provides compile-time indexed character access.

#pragma once
#include <cstddef>
#include <algorithm>

namespace tinyregex::detail
{

/// @brief A fixed-size string that can be used as a non-type template paramater.
/// @tparam N The size of the string, including the null terminator.
template <std::size_t N>
class fixed_string
{
public:
    /// @brief Constructs a fixed string by copying the supplied character array.
    /// @param text The character array to copy.
    constexpr fixed_string(const char (&text)[N])
    {
        std::copy_n(text, N, data);
    }

    /// @brief Retrieves a character with compile-time bounds checking.
    /// @tparam Index The zero-based character index.
    /// @return The character at the specified index.
    template <std::size_t Index>
    constexpr char get() const
    {
        static_assert(Index < N, "fixed_string: out of bounds");
        return data[Index];
    }

    char data[N] {}; ///< Character storage.

    static constexpr std::size_t len = { N - 1 }; ///< String length, excluding null terminator.
};

}