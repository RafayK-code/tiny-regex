#pragma once
#include <cstddef>
#include <algorithm>

namespace tinyregex::detail
{

template <std::size_t N>
class fixed_string
{
public:
    constexpr fixed_string(const char (&text)[N])
    {
        std::copy_n(text, N, data);
    }

    template <std::size_t Index>
    constexpr char get() const
    {
        static_assert(Index < N, "fixed_string: out of bounds");
        return data[Index];
    }

    char data[N] {};
    std::size_t len = N - 1;
};

}