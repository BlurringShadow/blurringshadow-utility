#include "stdsharp/memory/launder_iterator.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("launder iterator", "[memory][launder iterator]")
{
    STATIC_REQUIRE(contiguous_iterator<launder_iterator<int>>);
}
