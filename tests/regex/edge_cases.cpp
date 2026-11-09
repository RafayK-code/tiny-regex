#include <tinyregex/tinyregex.hpp>
#include <doctest/doctest.h>

TEST_CASE("EmptyInput")
{
    CHECK(tinyregex::search<"a*">(""));
    CHECK(tinyregex::search<"a?">(""));
    CHECK(tinyregex::search<"^$">(""));
    CHECK_FALSE(tinyregex::search<"a+">(""));
    CHECK_FALSE(tinyregex::search<"cat">(""));
}

TEST_CASE("EmptyMatch")
{
    CHECK(tinyregex::search<"a*">("xyz"));
    CHECK(tinyregex::search<"a?">("b"));
    CHECK_FALSE(tinyregex::search<"^a*$">("xyz"));
}

TEST_CASE("EndPositionMatch")
{
    CHECK(tinyregex::search<"$">("abc"));
    CHECK(tinyregex::search<"$">(""));
    CHECK(tinyregex::search<"a*$">("bbb"));
    CHECK_FALSE(tinyregex::search<"^$">("abc"));
}

TEST_CASE("WildcardNewline")
{
    CHECK_FALSE(tinyregex::search<".">("\n"));
    CHECK_FALSE(tinyregex::search<"a.b">("a\nb"));
    CHECK(tinyregex::search<"a.b">("a b"));
    CHECK(tinyregex::search<"a.b">("a\tb"));
    CHECK(tinyregex::search<".">("\na")); // Search continues after newline.
}

TEST_CASE("LiteralNewline")
{
    CHECK(tinyregex::search<"\n">("a\nb"));
    CHECK(tinyregex::search<"^a\nb$">("a\nb"));
    CHECK_FALSE(tinyregex::search<"\n">("ab"));
}

TEST_CASE("AnchorBoundaries")
{
    CHECK_FALSE(tinyregex::search<"^b">("a\nb"));
    CHECK_FALSE(tinyregex::search<"a$">("a\n"));
    CHECK(tinyregex::search<"b$">("a\nb"));
}

TEST_CASE("EscapedMetacharacters")
{
    CHECK(tinyregex::search<R"(a\.b)">("a.b"));
    CHECK_FALSE(tinyregex::search<R"(a\.b)">("aXb"));

    CHECK(tinyregex::search<R"(a\*b)">("a*b"));
    CHECK(tinyregex::search<R"(a\?b)">("a?b"));
    CHECK(tinyregex::search<R"(\(a\|b\))">("(a|b)"));
    CHECK(tinyregex::search<R"(\^a\$)">("^a$"));
    CHECK(tinyregex::search<R"(a\\b)">("a\\b"));
}

TEST_CASE("UnmatchedCapture")
{
    CHECK_FALSE(tinyregex::search<R"(^(a)?b\1$)">("b"));
    CHECK(tinyregex::search<R"(^(a)?b\1$)">("aba"));
}

TEST_CASE("EmptyCapture")
{
    CHECK(tinyregex::search<R"(^(a*)b\1$)">("b"));
    CHECK(tinyregex::search<R"(^(a*)b\1$)">("aabaa"));
    CHECK_FALSE(tinyregex::search<R"(^(a*)b\1$)">("aaba"));
}

TEST_CASE("EmptyRepetition")
{
    CHECK(tinyregex::search<"^(a?)*b$">("b"));
    CHECK(tinyregex::search<"^(a?)*b$">("aaab"));
    CHECK(tinyregex::search<"^(a?)+$">(""));
    CHECK_FALSE(tinyregex::search<"^(a?)*b$">("aaac"));
}

TEST_CASE("SearchCaptureRollback")
{
    CHECK(tinyregex::search<R"((a)?b\1)">("abxaba"));
    CHECK_FALSE(tinyregex::search<R"((a)?b\1)">("abxba"));
}

TEST_CASE("RegexReuse")
{
    tinyregex::Regex<R"((a|b)\1)"> regex;

    CHECK(regex.search("aa"));
    CHECK_FALSE(regex.search("ab"));
    CHECK(regex.search("bb"));
}

TEST_CASE("EmbeddedNull")
{
    constexpr char text[] = {'a', '\0', 'b'};
    const std::string_view view{text, sizeof(text)};

    CHECK(tinyregex::search<"b$">(view));
    CHECK(tinyregex::search<"^a.b$">(view));
    CHECK_FALSE(tinyregex::search<"^ab$">(view));
}