#include "type_info.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("type id compare")
{
    static_assert(cttid<int> == cttid<int>); // NOLINT
    static_assert(cttid<int> != cttid<float>);

    // extern variable of cttid
    // compare at runtime
    REQUIRE(int_type_1 == int_type_2);
    REQUIRE(!(int_type_1 < int_type_2));
    REQUIRE(int_type_1 <= int_type_2);
}
