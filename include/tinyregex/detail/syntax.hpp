/// @file syntax.hpp
/// @brief Type-based representation of parsed regex patterns.
///
/// Each node is an empty tag type that carries structure as template
/// parametersm e.g. @c Literal<'a'> or @c Sequence<Literal<'a'>, Wildcard>.

#pragma once
#include <cstdint>

namespace tinyregex::detail::syntax
{

/// @brief Matches one occurrence of C, including escaped metacharacters.
/// @tparam C The literal character represented by this node.
template <char C>
class Literal {};

/// @brief Represents \1 through \9, matching the referenced group's captured text.
/// @tparam Index The 1-based capture-group number.
template <std::uint32_t Index>
class Backreference {};

/// @brief Represents `.`, matching any single character except newline.
class Wildcard {};

/// @brief Represents `^`, asserting the beginning of the original input.
class StartAnchor {};

/// @brief Represents `$`, asserting the end of the original input.
class EndAnchor {};

/// @brief Represents concatenation, such as 'cat', matching children in order.
/// @tparam Children 
template <typename... Children>
class Sequence {};

template <typename Left, typename Right>
class Union {};

template <typename Child>
class RepeatZeroOrMore {};

template <typename Child>
class RepeatOneOrMore {};

template <typename Child>
class Optional {};

template <std::uint32_t Index, typename Child> 
class CaptureGroup {};

template <typename Child>
class NonCaptureGroup {};

}