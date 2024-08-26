#include "stdsharp/bitset/bitset.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("bitset iterator", "[bitset]")
{
    using bitset_iterator = bitset_iterator<4>;

    STATIC_REQUIRE(same_as<bitset_iterator::value_type, bitset<4>::reference>);
    STATIC_REQUIRE(same_as<bitset_const_iterator<4>::value_type, bool>);
    STATIC_REQUIRE(random_access_iterator<bitset_iterator>);
    STATIC_REQUIRE(constructible_from<bitset_iterator, bitset<4>&, size_t>);
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
