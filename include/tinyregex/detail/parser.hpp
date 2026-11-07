#pragma once
#include "fixed_string.hpp"
#include "syntax.hpp"
#include "inspection.hpp"

#include <cstddef>
#include <type_traits>

namespace tinyregex::detail::parser
{

template <typename T, std::size_t NextPosition, std::uint32_t NextIndex>
struct ParseResult
{
    using type = T;
    static constexpr std::size_t next_position = NextPosition;
    static constexpr std::uint32_t next_capture_index = NextIndex;
};

template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseGroup;

template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseAtom
{
private:
    static auto select_type()
    {
        constexpr char current = Pattern.template get<Position>();
        static_assert(is_valid(current), "Syntax error: expected atom");

        if constexpr (current == '.')
        {
            return ParseResult<syntax::Wildcard, Position + 1, NextIndex>{};
        }
        else if constexpr (current == '(')
        {
            using Group = ParseGroup<Pattern, Position + 1, NextIndex>;
            return ParseResult<typename Group::type, Group::next_position, Group::next_capture_index>{};
        }
        else if constexpr (current == '^')
        {
            return ParseResult<syntax::StartAnchor, Position + 1, NextIndex>{};
        }
        else if constexpr (current == '$')
        {
            return ParseResult<syntax::EndAnchor, Position + 1, NextIndex>{};
        }
        else if constexpr (current == '\\')
        {
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

    static constexpr bool is_valid(char token)
    {
        return (token != '*' &&
                token != '+' &&
                token != '?' &&
                token != '|' &&
                token != ')' &&
                token != '\0');
    }

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

    using Result = decltype(select_type());

public:
    using type = typename Result::type;

    static constexpr std::size_t next_position = Result::next_position;
    static constexpr std::uint32_t next_capture_index = Result::next_capture_index;
};

template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseRepeat
{
private:
    static auto select_type()
    {
        using Atom = ParseAtom<Pattern, Position, NextIndex>;
        using Child = typename Atom::type;

        constexpr char next = Pattern.template get<Atom::next_position>();
        static_assert(is_valid<typename Atom::type>(next), "Syntax error: anchors cannot be quantified");

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
            return ParseResult<Child, Atom::next_position, Atom::next_capture_index>{};
        }
    }

    template <typename Child>
    static constexpr bool is_valid(char token)
    {
        constexpr bool is_anchor = 
            std::is_same_v<Child, syntax::StartAnchor> ||
            std::is_same_v<Child, syntax::EndAnchor>;

        const bool has_quantifier =
            token == '*' ||
            token == '+' ||
            token == '?';

        return (!is_anchor || !has_quantifier);
    }

    using Result = decltype(select_type());

public:
    using type = typename Result::type;

    static constexpr std::size_t next_position = Result::next_position;
    static constexpr std::uint32_t next_capture_index = Result::next_capture_index;
};

template <typename T, typename Child>
class SequencePrepend
{
public:
    using type = syntax::Sequence<T, Child>;
};

template <typename T, typename... Children>
class SequencePrepend<T, syntax::Sequence<Children...>>
{
public:
    using type = syntax::Sequence<T, Children...>;
};

template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseSequence
{
private:
    static auto select_type()
    {
        using Repeat = ParseRepeat<Pattern, Position, NextIndex>;
        constexpr char next = Pattern.template get<Repeat::next_position>();

        if constexpr (next == '\0' || next == '|' || next == ')')
        {
            return ParseResult<typename Repeat::type, Repeat::next_position, Repeat::next_capture_index>{};
        }
        else
        {
            using Rest = ParseSequence<Pattern, Repeat::next_position, Repeat::next_capture_index>;
            using Sequence = SequencePrepend<typename Repeat::type, typename Rest::type>;

            return ParseResult<typename Sequence::type, Rest::next_position, Rest::next_capture_index>{};
        }
    }

    using Result = decltype(select_type());

public:
    using type = typename Result::type;

    static constexpr std::size_t next_position = Result::next_position;
    static constexpr std::uint32_t next_capture_index = Result::next_capture_index;
};

template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseExpression
{
private:
    static auto select_type()
    {
        using Sequence = ParseSequence<Pattern, Position, NextIndex>;
        constexpr char next = Pattern.template get<Sequence::next_position>();

        if constexpr (next == '|')
        {
            using Expression = ParseExpression<Pattern, Sequence::next_position + 1, Sequence::next_capture_index>;
            return ParseResult<syntax::Union<typename Sequence::type, typename Expression::type>, Expression::next_position, Expression::next_capture_index>{};
        }
        else
        {
            return ParseResult<typename Sequence::type, Sequence::next_position, Sequence::next_capture_index>{};
        }
    }

    using Result = decltype(select_type());

public:
    using type = typename Result::type;

    static constexpr std::size_t next_position = Result::next_position;
    static constexpr std::uint32_t next_capture_index = Result::next_capture_index;
};

template <fixed_string Pattern, std::size_t Position, std::uint32_t NextIndex>
class ParseGroup
{
private:
    static auto select_type()
    {
        constexpr char current = Pattern.template get<Position>();
        if constexpr (current == '?')
        {
            static_assert(Pattern.template get<Position + 1>() == ':', "Syntax error: Expected ':' after '(?'");

            using Expression = ParseExpression<Pattern, Position + 2, NextIndex>;
            static_assert(Pattern.template get<Expression::next_position>() == ')', "Expected ')'");
            return ParseResult<syntax::NonCaptureGroup<typename Expression::type>, Expression::next_position + 1, Expression::next_capture_index>{};
        }
        else
        {
            using Expression = ParseExpression<Pattern, Position, NextIndex + 1>;
            static_assert(Pattern.template get<Expression::next_position>() == ')', "Syntax error: Expected ')'");
            return ParseResult<syntax::CaptureGroup<NextIndex, typename Expression::type>, Expression::next_position + 1, Expression::next_capture_index>{};
        }
    }

    using Result = decltype(select_type());

public:
    using type = typename Result::type;

    static constexpr std::size_t next_position = Result::next_position;
    static constexpr std::uint32_t next_capture_index = Result::next_capture_index;
};

template <typename Node, std::uint32_t CaptureCount>
class ValidBackreference : public std::true_type {};

template <std::uint32_t Index, std::uint32_t CaptureCount>
class ValidBackreference<syntax::Backreference<Index>, CaptureCount> : public std::bool_constant<(Index >= 1 && Index <= CaptureCount)> {};

template <std::uint32_t CaptureCount>
class BackreferencePredicate
{
public:
    template <typename Node>
    using Apply = ValidBackreference<Node, CaptureCount>;
};

template <fixed_string Pattern>
class Parse
{
private:
    using Parsed = ParseExpression<Pattern, 0, 1>;

    static_assert(Parsed::next_position == Pattern.len, "Syntax error: unexpected characters after expression");

    static_assert(inspect::Check<typename Parsed::type, BackreferencePredicate<Parsed::next_capture_index - 1>::template Apply>::value,
        "Syntax error: Invalid backreference");

public:
    using type = typename Parsed::type;
};

}