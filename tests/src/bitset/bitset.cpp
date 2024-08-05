#include "stdsharp/bitset/bitset.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("bitset iterator", "[bitset]")
{
    STATIC_REQUIRE(random_access_iterator<bitset_iterator<4>>);
    STATIC_REQUIRE(constructible_from<bitset_iterator<4>, bitset<4>&, size_t>);
}

SCENARIO("bitset range", "[bitset]")
{
    bitset<4> set{0b0101};

    STATIC_REQUIRE(std::ranges::random_access_range<decltype(bitset_rng(set))>);

    REQUIRE(bitset_rng(set).size() == 4);
    REQUIRE(bitset_rng(set)[0] == true);
    REQUIRE(bitset_crng(set)[1] == false);
    REQUIRE(bitset_rng(as_const(set))[1] == false);
}
