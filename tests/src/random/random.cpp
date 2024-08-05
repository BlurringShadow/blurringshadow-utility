#include "stdsharp/random/random.h"
#include "test.h"

#include <catch2/catch_test_macros.hpp>

STDSHARP_TEST_NAMESPACES;

SCENARIO("random concept", "[random]")
{
    STATIC_REQUIRE(seed_sequence<seed_seq>);
    STATIC_REQUIRE(random_number_engine<mt19937>);
    STATIC_REQUIRE(random_number_distribution<bernoulli_distribution>);
}

SCENARIO("random distribution", "[random]")
{
    GIVEN("a uniform distribution")
    {
        auto&& dist = make_uniform_distribution(0.5, 1);

        WHEN("calling operator()")
        {
            THEN("the result is a bool") { REQUIRE(dist(get_engine<>())); }
        }
    }
}
