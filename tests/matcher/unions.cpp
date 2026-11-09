#include "matcher_test.hpp"
#include <doctest/doctest.h>

using namespace tinyregex::detail::syntax;
using namespace tinyregex_test;

TEST_CASE("UnionMatching")
{
    using AST = Union<Literal<'a'>, Literal<'b'>>;

    CHECK(match<AST>("a"));
    CHECK(match<AST>("b"));
    CHECK_FALSE(match<AST>("c"));
    CHECK_FALSE(match<AST>(""));
}

TEST_CASE("UnionSequenceMatching")
{
    using AST = Sequence<
        Union<
            Sequence<Literal<'a'>, Literal<'b'>>,
            Sequence<Literal<'a'>, Literal<'c'>>
        >,
        EndAnchor
    >;

    CHECK(match<AST>("ab"));
    CHECK(match<AST>("ac")); // Restore position after partial left failure.
    CHECK_FALSE(match<AST>("ad"));
}

TEST_CASE("NestedUnionMatching")
{
    using AST = Sequence<
        Union<
            Literal<'a'>,
            Union<
                Literal<'b'>,
                Sequence<Literal<'c'>, Literal<'d'>>
            >
        >,
        EndAnchor
    >;

    CHECK(match<AST>("a"));
    CHECK(match<AST>("b"));
    CHECK(match<AST>("cd"));
    CHECK_FALSE(match<AST>("c"));
}

TEST_CASE("UnionSuffixBacktracking")
{
    using AST = Sequence<
        Union<
            Sequence<Literal<'a'>, Literal<'b'>>,
            Literal<'a'>
        >,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("abb"));
    CHECK(match<AST>("ab")); // Left succeeds locally; suffix forces right.
    CHECK_FALSE(match<AST>("abc"));
}

TEST_CASE("UnionLongerAlternativeBacktracking")
{
    using AST = Sequence<
        Union<
            Literal<'a'>,
            Sequence<Literal<'a'>, Literal<'b'>>
        >,
        Literal<'c'>,
        EndAnchor
    >;

    CHECK(match<AST>("ac"));
    CHECK(match<AST>("abc"));
    CHECK_FALSE(match<AST>("abbc"));
}

TEST_CASE("RepeatedUnionMatching")
{
    using AST = Sequence<
        RepeatOneOrMore<
            Union<
                Literal<'a'>,
                Sequence<Literal<'a'>, Literal<'b'>>
            >
        >,
        Literal<'c'>,
        EndAnchor
    >;

    CHECK(match<AST>("ac"));
    CHECK(match<AST>("abac"));
    CHECK(match<AST>("aababc"));
    CHECK_FALSE(match<AST>("c"));
    CHECK_FALSE(match<AST>("abbc"));
}

TEST_CASE("UnionQuantifierBacktracking")
{
    using AST = Sequence<
        Union<
            RepeatOneOrMore<Literal<'a'>>,
            Sequence<Literal<'a'>, Literal<'b'>>
        >,
        Optional<Literal<'c'>>,
        Literal<'d'>,
        EndAnchor
    >;

    CHECK(match<AST>("aaad"));
    CHECK(match<AST>("aaacd"));
    CHECK(match<AST>("abd"));
    CHECK(match<AST>("abcd"));
    CHECK_FALSE(match<AST>("adcd"));
}

TEST_CASE("UnionEmptyAlternativeMatching")
{
    using AST = Sequence<
        Union<Optional<Literal<'a'>>, Literal<'b'>>,
        Literal<'c'>,
        EndAnchor
    >;

    CHECK(match<AST>("c"));
    CHECK(match<AST>("ac"));
    CHECK(match<AST>("bc")); // Empty left match must not hide the right branch.
    CHECK_FALSE(match<AST>("abc"));
}