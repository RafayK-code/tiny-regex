#include <tinyregex/tinyregex.hpp>
#include <doctest/doctest.h>

TEST_CASE("SubstringSearch")
{
    CHECK(tinyregex::search<"cat">("the cat sat"));
    CHECK(tinyregex::search<"cat">("concatenate"));
    CHECK(tinyregex::search<"cat">("cacat"));
}

TEST_CASE("SearchAtStart")
{
    CHECK(tinyregex::search<"cat">("cat sat"));
    CHECK(tinyregex::search<"^cat">("cat sat"));
    CHECK_FALSE(tinyregex::search<"^cat">("the cat sat"));
}

TEST_CASE("SearchAtEnd")
{
    CHECK(tinyregex::search<"cat">("the cat"));
    CHECK(tinyregex::search<"cat$">("the cat"));
    CHECK_FALSE(tinyregex::search<"cat$">("the cat sat"));
}

TEST_CASE("SearchFailure")
{
    CHECK_FALSE(tinyregex::search<"cat">("dog"));
    CHECK_FALSE(tinyregex::search<"cat">(""));
}

TEST_CASE("AnchoredSearch")
{
    CHECK(tinyregex::search<"^cat$">("cat"));
    CHECK_FALSE(tinyregex::search<"^cat$">("scat"));
    CHECK_FALSE(tinyregex::search<"^cat$">("catapult"));
}

TEST_CASE("PhraseSearch")
{
    CHECK(tinyregex::search<"hello world">("say hello world please"));
    CHECK_FALSE(tinyregex::search<"hello world">("say hello there"));
}
