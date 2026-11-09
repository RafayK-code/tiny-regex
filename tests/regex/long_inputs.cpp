#include <tinyregex/tinyregex.hpp>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("LongRepetition")
{
    const std::string text(1000, 'a');

    CHECK(tinyregex::search<"^a+$">(text));
    CHECK(tinyregex::search<"^a*$">(text));
    CHECK(tinyregex::search<"^a*b$">(text + "b"));
    CHECK_FALSE(tinyregex::search<"^a+$">(text + "c"));
}

TEST_CASE("LongRepetitionBacktracking")
{
    const std::string text(1000, 'a');

    CHECK(tinyregex::search<"^a*aaaaab$">(text + "b"));
    CHECK_FALSE(tinyregex::search<"^a+b$">(text));
}

TEST_CASE("LongRepeatedGroups")
{
    std::string pairs;
    std::string alternatives;

    for (int i = 0; i < 300; ++i)
    {
        pairs += "ab";
        alternatives += "abcd";
    }

    CHECK(tinyregex::search<"^(ab)+$">(pairs));
    CHECK_FALSE(tinyregex::search<"^(ab)+$">(pairs + "a"));

    CHECK(tinyregex::search<"^(ab|cd)+$">(alternatives));
    CHECK_FALSE(tinyregex::search<"^(ab|cd)+$">(alternatives + "x"));
}

TEST_CASE("LongSearch")
{
    const std::string prefix(10000, 'x');

    CHECK(tinyregex::search<"needle$">(prefix + "needle"));
    CHECK_FALSE(tinyregex::search<"needle">(prefix));
}

TEST_CASE("RepeatedSearchAttempts")
{
    const std::string prefix(256, 'a');

    CHECK(tinyregex::search<"a+b">(prefix + "b"));
    CHECK_FALSE(tinyregex::search<"a+b">(prefix));
}

TEST_CASE("LongBackreference")
{
    const std::string captured(500, 'a');
    const std::string text = captured + "b" + captured + "b";

    CHECK(tinyregex::search<R"(^(a+)b\1b$)">(text));

    const std::string mismatch =
        captured + "b" + std::string(499, 'a') + "cb";

    CHECK_FALSE(tinyregex::search<R"(^(a+)b\1b$)">(mismatch));
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

TEST_CASE("LongPattern")
{
    std::string text;

    for (int i = 0; i < 4; ++i)
        text += "abcdefghijklmnop";

    CHECK(tinyregex::search<
        "^abcdefghijklmnop"
        "abcdefghijklmnop"
        "abcdefghijklmnop"
        "abcdefghijklmnop$"
    >(text));

    text.back() = 'x';

    CHECK_FALSE(tinyregex::search<
        "^abcdefghijklmnop"
        "abcdefghijklmnop"
        "abcdefghijklmnop"
        "abcdefghijklmnop$"
    >(text));
}