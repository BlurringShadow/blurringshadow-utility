#include "stdsharp/memory/allocation_traits.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

using int_allocator = allocator<int>;
using int_allocation_traits = allocation_traits<int_allocator>;
using int_allocation = int_allocation_traits::allocation;
using int_callocation = int_allocation_traits::callocation;
using int_allocations = vector<int_allocation>;
using int_callocations = vector<int_callocation>;

SCENARIO("allocation", "[memory][allocation_traits]")
{
    STATIC_REQUIRE(std::ranges::input_range<int_allocation>);
    STATIC_REQUIRE(std::ranges::output_range<int_allocation, int>);
    STATIC_REQUIRE(std::constructible_from<int_allocation>);
    STATIC_REQUIRE(std::constructible_from<int_allocation, int*, int*>);
    STATIC_REQUIRE(std::constructible_from<int_allocation, int*, int_allocation_traits::size_type>);

    STATIC_REQUIRE(std::ranges::input_range<int_callocation>);
    STATIC_REQUIRE(std::constructible_from<int_callocation>);
    STATIC_REQUIRE(std::constructible_from<int_callocation, const int*, const int*>);
    STATIC_REQUIRE(std::constructible_from<
                   int_callocation,
                   const int*,
                   int_allocation_traits::size_type>);

    STATIC_REQUIRE(allocations<int_allocations, int_allocator>);
    STATIC_REQUIRE(callocations<int_callocations, int_allocator>);
}
