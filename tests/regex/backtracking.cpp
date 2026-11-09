#include <tinyregex/tinyregex.hpp>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("RepetitionBacktracking")
{
    CHECK(tinyregex::search<"^a*aaaaab$">("aaaaaaaab"));
    CHECK(tinyregex::search<"^a*aaaaab$">("aaaaab"));
    CHECK_FALSE(tinyregex::search<"^a*aaaaab$">("aaaab"));
    CHECK_FALSE(tinyregex::search<"^a*aaaaab$">("aaaaaaaac"));
}

TEST_CASE("AdjacentRepetitionBacktracking")
{
    CHECK(tinyregex::search<"^a*a+aaab$">("aaaaaab"));
    CHECK(tinyregex::search<"^a*a+aaab$">("aaaab"));
    CHECK_FALSE(tinyregex::search<"^a*a+aaab$">("aaab"));
}

TEST_CASE("OptionalBacktracking")
{
    CHECK(tinyregex::search<"^(ab)?abc$">("abc"));
    CHECK(tinyregex::search<"^(ab)?abc$">("ababc"));
    CHECK_FALSE(tinyregex::search<"^(ab)?abc$">("ababbc"));
}

TEST_CASE("AlternationBacktracking")
{
    CHECK(tinyregex::search<"^(a|ab)c$">("abc"));
    CHECK(tinyregex::search<"^(ab|a)b$">("ab"));
    CHECK_FALSE(tinyregex::search<"^(a|ab)c$">("abbc"));
}

TEST_CASE("RepeatedAlternationBacktracking")
{
    CHECK(tinyregex::search<"^(a|ab)+c$">("ababac"));
    CHECK(tinyregex::search<"^(ab|a)*ab$">("abab"));
    CHECK_FALSE(tinyregex::search<"^(a|ab)+c$">("ababbc"));
}

TEST_CASE("CaptureBacktracking")
{
    CHECK(tinyregex::search<R"(^(a+)\1$)">("aaaaaaaa"));
    CHECK(tinyregex::search<R"(^(a+)\1$)">("aa"));
    CHECK_FALSE(tinyregex::search<R"(^(a+)\1$)">("aaaaaaa"));
}

TEST_CASE("CaptureAlternationBacktracking")
{
    CHECK(tinyregex::search<R"(^(a|ab)\1$)">("abab"));
    CHECK(tinyregex::search<R"(^(ab|a)\1b$)">("aab"));
    CHECK_FALSE(tinyregex::search<R"(^(a|ab)\1$)">("aba"));
}

TEST_CASE("MultipleCaptureBacktracking")
{
    CHECK(tinyregex::search<R"(^(a+)(a+)\1\2$)">("aaaaaaaa"));
    CHECK(tinyregex::search<R"(^(a+)(a+)\1\2$)">("aaaa"));
    CHECK_FALSE(tinyregex::search<R"(^(a+)(a+)\1\2$)">("aaaaaaa"));
}

TEST_CASE("AmbiguousAlternation")
{
    const std::string text(16, 'a');

    CHECK(tinyregex::search<"^(a|aa)+b$">(text + "b"));
    CHECK_FALSE(tinyregex::search<"^(a|aa)+b$">(text + "c"));
}

TEST_CASE("NestedRepetition")
{
    const std::string text(12, 'a');

    CHECK(tinyregex::search<"^(a+)+b$">(text + "b"));
    CHECK_FALSE(tinyregex::search<"^(a+)+b$">(text + "c"));
}

TEST_CASE("AdjacentRepetitions")
{
    const std::string text(32, 'a');

    CHECK(tinyregex::search<"^a*a*a*aaaaab$">(text + "b"));
    CHECK_FALSE(tinyregex::search<"^a*a*a*aaaaab$">(text + "c"));
}
