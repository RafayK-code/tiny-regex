#pragma once
#include "syntax.hpp"

#include <array>
#include <string_view>

namespace tinyregex::detail::matcher
{

constexpr bool in_bounds(std::string_view text, std::size_t idx)
{
    return idx < text.size();
}

struct Capture
{
    std::size_t start {};
    std::size_t end {};
    bool matched { false };
};

class Success
{
public:
    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_cast<void>(text);
        static_cast<void>(idx);
        static_cast<void>(captures);
        return true;
    }
};

template <typename T, typename K>
class Match;

template <char C, typename K>
class Match<syntax::Literal<C>, K>
{
public:
    K next;

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

template <typename K>
class Match<syntax::Wildcard, K>
{
public:
    K next;

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

template <typename K>
class Match<syntax::StartAnchor, K>
{
public:
    K next;

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

template <typename K>
class Match<syntax::EndAnchor, K>
{
public:
    K next;

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

template <typename T, typename K>
class BuildSequenceMatch;

template <typename K>
class BuildSequenceMatch<syntax::Sequence<>, K>
{
public:
    using type = K;

    static type make(const K& next)
    {
        return next;
    }
};

template <typename First, typename... Rest, typename K>
class BuildSequenceMatch<syntax::Sequence<First, Rest...>, K>
{
private:
    using Tail = BuildSequenceMatch<syntax::Sequence<Rest...>, K>;

public:
    using type = Match<First, typename Tail::type>;

    static type make(const K& next)
    {
        return type{Tail::make(next)};
    }
};

template <typename... Children, typename K>
class Match<syntax::Sequence<Children...>, K>
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        auto chain = BuildSequenceMatch<syntax::Sequence<Children...>, K>::make(next);
        return chain(text, idx, captures);
    }
};

template <typename Left, typename Right, typename K>
class Match<syntax::Union<Left, Right>, K>
{
public:
    K next;

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

template <typename Child, typename K>
class RepeatContinue
{
public:
    std::size_t start_idx {};
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (idx == start_idx)
        {
            return false;
        }

        return Match<syntax::RepeatZeroOrMore<Child>, K>{next}(text, idx, captures);
    }
};

template <typename Child, typename K>
class Match<syntax::RepeatZeroOrMore<Child>, K>
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        auto repeat = Match<Child, RepeatContinue<Child, K>>{ RepeatContinue<Child, K>{idx, next} };
        if (repeat(text, idx, captures))
        {
            return true;
        }

        return next(text, idx, captures);
    }
};

template <typename Child, typename K>
class Match<syntax::RepeatOneOrMore<Child>, K>
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        using Rest = Match<syntax::RepeatZeroOrMore<Child>, K>;
        if (Match<Child, Rest>{Rest{next}}(text, idx, captures))
        {
            return true;
        }

        return false;
    }
};

template <typename Child, typename K>
class Match<syntax::Optional<Child>, K>
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        if (Match<Child, K>{next}(text, idx, captures))
        {
            return true;
        }

        return next(text, idx, captures);
    }
};

template <std::uint32_t Index, typename K>
class CaptureEnd
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_assert(Index >= 1 && Index <= N);
        auto& capture = captures[Index - 1];
        const auto prev = capture;

        capture.end = idx;
        capture.matched = true;
        if (next(text, idx, captures))
        {
            return true;
        }

        capture = prev;
        return false;
    }
};

template <std::uint32_t Index, typename Child, typename K>
class Match<syntax::CaptureGroup<Index, Child>, K>
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_assert(Index >= 1 && Index <= N);
        auto& capture = captures[Index - 1];
        const auto prev = capture;

        capture.start = idx;
        capture.end = idx;
        capture.matched = false;

        auto group = Match<Child, CaptureEnd<Index, K>>{ CaptureEnd<Index, K>{next} };
        if (group(text, idx, captures))
        {
            return true;
        }

        capture = prev;
        return false;
    }
};

template <typename Child, typename K>
class Match<syntax::NonCaptureGroup<Child>, K>
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        return Match<Child, K>{next}(text, idx, captures);
    }
};

template <std::uint32_t Index, typename K>
class Match<syntax::Backreference<Index>, K>
{
public:
    K next;

    template <std::size_t N>
    bool operator()(std::string_view text, std::size_t idx, std::array<Capture, N>& captures) const
    {
        static_assert(Index >= 1 && Index <= N);
        const auto& group = captures[Index  - 1];
        if (!group.matched || group.start > group.end || group.end > text.size())
        {
            return false;
        }

        const std::size_t length = group.end - group.start;
        if (idx > text.size() || length > text.size() - idx)
        {
            return false;
        }

        if (text.substr(group.start, length) != text.substr(idx, length))
        {
            return false;
        }

        return next(text, idx + length, captures);
    }
};

}