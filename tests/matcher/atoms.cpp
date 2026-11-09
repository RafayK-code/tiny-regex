#include "matcher_test.hpp"
#include <doctest/doctest.h>

using namespace tinyregex::detail::syntax;
using namespace tinyregex_test;

TEST_CASE("LiteralMatching")
{
    CHECK(match<Literal<'a'>>("a"));
    CHECK(match<Literal<'b'>>("b"));
    CHECK(match<Literal<'5'>>("5"));
    CHECK(match<Literal<' '>>(" "));
    CHECK(match<Literal<'\n'>>("\n"));
}

TEST_CASE("LiteralMismatch")
{
    CHECK_FALSE(match<Literal<'a'>>("b"));
    CHECK_FALSE(match<Literal<'a'>>("A"));
    CHECK_FALSE(match<Literal<'a'>>("5"));
}

TEST_CASE("LiteralEmptyInput")
{
    CHECK_FALSE(match<Literal<'a'>>(""));
}

TEST_CASE("WildcardMatching")
{
    CHECK(match<Wildcard>("a"));
    CHECK(match<Wildcard>("Z"));
    CHECK(match<Wildcard>("0"));
    CHECK(match<Wildcard>(" "));
    CHECK(match<Wildcard>("\t"));

    CHECK_FALSE(match<Wildcard>("\n"));
    CHECK_FALSE(match<Wildcard>(""));
}

TEST_CASE("StartAnchorMatching")
{
    CHECK(match_at<StartAnchor>("abc", 0));
    CHECK(match_at<StartAnchor>("", 0));

    CHECK_FALSE(match_at<StartAnchor>("abc", 1));
    CHECK_FALSE(match_at<StartAnchor>("abc", 3));
}

TEST_CASE("EndAnchorMatching")
{
    CHECK(match_at<EndAnchor>("abc", 3));
    CHECK(match_at<EndAnchor>("", 0));

    CHECK_FALSE(match_at<EndAnchor>("abc", 0));
    CHECK_FALSE(match_at<EndAnchor>("abc", 2));
}