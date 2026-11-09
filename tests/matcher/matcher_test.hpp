#pragma once

#include <tinyregex/detail/matcher.hpp>

#include <array>
#include <string_view>

using namespace tinyregex::detail::matcher;

namespace tinyregex_test
{

template <typename AST, std::size_t CaptureCount = 0>
bool match_at(std::string_view text, std::size_t idx)
{
    std::array<Capture, CaptureCount> captures{};
    Match<AST, Success> matcher{ Success{} };
    return matcher(text, idx, captures);
}

template <typename AST, std::size_t CaptureCount = 0>
bool match(std::string_view text)
{
    return match_at<AST, CaptureCount>(text, 0);
}

}
