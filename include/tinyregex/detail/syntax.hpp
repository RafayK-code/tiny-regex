/// @file syntax.hpp
/// @brief Type-based representation of parsed regex patterns.
///
/// Each node is an empty tag type that carries structure as template
/// parameters e.g. @c Literal<'a'> or @c Sequence<Literal<'a'>, Wildcard>.

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


/// @brief Represents concatenation, such as 'cat', matching child expressions in order.
/// @tparam Children Expressions to match sequentially.
template <typename... Children>
class Sequence {};


/// @brief Represents union or alteration, such as 'a|b', matching either the left expression or right.
/// @tparam Left The left hand side expression of the union, attempted first.
/// @tparam Right The right hand side expression of the union, attempted after the left.
template <typename Left, typename Right>
class Union {};


/// @brief Represents Kleene star, such as a*, matching an expression zero or more times.
/// @tparam Child The expression to be repeated.
template <typename Child>
class RepeatZeroOrMore {};


/// @brief Represents Kleene plus, such as a+, matching an expression one or more times.
/// @tparam Child The expression to be repeated.
template <typename Child>
class RepeatOneOrMore {};


/// @brief Represents the optional quantifier, such as a?, matching an expression zero or one times.
/// @tparam Child The expression to be quantified.
template <typename Child>
class Optional {};


/// @brief Represents a capturing group, such as (abc), capturing the text matched by its child expression.
/// @tparam Index The 1-based capture-group number.
/// @tparam Child The expression enclosed by the group.
template <std::uint32_t Index, typename Child> 
class CaptureGroup {};


/// @brief Represents a non-capturing group, such as (?:abc), grouping an expression without capturing its matched text.
/// @tparam Child The expression enclosed by the group.
template <typename Child>
class NonCaptureGroup {};

}