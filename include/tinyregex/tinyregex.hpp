#pragma once
#include "detail/fixed_string.hpp"
#include "detail/parser.hpp"
#include "detail/matcher.hpp"

#include <string_view>

namespace tinyregex
{

template <detail::fixed_string Pattern>
class Regex
{
private:
    using Parsed = detail::parser::Parse<Pattern>;
    using AST = typename Parsed::type;

public:
    bool match(std::string_view text)
    {
        std::array<detail::matcher::Capture, Parsed::capture_group_count> captures{};

        static constexpr auto matcher = detail::matcher::Match<AST, detail::matcher::Success>{ detail::matcher::Success{} };

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

template <detail::fixed_string Pattern>
inline bool match(std::string_view text)
{
    return Regex<Pattern>{}.match(text);
}

}