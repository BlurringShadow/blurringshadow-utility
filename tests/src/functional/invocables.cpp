#include "stdsharp/functional/invocables.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("invocables", "[function][invocables]")
{
    struct f0
    {
        int operator()(int /*unused*/) const { return 0; }
    };

    struct f1
    {
        int operator()(char /*unused*/) const { return 1; }
    };

    struct f2
    {
        int operator()(const char* /*unused*/) const { return 2; }
    };

    struct f3
    {
        int operator()(const int* /*unused*/) const { return 3; }
    };

    using invocables = invocables<f0, f1, f2, f3>;

    invocables fn{};

    REQUIRE(fn.get<0>()(1) == 0);
    REQUIRE(fn.cget<0>()(1) == 0);
    STATIC_REQUIRE(constructible_from<invocables, f0, f1, f2, f3>);
    STATIC_REQUIRE(constructible_from<invocables, f0, f1, f2>);
    STATIC_REQUIRE(constructible_from<invocables, f0, f1>);
    STATIC_REQUIRE(constructible_from<invocables, f0>);
    STATIC_REQUIRE(constructible_from<invocables>);
    STATIC_REQUIRE(!invocable<invocables, int>);
    REQUIRE(fn(static_cast<char*>(nullptr)) == 2);
    REQUIRE(fn(static_cast<int*>(nullptr)) == 3);
}
