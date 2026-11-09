#include <tinyregex/tinyregex.hpp>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("RepetitionBacktracking")
{
    CHECK(tinyregex::match<"^a*aaaaab$">("aaaaaaaab"));
    CHECK(tinyregex::match<"^a*aaaaab$">("aaaaab"));
    CHECK_FALSE(tinyregex::match<"^a*aaaaab$">("aaaab"));
    CHECK_FALSE(tinyregex::match<"^a*aaaaab$">("aaaaaaaac"));
}

TEST_CASE("AdjacentRepetitionBacktracking")
{
    CHECK(tinyregex::match<"^a*a+aaab$">("aaaaaab"));
    CHECK(tinyregex::match<"^a*a+aaab$">("aaaab"));
    CHECK_FALSE(tinyregex::match<"^a*a+aaab$">("aaab"));
}

TEST_CASE("OptionalBacktracking")
{
    CHECK(tinyregex::match<"^(ab)?abc$">("abc"));
    CHECK(tinyregex::match<"^(ab)?abc$">("ababc"));
    CHECK_FALSE(tinyregex::match<"^(ab)?abc$">("ababbc"));
}

TEST_CASE("AlternationBacktracking")
{
    CHECK(tinyregex::match<"^(a|ab)c$">("abc"));
    CHECK(tinyregex::match<"^(ab|a)b$">("ab"));
    CHECK_FALSE(tinyregex::match<"^(a|ab)c$">("abbc"));
}

TEST_CASE("RepeatedAlternationBacktracking")
{
    CHECK(tinyregex::match<"^(a|ab)+c$">("ababac"));
    CHECK(tinyregex::match<"^(ab|a)*ab$">("abab"));
    CHECK_FALSE(tinyregex::match<"^(a|ab)+c$">("ababbc"));
}

TEST_CASE("CaptureBacktracking")
{
    CHECK(tinyregex::match<R"(^(a+)\1$)">("aaaaaaaa"));
    CHECK(tinyregex::match<R"(^(a+)\1$)">("aa"));
    CHECK_FALSE(tinyregex::match<R"(^(a+)\1$)">("aaaaaaa"));
}

TEST_CASE("CaptureAlternationBacktracking")
{
    CHECK(tinyregex::match<R"(^(a|ab)\1$)">("abab"));
    CHECK(tinyregex::match<R"(^(ab|a)\1b$)">("aab"));
    CHECK_FALSE(tinyregex::match<R"(^(a|ab)\1$)">("aba"));
}

TEST_CASE("MultipleCaptureBacktracking")
{
    CHECK(tinyregex::match<R"(^(a+)(a+)\1\2$)">("aaaaaaaa"));
    CHECK(tinyregex::match<R"(^(a+)(a+)\1\2$)">("aaaa"));
    CHECK_FALSE(tinyregex::match<R"(^(a+)(a+)\1\2$)">("aaaaaaa"));
}

TEST_CASE("AmbiguousAlternation")
{
    const std::string text(16, 'a');

    CHECK(tinyregex::match<"^(a|aa)+b$">(text + "b"));
    CHECK_FALSE(tinyregex::match<"^(a|aa)+b$">(text + "c"));
}

TEST_CASE("NestedRepetition")
{
    const std::string text(12, 'a');

    CHECK(tinyregex::match<"^(a+)+b$">(text + "b"));
    CHECK_FALSE(tinyregex::match<"^(a+)+b$">(text + "c"));
}

TEST_CASE("AdjacentRepetitions")
{
    const std::string text(32, 'a');

    CHECK(tinyregex::match<"^a*a*a*aaaaab$">(text + "b"));
    CHECK_FALSE(tinyregex::match<"^a*a*a*aaaaab$">(text + "c"));
}
