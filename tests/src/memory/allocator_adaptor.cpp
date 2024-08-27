#include "stdsharp/memory/allocator_adaptor.h"

#include "test.h"

STDSHARP_TEST_NAMESPACES;

using int_allocator = allocator<int>;
using int_adaptor = allocator_adaptor<int_allocator>;

SCENARIO("allocator_adaptor", "[memory][allocator_adaptor]")
{
    GIVEN("an allocator_adaptor")
    {
        stdsharp::allocator_adaptor<std::allocator<int>> allocator;

        WHEN("the allocator is default constructed")
        {
            THEN("the allocator is equal to itself")
            {
                REQUIRE(allocator.is_equal(allocator));
            }
        }

        WHEN("the allocator is copy constructed")
        {
            stdsharp::allocator_adaptor<std::allocator<int>> other(allocator);

            THEN("the allocator is equal to the other allocator")
            {
                REQUIRE(allocator.is_equal(other));
            }
        }

        WHEN("the allocator is move constructed")
        {
            stdsharp::allocator_adaptor<std::allocator<int>> other(std::move(allocator));

            THEN("the allocator is equal to the other allocator")
            {
                REQUIRE(allocator.is_equal(other));
            }
        }

        WHEN("the allocator is constructed in place")
        {
            stdsharp::allocator_adaptor<std::allocator<int>> other(std::in_place, 0);

            THEN("the allocator is equal to the other allocator")
            {
                REQUIRE(allocator.is_equal(other));
            }
        }
    }
}
