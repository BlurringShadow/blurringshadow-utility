#include "stdsharp/memory/allocator_adaptor.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

using int_allocator = allocator<int>;
using int_adaptor = allocator_adaptor<int_allocator>;

SCENARIO("allocator_adaptor", "[memory][allocator_adaptor]")
{
    GIVEN("an allocator_adaptor")
    {
        int_adaptor allocator;

        WHEN("the allocator is default constructed")
        {
            THEN("the allocator is equal to itself") { REQUIRE(allocator == allocator); }
        }

        WHEN("the allocator is copy constructed")
        {
            int_adaptor other(allocator);

            THEN("the allocator is equal to the other allocator") { REQUIRE(allocator == other); }
        }

        WHEN("the allocator is move constructed")
        {
            int_adaptor other(std::move(allocator));

            THEN("the allocator is equal to the other allocator") { REQUIRE(allocator == other); }
        }

        WHEN("the allocator is constructed in place")
        {
            int_adaptor other(std::in_place);

            THEN("the allocator is equal to the other allocator") { REQUIRE(allocator == other); }
        }
    }
}
