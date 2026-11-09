#include <tinyregex/tinyregex.hpp>
#include <doctest/doctest.h>

TEST_CASE("SubstringSearch")
{
    CHECK(tinyregex::match<"cat">("the cat sat"));
    CHECK(tinyregex::match<"cat">("concatenate"));
    CHECK(tinyregex::match<"cat">("cacat"));
}

TEST_CASE("SearchAtStart")
{
    CHECK(tinyregex::match<"cat">("cat sat"));
    CHECK(tinyregex::match<"^cat">("cat sat"));
    CHECK_FALSE(tinyregex::match<"^cat">("the cat sat"));
}

TEST_CASE("SearchAtEnd")
{
    CHECK(tinyregex::match<"cat">("the cat"));
    CHECK(tinyregex::match<"cat$">("the cat"));
    CHECK_FALSE(tinyregex::match<"cat$">("the cat sat"));
}

TEST_CASE("SearchFailure")
{
    CHECK_FALSE(tinyregex::match<"cat">("dog"));
    CHECK_FALSE(tinyregex::match<"cat">(""));
}

TEST_CASE("AnchoredSearch")
{
    CHECK(tinyregex::match<"^cat$">("cat"));
    CHECK_FALSE(tinyregex::match<"^cat$">("scat"));
    CHECK_FALSE(tinyregex::match<"^cat$">("catapult"));
}

TEST_CASE("PhraseSearch")
{
    CHECK(tinyregex::match<"hello world">("say hello world please"));
    CHECK_FALSE(tinyregex::match<"hello world">("say hello there"));
}
