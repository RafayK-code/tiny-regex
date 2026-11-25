/// @file parser.hpp
/// @brief Compile-time LL(1) recursive descent parser for regex patterns.
///
/// Parses a regex pattern into a type-based abstract syntax tree (AST)
/// through recursive template instantiation. The grammar is LL(1),
/// allowing parsing decisions to be made using a single character of
/// lookahead without backtracking.
///
/// The parser implements the following grammar:
///
/// @code
/// Expression     ::= Sequence ExpressionTail
/// ExpressionTail ::= '|' Expression | ε
///
/// Sequence       ::= Repeat SequenceTail
/// SequenceTail   ::= Repeat SequenceTail | ε
///
/// Repeat         ::= Atom RepeatTail
/// RepeatTail     ::= '*' | '+' | '?' | ε
///
/// Atom           ::= Literal
///                  | Escape
///                  | '.'
///                  | '^'
///                  | '$'
///                  | '(' Group
///
/// Group          ::= Expression ')'
///                  | '?' ':' Expression ')'
///
/// Escape         ::= '\' (Metacharacter | Backreference)
/// Backreference  ::= '1' | ... | '9'
/// @endcode
///
/// Each major nonterminal is implemented as a class template parameterized
/// by the pattern, current parsing position, and next available capture-group
/// index.
///
/// Each parser defines a private parse() function whose deduced return type
/// is a specialization of ParseResult. This type encapsulates the resulting
/// syntax node, the position of the next unconsumed character, and the next
/// available capture-group index.
///
/// The return type is obtained through decltype(parse()), requiring the
/// compiler to instantiate the function body for return type deduction
/// without evaluating the function itself. Compile-time branching through
/// if constexpr determines which syntax nodes and subsequent parsers are
/// instantiated.
///
/// Each parser exposes its result through three public members:
/// - type: The resulting syntax node type.
/// - next_position: The position of the next unconsumed character.
/// - next_capture_index: The next available capture-group index.
///
/// Accessing these members requires instantiation of the corresponding
/// parser, allowing parsing to proceed recursively through template
/// instantiation without executing any parsing functions.
///
/// Parse serves as the entry point, ensuring that the entire pattern has
/// been consumed and that all backreferences refer to existing capture
/// groups.

#pragma once
#include "fixed_string.hpp"
#include "syntax.hpp"
#include "inspection.hpp"

#include <cstddef>
#include <type_traits>

namespace tinyregex::detail::parser
{

/// @brief Carries the resulting syntax node and updated parser state.
/// @tparam T The resulting syntax node type.
/// @tparam NextPosition The position of the next unconsumed character.
/// @tparam NextIndex The next available capture-group index.
template <typename T, std::size_t NextPosition, std::uint32_t NextIndex>
struct ParseResult
{
    /// @brief The resulting syntax node type.
    using type = T;

    /// @brief The position of the next unconsumed character.
    static constexpr std::size_t next_position { NextPosition };

    /// @brief The next available capture-group index.
    static constexpr std::uint32_t next_capture_index { NextIndex };
};


template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseGroup;


/// @brief Parses a single atom, such as a literal, wildcard, anchor, backreference, or group.
/// @tparam Pattern The regex pattern being parsed.
/// @tparam Position The position at which parsing begins.
/// @tparam NextIndex The next available capture-group index.
template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseAtom
{
private:
    /// @brief Parses the Atom nonterminal of the regex grammar.
    static auto parse()
    {
        constexpr char current = Pattern.template get<Position>();
        static_assert(is_valid(current), "Syntax error: expected atom");

        if constexpr (current == '.')
        {
            return ParseResult<syntax::Wildcard, Position + 1, NextIndex>{};
        }
        else if constexpr (current == '^')
        {
            return ParseResult<syntax::StartAnchor, Position + 1, NextIndex>{};
        }
        else if constexpr (current == '$')
        {
            return ParseResult<syntax::EndAnchor, Position + 1, NextIndex>{};
        }
        else if constexpr (current == '(')
        {
            // Descend into the Group nonterminal.
            using Group = ParseGroup<Pattern, Position + 1, NextIndex>;
            return ParseResult<typename Group::type, Group::next_position, Group::next_capture_index>{};
        }
        else if constexpr (current == '\\')
        {
            // Read the next escaped character.
            constexpr char next = Pattern.template get<Position + 1>();
            static_assert(can_escape(next), "Syntax error: unescapable character");

            if constexpr (next >= '1' && next <= '9')
            {
                constexpr auto index = static_cast<std::uint32_t>(next - '0');
                return ParseResult<syntax::Backreference<index>, Position + 2, NextIndex>{};
            }
            else
            {
                return ParseResult<syntax::Literal<next>, Position + 2, NextIndex>{};
            }
        }
        else
        {
            return ParseResult<syntax::Literal<current>, Position + 1, NextIndex>{};
        }
    }

    /// @brief Determines whether a character can begin an atom.
    static constexpr bool is_valid(char token)
    {
        return (token != '*' &&
                token != '+' &&
                token != '?' &&
                token != '|' &&
                token != ')' &&
                token != '\0');
    }

    /// @brief Determines whether a character is supported after an escape character.
    static constexpr bool can_escape(char token)
    {
        return (token == '*'  ||
                token == '+'  ||
                token == '?'  ||
                token == '|'  ||
                token == '('  ||
                token == ')'  ||
                token == '.'  ||
                token == '^'  ||
                token == '$'  ||
                token == '\\' ||
                (token >= '1' && token <= '9'));
    }

    using Result = decltype(parse()); ///< The deduced parse result type.

public:
    using type = typename Result::type; ///< The resulting syntax node type.

    static constexpr std::size_t next_position { Result::next_position };  ///< The next unconsumed character position.
    static constexpr std::uint32_t next_capture_index { Result::next_capture_index }; ///< The next available capture-group index.
};


/// @brief Parses an atom followed by an optional quantifier (*, +, or ?).
/// @tparam Pattern The regex pattern being parsed.
/// @tparam Position The position at which parsing begins.
/// @tparam NextIndex The next available capture-group index.
template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseRepeat
{
private:
    /// @brief Parses the Repeat nonterminal of the regex grammar.
    static auto parse()
    {
        // Descend into the Atom nonterminal.
        using Atom = ParseAtom<Pattern, Position, NextIndex>;
        using Child = typename Atom::type;

        constexpr char next = Pattern.template get<Atom::next_position>();
        static_assert(is_valid<typename Atom::type>(next), "Syntax error: anchors cannot be quantified");

        // Parse the RepeatTail nonterminal.
        if constexpr (next == '*')
        {
            return ParseResult<syntax::RepeatZeroOrMore<Child>, Atom::next_position + 1, Atom::next_capture_index>{};
        }
        else if constexpr (next == '+')
        {
            return ParseResult<syntax::RepeatOneOrMore<Child>, Atom::next_position + 1, Atom::next_capture_index>{};
        }
        else if constexpr (next == '?')
        {
            return ParseResult<syntax::Optional<Child>, Atom::next_position + 1, Atom::next_capture_index>{};
        }
        else
        {
            // Epsilon production, propagate the atom's parse result unchanged.
            return ParseResult<Child, Atom::next_position, Atom::next_capture_index>{};
        }
    }

    /// @brief Determines whether a quantifier can be applied to the parsed atom.
    /// @tparam Child The syntax node representing the parsed atom.
    /// @param token The character following the atom.
    /// @return Whether the atom and following character form a valid combination.
    template <typename Child>
    static constexpr bool is_valid(char token)
    {
        // Anchors cannot be directly quantified.
        constexpr bool is_anchor = 
            std::is_same_v<Child, syntax::StartAnchor> ||
            std::is_same_v<Child, syntax::EndAnchor>;

        const bool has_quantifier =
            token == '*' ||
            token == '+' ||
            token == '?';

        return (!is_anchor || !has_quantifier);
    }

    using Result = decltype(parse()); ///< The deduced parse result type.

public:
    using type = typename Result::type; ///< The resulting syntax node type.

    static constexpr std::size_t next_position { Result::next_position }; ///< The next unconsumed character position.
    static constexpr std::uint32_t next_capture_index { Result::next_capture_index }; ///< The next available capture-group index.
};


/// @brief Prepends a syntax node to an expression to form a sequence.
/// @tparam T The syntax node to prepend.
/// @tparam Child The expression to prepend to.
template <typename T, typename Child>
class SequencePrepend
{
public:
    using type = syntax::Sequence<T, Child>; ///< The resulting sequence type.
};


/// @brief Prepends a syntax node to an existing sequence without introducing nesting.
/// @tparam T The syntax node to prepend.
/// @tparam Children The existing sequence's child expressions.
template <typename T, typename... Children>
class SequencePrepend<T, syntax::Sequence<Children...>>
{
public:
    using type = syntax::Sequence<T, Children...>; ///< The resulting sequence type.
};


/// @brief Parses a sequence of one or more optionally quantified atoms.
/// @tparam Pattern The regex pattern being parsed.
/// @tparam Position The position at which parsing begins.
/// @tparam NextIndex The next available capture-group index.
template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseSequence
{
private:
    /// @brief Parses the Sequence nonterminal of the regex grammar.
    static auto parse()
    {
        // Descend into the Repeat nonterminal.
        using Repeat = ParseRepeat<Pattern, Position, NextIndex>;
        constexpr char next = Pattern.template get<Repeat::next_position>();

        // Parse the SequenceTail nonterminal.
        if constexpr (next == '\0' || next == '|' || next == ')')
        {
            // Epsilon production, propagate the Repeat parse result unchanged.
            return ParseResult<typename Repeat::type, Repeat::next_position, Repeat::next_capture_index>{};
        }
        else
        {
            // Recurisve production, parse another Sequence, then flatten the result.
            using Rest = ParseSequence<Pattern, Repeat::next_position, Repeat::next_capture_index>;
            using Sequence = SequencePrepend<typename Repeat::type, typename Rest::type>;

            return ParseResult<typename Sequence::type, Rest::next_position, Rest::next_capture_index>{};
        }
    }

    using Result = decltype(parse()); ///< The deduced parse result type.

public:
    using type = typename Result::type; ///< The resulting syntax node type.

    static constexpr std::size_t next_position { Result::next_position }; ///< The next unconsumed character position.
    static constexpr std::uint32_t next_capture_index { Result::next_capture_index }; ///< The next available capture-group index.
};


/// @brief Parses one or more sequences separated by the alternation operator (|).
/// @tparam Pattern The regex pattern being parsed.
/// @tparam Position The position at which parsing begins.
/// @tparam NextIndex The next available capture-group index.
template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseExpression
{
private:
    /// @brief Parses the Expression nonterminal of the regex grammar.
    static auto parse()
    {
        // Descend into the Sequence nonterminal.
        using Sequence = ParseSequence<Pattern, Position, NextIndex>;
        constexpr char next = Pattern.template get<Sequence::next_position>();

        // Parse the ExpressionTail nonterminal.
        if constexpr (next == '|')
        {
            // Recursive production, descend into Expression, and combine both nodes into a union.
            using Expression = ParseExpression<Pattern, Sequence::next_position + 1, Sequence::next_capture_index>;
            return ParseResult<syntax::Union<typename Sequence::type, typename Expression::type>, Expression::next_position, Expression::next_capture_index>{};
        }
        else
        {
            // Epsilon production, propagate the Sequence parse result unchanged.
            return ParseResult<typename Sequence::type, Sequence::next_position, Sequence::next_capture_index>{};
        }
    }

    using Result = decltype(parse()); ///< The deduced parse result type.

public:
    using type = typename Result::type; ///< The resulting syntax node type.

    static constexpr std::size_t next_position { Result::next_position }; ///< The next unconsumed character position.
    static constexpr std::uint32_t next_capture_index { Result::next_capture_index }; ///< The next available capture-group index.
};


/// @brief Parses a capturing or non-capturing group.
/// @tparam Pattern The regex pattern being parsed.
/// @tparam Position The position immediately after the opening parenthesis.
/// @tparam NextIndex The next available capture-group index.
template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseGroup
{
private:
    /// @brief Parses the Group nonterminal of the regex grammar.
    static auto parse()
    {
        constexpr char current = Pattern.template get<Position>();
        if constexpr (current == '?')
        {
            // Non-capturing group production, consume the '?' and ':' tokens
            // before descending into the Expression nonterminal.
            static_assert(Pattern.template get<Position + 1>() == ':', "Syntax error: Expected ':' after '(?'");
            using Expression = ParseExpression<Pattern, Position + 2, NextIndex>;

            // Validate and consume the closing parenthesis.
            static_assert(Pattern.template get<Expression::next_position>() == ')', "Expected ')'");
            return ParseResult<syntax::NonCaptureGroup<typename Expression::type>, Expression::next_position + 1, Expression::next_capture_index>{};
        }
        else
        {
            // Capturing group production: reserve the current capture-group
            // index before descending into the Expression nonterminal.
            using Expression = ParseExpression<Pattern, Position, NextIndex + 1>;

            // Validate and consume the closing parenthesis.
            static_assert(Pattern.template get<Expression::next_position>() == ')', "Syntax error: Expected ')'");
            return ParseResult<syntax::CaptureGroup<NextIndex, typename Expression::type>, Expression::next_position + 1, Expression::next_capture_index>{};
        }
    }

    using Result = decltype(parse()); ///< The deduced parse result type.

public:
    using type = typename Result::type; ///< The resulting syntax node type.

    static constexpr std::size_t next_position { Result::next_position }; ///< The next unconsumed character position.
    static constexpr std::uint32_t next_capture_index { Result::next_capture_index }; ///< The next available capture-group index.
};

// Post-parsing passes
//
// Once the syntax tree has been constructed, additional passes can be
// performed to validate, analyze, or transform the tree.
//
// Currently, the only post-parsing pass validates that all backreferences
// refer to existing capture groups. Future passes may perform syntax tree
// optimizations or analyze the tree to select a more efficient matching
// strategy, such as NFA-based matching for compatible patterns.

/// @brief Predicate for backreference validation, accepting non-backreference nodes by default.
/// @tparam Node The syntax node to validate.
/// @tparam CaptureCount The total number of capturing groups.
template <typename Node, std::uint32_t CaptureCount>
class ValidBackreference : public std::true_type {};


/// @brief Determines whether a backreference refers to an existing capture group.
/// @tparam Index The 1-based index of the referenced capture group.
/// @tparam CaptureCount The total number of capturing groups.
template <std::uint32_t Index, std::uint32_t CaptureCount>
class ValidBackreference<syntax::Backreference<Index>, CaptureCount> : public std::bool_constant<(Index >= 1 && Index <= CaptureCount)> {};


/// @brief Binds the capture-group count to the backreference validation predicate.
/// @tparam CaptureCount The total number of capturing groups.
template <std::uint32_t CaptureCount>
class BackreferencePredicate
{
public:
    /// @brief Applies the backreference validation predicate to a syntax node.
    /// @tparam Node The syntax node to validate.
    template <typename Node>
    using Apply = ValidBackreference<Node, CaptureCount>;
};

/// @brief Entry point for compile-time regex parsing and validation.
/// Ensures the entire pattern is consumed and all backreferences
/// refer to existing capture groups.
/// @tparam Pattern The regex pattern being parsed.
template <fixed_string Pattern>
class Parse
{
private:
    using Parsed = ParseExpression<Pattern, 0, 1>; ///< The top-level parse result.

    static_assert(Parsed::next_position == Pattern.len, "Syntax error: unexpected characters after expression");

    static_assert(inspect::Check<typename Parsed::type, BackreferencePredicate<Parsed::next_capture_index - 1>::template Apply>::value,
        "Syntax error: Invalid backreference");

public:
    /// @brief The root node type of the parsed AST
    using type = typename Parsed::type;
    
    /// @brief The total number of capturing groups in the pattern.
    static constexpr std::uint32_t capture_group_count { Parsed::next_capture_index - 1 };
};

}