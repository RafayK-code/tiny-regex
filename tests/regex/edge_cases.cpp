#include <tinyregex/tinyregex.hpp>
#include <doctest/doctest.h>

TEST_CASE("EmptyInput")
{
    CHECK(tinyregex::match<"a*">(""));
    CHECK(tinyregex::match<"a?">(""));
    CHECK(tinyregex::match<"^$">(""));
    CHECK_FALSE(tinyregex::match<"a+">(""));
    CHECK_FALSE(tinyregex::match<"cat">(""));
}

TEST_CASE("EmptyMatch")
{
    CHECK(tinyregex::match<"a*">("xyz"));
    CHECK(tinyregex::match<"a?">("b"));
    CHECK_FALSE(tinyregex::match<"^a*$">("xyz"));
}

TEST_CASE("EndPositionMatch")
{
    CHECK(tinyregex::match<"$">("abc"));
    CHECK(tinyregex::match<"$">(""));
    CHECK(tinyregex::match<"a*$">("bbb"));
    CHECK_FALSE(tinyregex::match<"^$">("abc"));
}

TEST_CASE("WildcardNewline")
{
    CHECK_FALSE(tinyregex::match<".">("\n"));
    CHECK_FALSE(tinyregex::match<"a.b">("a\nb"));
    CHECK(tinyregex::match<"a.b">("a b"));
    CHECK(tinyregex::match<"a.b">("a\tb"));
    CHECK(tinyregex::match<".">("\na")); // Search continues after newline.
}

TEST_CASE("LiteralNewline")
{
    CHECK(tinyregex::match<"\n">("a\nb"));
    CHECK(tinyregex::match<"^a\nb$">("a\nb"));
    CHECK_FALSE(tinyregex::match<"\n">("ab"));
}

TEST_CASE("AnchorBoundaries")
{
    CHECK_FALSE(tinyregex::match<"^b">("a\nb"));
    CHECK_FALSE(tinyregex::match<"a$">("a\n"));
    CHECK(tinyregex::match<"b$">("a\nb"));
}

TEST_CASE("EscapedMetacharacters")
{
    CHECK(tinyregex::match<R"(a\.b)">("a.b"));
    CHECK_FALSE(tinyregex::match<R"(a\.b)">("aXb"));

    CHECK(tinyregex::match<R"(a\*b)">("a*b"));
    CHECK(tinyregex::match<R"(a\?b)">("a?b"));
    CHECK(tinyregex::match<R"(\(a\|b\))">("(a|b)"));
    CHECK(tinyregex::match<R"(\^a\$)">("^a$"));
    CHECK(tinyregex::match<R"(a\\b)">("a\\b"));
}

TEST_CASE("UnmatchedCapture")
{
    CHECK_FALSE(tinyregex::match<R"(^(a)?b\1$)">("b"));
    CHECK(tinyregex::match<R"(^(a)?b\1$)">("aba"));
}

TEST_CASE("EmptyCapture")
{
    CHECK(tinyregex::match<R"(^(a*)b\1$)">("b"));
    CHECK(tinyregex::match<R"(^(a*)b\1$)">("aabaa"));
    CHECK_FALSE(tinyregex::match<R"(^(a*)b\1$)">("aaba"));
}

TEST_CASE("EmptyRepetition")
{
    CHECK(tinyregex::match<"^(a?)*b$">("b"));
    CHECK(tinyregex::match<"^(a?)*b$">("aaab"));
    CHECK(tinyregex::match<"^(a?)+$">(""));
    CHECK_FALSE(tinyregex::match<"^(a?)*b$">("aaac"));
}

TEST_CASE("SearchCaptureRollback")
{
    CHECK(tinyregex::match<R"((a)?b\1)">("abxaba"));
    CHECK_FALSE(tinyregex::match<R"((a)?b\1)">("abxba"));
}

TEST_CASE("RegexReuse")
{
    tinyregex::Regex<R"((a|b)\1)"> regex;

    CHECK(regex.match("aa"));
    CHECK_FALSE(regex.match("ab"));
    CHECK(regex.match("bb"));
}

TEST_CASE("EmbeddedNull")
{
    constexpr char text[] = {'a', '\0', 'b'};
    const std::string_view view{text, sizeof(text)};

    CHECK(tinyregex::match<"b$">(view));
    CHECK(tinyregex::match<"^a.b$">(view));
    CHECK_FALSE(tinyregex::match<"^ab$">(view));
}