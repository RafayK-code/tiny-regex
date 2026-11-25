/// @file matcher.hpp
/// @brief Regex-directed backtracking matcher for compiled regex patterns.
///
/// Implements a regex-directed matching engine that traverses a type-based
/// abstract syntax tree (AST), attempting to match its syntax nodes against
/// an input string. Alternatives are attempted from left to right, and
/// quantifiers are greedy by default. When a matching path fails, the engine
/// backtracks to explore remaining alternatives.
///
/// Matching is implemented using continuation-passing style (CPS).
/// Each syntax node is associated with a specialization of Match<T, K>,
/// where T is the syntax node type and K is the continuation type.
/// The continuation represents the remaining matching operations to be
/// performed after the current node has matched.
///
/// For example, a sequence of three syntax nodes A, B, and C is represented
/// by the following continuation chain:
///
/// @code
/// Match<A, Match<B, Match<C, Success>>>
/// @endcode
///
/// Each matcher receives the input string, the current input position,
/// and the shared capture state. Upon successfully matching its node,
/// the matcher invokes its continuation with the updated input position.
///
/// A matcher only reports success if both its own matching operation and
/// its continuation succeed. Consequently, a failure in a continuation
/// can cause an earlier matcher to explore another matching path.
///
/// Input positions are passed by value, allowing them to be implicitly
/// restored during backtracking. Capture state is shared by reference
/// and is explicitly restored by the matchers that modify it whenever
/// their matching path fails.
///
/// The structure of the matcher and its continuation types are determined
/// at compile time through template instantiation. Matching itself is
/// performed at runtime against the supplied input string.

#pragma once
#include "syntax.hpp"

#include <array>
#include <string_view>

namespace tinyregex::detail::matcher
{

/// @brief Determines whether an input position refers to a valid character.
/// @param text The original input string.
/// @param idx The position to validate.
/// @return Whether the position is within the bounds of the input.
constexpr bool in_bounds(std::string_view text, std::size_t idx)
{
    return idx < text.size();
}


/// @brief Stores the matching state of a capturing group.
/// The captured text is represented by a half-open interval [start, end)
/// within the original input string.
struct Capture
{
    std::size_t start {}; ///< The starting position of the captured text.
    std::size_t end {}; ///< The position immediately after the captured text.
    bool matched { false }; ///< Whether the group has completed a successful match.
};


/// @brief Terminal continuation that reports a successful matching path.
class Success
{
public:
    /// @brief Terminates the continuation chain successfully.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_cast<void>(text);
        static_cast<void>(idx);
        static_cast<void>(captures);
        return true;
    }
};


/// @brief Matches a syntax node and invokes its continuation upon success.
///
/// Each specialization implements the matching behaviour of a particular
/// syntax node and exposes a common callable interface.
///
/// @tparam T The syntax node to match.
/// @tparam K The continuation representing subsequent matching operations.
template <typename T, typename K>
class Match;


/// @brief Matches a literal character.
/// @tparam C The character to match.
/// @tparam K The continuation type.
template <char C, typename K>
class Match<syntax::Literal<C>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Attempts to match the literal at the current input position.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (!in_bounds(text, idx) || text[idx] != C)
        {
            return false;
        }

        return next(text, idx + 1, captures);
    }
};


/// @brief Matches any single character except newline.
/// @tparam K The continuation type.
template <typename K>
class Match<syntax::Wildcard, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Attempts to match a wildcard at the current input position.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (!in_bounds(text, idx) || text[idx] == '\n')
        {
            return false;
        }

        return next(text, idx + 1, captures);
    }
};


/// @brief Asserts the beginning of the original input without consuming a character.
/// @tparam K The continuation type.
template <typename K>
class Match<syntax::StartAnchor, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Checks whether the current input position is the beginning of the input.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (idx != 0)
        {
            return false;
        }

        return next(text, idx, captures);
    };
};


/// @brief Asserts the end of the original input without consuming a character.
/// @tparam K The continuation type.
template <typename K>
class Match<syntax::EndAnchor, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Checks whether the current input position is the end of the input.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (idx != text.size())
        {
            return false;
        }

        return next(text, idx, captures);
    }
};


/// @brief Constructs a continuation chain from a sequence of syntax nodes.
/// @tparam T The sequence of syntax nodes.
/// @tparam K The continuation following the sequence.
template <typename T, typename K>
class BuildSequenceMatch;


/// @brief Terminates sequence construction with the supplied continuation.
/// @tparam K The continuation following the sequence.
template <typename K>
class BuildSequenceMatch<syntax::Sequence<>, K>
{
public:
    using type = K; ///< The terminal continuation type.

    /// @brief Returns the continuation following the sequence.
    /// @param next The terminal continuation.
    static type make(const K& next)
    {
        return next;
    }
};


/// @brief Recursively constructs a continuation chain from a non-empty sequence.
/// @tparam First The first syntax node in the sequence.
/// @tparam Rest The remaining syntax nodes.
/// @tparam K The continuation following the sequence.
template <typename First, typename... Rest, typename K>
class BuildSequenceMatch<syntax::Sequence<First, Rest...>, K>
{
private:
    using Tail = BuildSequenceMatch<syntax::Sequence<Rest...>, K>; ///< Builder for the remaining sequence.

public:
    using type = Match<First, typename Tail::type>; ///< The resulting continuation chain type.

    /// @brief Constructs the continuation chain recursively.
    /// @param next The continuation following the sequence.
    /// @return The constructed continuation chain.
    static type make(const K& next)
    {
        return type{Tail::make(next)};
    }
};


/// @brief Matches a sequence of syntax nodes in order.
/// @tparam Children The syntax nodes to match sequentially.
/// @tparam K The continuation type.
template <typename... Children, typename K>
class Match<syntax::Sequence<Children...>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Matches the sequence through its constructed continuation chain.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        auto chain = BuildSequenceMatch<syntax::Sequence<Children...>, K>::make(next);
        return chain(text, idx, captures);
    }
};


/// @brief Matches either branch of an alternation, prioritizing the left branch.
/// @tparam Left The left-hand side expression.
/// @tparam Right The right-hand side expression.
/// @tparam K The continuation type.
template <typename Left, typename Right, typename K>
class Match<syntax::Union<Left, Right>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Attempts each alternative in left-to-right order.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (Match<Left, K>{next}(text, idx, captures))
        {
            return true;
        }

        return Match<Right, K>{next}(text, idx, captures);
    }
};


/// @brief Continuation that attempts another repetition after a child matches.
/// Prevents unbounded repetition of expressions that can match without consuming input.
/// @tparam Child The expression being repeated.
/// @tparam K The continuation following the repetition.
template <typename Child, typename K>
class RepeatContinue
{
public:
    std::size_t start_idx {}; ///< The input position before the current repetition.
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Continues repetition only if the previous match consumed input.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        // Reject zero-width repetitions to prevent infinite recursion.
        if (idx == start_idx)
        {
            return false;
        }

        return Match<syntax::RepeatZeroOrMore<Child>, K>{next}(text, idx, captures);
    }
};


/// @brief Greedily matches zero or more occurrences of an expression.
/// @tparam Child The expression to repeat.
/// @tparam K The continuation type.
template <typename Child, typename K>
class Match<syntax::RepeatZeroOrMore<Child>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Attempts another repetition before trying the continuation.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        // Greedily attempt another repetition.
        auto repeat = Match<Child, RepeatContinue<Child, K>>{ RepeatContinue<Child, K>{idx, next} };
        if (repeat(text, idx, captures))
        {
            return true;
        }

        // Backtrack by attempting the continuation without another repetition.
        return next(text, idx, captures);
    }
};


/// @brief Greedily matches one or more occurrences of an expression.
/// @tparam Child The expression to repeat.
/// @tparam K The continuation type.
template <typename Child, typename K>
class Match<syntax::RepeatOneOrMore<Child>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Requires an initial match before attempting further repetitions.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        // Match one required occurrence followed by zero or more occurrences.
        using Rest = Match<syntax::RepeatZeroOrMore<Child>, K>;
        if (Match<Child, Rest>{Rest{next}}(text, idx, captures))
        {
            return true;
        }

        return false;
    }
};


/// @brief Greedily matches zero or one occurrence of an expression.
/// @tparam Child The optional expression.
/// @tparam K The continuation type.
template <typename Child, typename K>
class Match<syntax::Optional<Child>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Attempts to match the child before skipping it.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (Match<Child, K>{next}(text, idx, captures))
        {
            return true;
        }

        // Backtrack by skipping the optional expression.
        return next(text, idx, captures);
    }
};

/// @brief Continuation that finalizes a capturing group's matched interval.
/// Makes the captured text available to subsequent matching operations,
/// including backreferences. Restores the previous capture state if the
/// continuation fails.
/// @tparam Index The 1-based capture-group index.
/// @tparam K The continuation following the capturing group.
template <std::uint32_t Index, typename K>
class CaptureEnd
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Finalizes the capture and attempts the continuation.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_assert(Index >= 1 && Index <= N);
        auto& capture = captures[Index - 1];
        const auto prev = capture;

        // Finalize the captured interval before invoking the continuation.
        capture.end = idx;
        capture.matched = true;
        if (next(text, idx, captures))
        {
            return true;
        }

        // Restore the previous capture state upon backtracking.
        capture = prev;
        return false;
    }
};


/// @brief Matches a capturing group and records its matched input interval.
/// @tparam Index The 1-based capture-group index.
/// @tparam Child The expression enclosed by the group.
/// @tparam K The continuation type.
template <std::uint32_t Index, typename Child, typename K>
class Match<syntax::CaptureGroup<Index, Child>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Initializes the capture and attempts to match the enclosed expression.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_assert(Index >= 1 && Index <= N);
        auto& capture = captures[Index - 1];
        const auto prev = capture;

        // Initialize the capture at the current input position.
        capture.start = idx;
        capture.end = idx;
        capture.matched = false;

        // Match the enclosed expression with a continuation that finalizes
        // the capture before subsequent matching operations.
        auto group = Match<Child, CaptureEnd<Index, K>>{ CaptureEnd<Index, K>{next} };
        if (group(text, idx, captures))
        {
            return true;
        }

        // Restore the previous capture state upon backtracking.
        capture = prev;
        return false;
    }
};


/// @brief Matches a grouped expression without creating a capture.
/// @tparam Child The expression enclosed by the group.
/// @tparam K The continuation type.
template <typename Child, typename K>
class Match<syntax::NonCaptureGroup<Child>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Matches the enclosed expression directly.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        return Match<Child, K>{next}(text, idx, captures);
    }
};


/// @brief Matches the previously captured text of a referenced group.
/// @tparam Index The 1-based index of the referenced capture group.
/// @tparam K The continuation type.
template <std::uint32_t Index, typename K>
class Match<syntax::Backreference<Index>, K>
{
public:
    K next; ///< The continuation to invoke upon matching the node.

    /// @brief Attempts to match the referenced capture at the current input position.
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_assert(Index >= 1 && Index <= N);
        const auto& group = captures[Index  - 1];

        // Ensure the referenced group contains a valid captured interval.
        if (!group.matched || group.start > group.end || group.end > text.size())
        {
            return false;
        }

        // Ensure sufficient input remains to match the captured text.
        const std::size_t length = group.end - group.start;
        if (idx > text.size() || length > text.size() - idx)
        {
            return false;
        }

        // Compare the captured text against the input at the current position.
        if (text.substr(group.start, length) != text.substr(idx, length))
        {
            return false;
        }

        return next(text, idx + length, captures);
    }
};

}