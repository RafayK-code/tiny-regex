#include "matcher_test.hpp"
#include <doctest/doctest.h>

using namespace tinyregex::detail::syntax;
using namespace tinyregex_test;

TEST_CASE("SequenceMatching")
{
    using AST = Sequence<Literal<'a'>, Literal<'b'>, Literal<'c'>>;

    CHECK(match<AST>("abc"));
    CHECK_FALSE(match<AST>("acb"));
    CHECK_FALSE(match<AST>("ab"));
    CHECK_FALSE(match<AST>("abd"));
    CHECK_FALSE(match<AST>(""));
}

TEST_CASE("NestedSequenceMatching")
{
    using AST = Sequence<
        Sequence<Literal<'a'>, Literal<'b'>>,
        Sequence<Literal<'c'>, Literal<'d'>>,
        EndAnchor
    >;

    CHECK(match<AST>("abcd"));
    CHECK_FALSE(match<AST>("abdc"));
    CHECK_FALSE(match<AST>("abc"));
}

TEST_CASE("SequenceAnchorMatching")
{
    using AST = Sequence<
        StartAnchor,
        Literal<'a'>,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("ab"));
    CHECK_FALSE(match<AST>("abc"));
    CHECK_FALSE(match_at<AST>("zab", 1));
}

TEST_CASE("SequenceWildcardMatching")
{
    using AST = Sequence<
        Literal<'a'>,
        Wildcard,
        Wildcard,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("axyb"));
    CHECK(match<AST>("a \tb"));
    CHECK_FALSE(match<AST>("axb"));
    CHECK_FALSE(match<AST>("ax\nb"));
}

TEST_CASE("SequenceMixedQuantifiers")
{
    using AST = Sequence<
        RepeatOneOrMore<Literal<'a'>>,
        Optional<Literal<'b'>>,
        RepeatZeroOrMore<Literal<'c'>>,
        Literal<'d'>,
        EndAnchor
    >;

    CHECK(match<AST>("ad"));
    CHECK(match<AST>("aabcccd"));
    CHECK_FALSE(match<AST>("bccd"));
    CHECK_FALSE(match<AST>("aaccbd"));
}

TEST_CASE("NestedSequenceBacktracking")
{
    using AST = Sequence<
        Sequence<
            RepeatZeroOrMore<Literal<'a'>>,
            Optional<Literal<'a'>>
        >,
        Sequence<Literal<'a'>, Literal<'b'>>,
        EndAnchor
    >;

    CHECK(match<AST>("ab"));
    CHECK(match<AST>("aaaab"));
    CHECK_FALSE(match<AST>("b"));
    CHECK_FALSE(match<AST>("aaaac"));
}