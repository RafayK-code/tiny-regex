#include "matcher_test.hpp"
#include <doctest/doctest.h>

using namespace tinyregex::detail::syntax;
using namespace tinyregex_test;

TEST_CASE("NonCaptureGroupMatching")
{
    using AST = Sequence<
        NonCaptureGroup<Sequence<Literal<'a'>, Literal<'b'>>>,
        EndAnchor
    >;

    CHECK(match<AST>("ab"));
    CHECK_FALSE(match<AST>("a"));
    CHECK_FALSE(match<AST>("ba"));
}

TEST_CASE("BackreferenceMatching")
{
    using AST = Sequence<
        CaptureGroup<1, Literal<'a'>>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("aa"));
    CHECK_FALSE(match<AST, 1>("ab"));
    CHECK_FALSE(match<AST, 1>("a"));
    CHECK_FALSE(match<AST, 1>(""));
}

TEST_CASE("BackreferenceSequenceMatching")
{
    using AST = Sequence<
        CaptureGroup<1, Sequence<Literal<'a'>, Literal<'b'>>>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("abab"));
    CHECK_FALSE(match<AST, 1>("abac"));
    CHECK_FALSE(match<AST, 1>("aba"));
}

TEST_CASE("BackreferenceUnionMatching")
{
    using AST = Sequence<
        CaptureGroup<1, Union<Literal<'a'>, Literal<'b'>>>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("aa"));
    CHECK(match<AST, 1>("bb"));
    CHECK_FALSE(match<AST, 1>("ab"));
}

TEST_CASE("BackreferenceMultipleGroups")
{
    using AST = Sequence<
        CaptureGroup<1, Literal<'a'>>,
        CaptureGroup<2, Literal<'b'>>,
        Backreference<2>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 2>("abba"));
    CHECK_FALSE(match<AST, 2>("abab"));
    CHECK_FALSE(match<AST, 2>("abb"));
}

TEST_CASE("BackreferenceWildcardMatching")
{
    using AST = Sequence<
        CaptureGroup<1, Wildcard>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("aa"));
    CHECK(match<AST, 1>("  "));
    CHECK_FALSE(match<AST, 1>("ab"));
    CHECK_FALSE(match<AST, 1>("\n\n"));
}

TEST_CASE("BackreferenceGreedyBacktracking")
{
    using AST = Sequence<
        CaptureGroup<1, RepeatOneOrMore<Wildcard>>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("aa"));
    CHECK(match<AST, 1>("abab"));
    CHECK(match<AST, 1>("abcabc"));
    CHECK_FALSE(match<AST, 1>("aba"));
    CHECK_FALSE(match<AST, 1>("abcabd"));
}

TEST_CASE("BackreferenceUnmatchedCapture")
{
    using AST = Sequence<
        Optional<CaptureGroup<1, Literal<'a'>>>,
        Literal<'b'>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("aba"));
    CHECK_FALSE(match<AST, 1>("b")); // Group never participated.
}

TEST_CASE("BackreferenceEmptyCapture")
{
    using AST = Sequence<
        CaptureGroup<1, RepeatZeroOrMore<Literal<'a'>>>,
        Literal<'b'>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("b")); // Group participated with an empty capture.
    CHECK(match<AST, 1>("aabaa"));
    CHECK_FALSE(match<AST, 1>("aaba"));
}

TEST_CASE("BackreferenceAlternativeBacktracking")
{
    using AST = Sequence<
        CaptureGroup<
            1,
            Union<
                Literal<'a'>,
                Sequence<Literal<'a'>, Literal<'b'>>
            >
        >,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("aa"));
    CHECK(match<AST, 1>("abab")); // Must reconsider the shorter capture.
    CHECK_FALSE(match<AST, 1>("aba"));
}

TEST_CASE("BackreferenceCaptureRollback")
{
    using AST = Sequence<
        Union<
            Sequence<CaptureGroup<1, Literal<'a'>>, Literal<'b'>>,
            Literal<'a'>
        >,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("aba"));
    CHECK_FALSE(match<AST, 1>("aa")); // Failed left branch must not leak its capture.
}

TEST_CASE("BackreferenceNestedCaptures")
{
    using AST = Sequence<
        CaptureGroup<
            1,
            Sequence<Literal<'a'>, CaptureGroup<2, Literal<'b'>>>
        >,
        Backreference<2>,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 2>("abbab"));
    CHECK_FALSE(match<AST, 2>("ababa"));
}

TEST_CASE("BackreferenceRepeatedCapture")
{
    using AST = Sequence<
        RepeatOneOrMore<
            CaptureGroup<1, Union<Literal<'a'>, Literal<'b'>>>
        >,
        Backreference<1>,
        EndAnchor
    >;

    CHECK(match<AST, 1>("abb"));
    CHECK(match<AST, 1>("baa"));
    CHECK_FALSE(match<AST, 1>("aba"));
}