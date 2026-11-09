#include "matcher_test.hpp"
#include <doctest/doctest.h>

using namespace tinyregex::detail::syntax;
using namespace tinyregex_test;

TEST_CASE("ZeroOrMoreOptional")
{
    using AST = Sequence<
        RepeatZeroOrMore<Optional<Literal<'a'>>>,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("b"));
    CHECK(match<AST>("aaab"));
    CHECK_FALSE(match<AST>("aaac"));
}

TEST_CASE("OneOrMoreOptional")
{
    using AST = Sequence<
        RepeatOneOrMore<Optional<Literal<'a'>>>,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("b")); // One occurrence of the child can be empty.
    CHECK(match<AST>("aaab"));
    CHECK_FALSE(match<AST>("aaac"));
}

TEST_CASE("OptionalOneOrMore")
{
    using AST = Sequence<
        Optional<RepeatOneOrMore<Literal<'a'>>>,
        Literal<'a'>,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("ab"));
    CHECK(match<AST>("aaaab"));
    CHECK_FALSE(match<AST>("b"));
}

TEST_CASE("AdjacentRepetitions")
{
    using AST = Sequence<
        RepeatZeroOrMore<Literal<'a'>>,
        RepeatOneOrMore<Literal<'a'>>,
        Optional<Literal<'a'>>,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("ab"));
    CHECK(match<AST>("aaaab"));
    CHECK_FALSE(match<AST>("b"));
}

TEST_CASE("RepeatedOptionalSequence")
{
    using AST = Sequence<
        RepeatZeroOrMore<
            Sequence<Literal<'a'>, Optional<Literal<'b'>>>
        >,
        Literal<'a'>,
        Literal<'b'>,
        EndAnchor
    >;

    CHECK(match<AST>("ab"));
    CHECK(match<AST>("aabab"));
    CHECK_FALSE(match<AST>("abb"));
}
