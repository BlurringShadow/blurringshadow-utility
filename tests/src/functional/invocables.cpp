#include "stdsharp/functional/invocables.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("invocables", "[function][invocables]")
{
    struct f0
    {
        constexpr int operator()(int /*unused*/) const { return 0; }
    };

    struct f1
    {
        constexpr int operator()(char /*unused*/) const { return 1; }
    };

    struct f2
    {
        constexpr int operator()(const char* /*unused*/) const { return 2; }
    };

    struct f3
    {
        constexpr int operator()(const int* /*unused*/) const { return 3; }
    };

    using invocables = invocables<f0, f1, f2, f3>;

    constexpr invocables fn;

    STATIC_REQUIRE(fn.get<0>()(1) == 0);
    STATIC_REQUIRE(fn.cget<1>()(1) == 1);
    STATIC_REQUIRE(fn.invoke_at<1>(1) == 1);
    STATIC_REQUIRE(fn.invoke_at<2>("1") == 2);
    STATIC_REQUIRE(fn.invoke_at<3>(nullptr) == 3);
    STATIC_REQUIRE(!requires { fn.invoke_at<4>(); });

    STATIC_REQUIRE(constructible_from<invocables, f0, f1, f2, f3>);
    STATIC_REQUIRE(constructible_from<invocables, f0, f1, f2>);
    STATIC_REQUIRE(constructible_from<invocables, f0, f1>);
    STATIC_REQUIRE(constructible_from<invocables, f0>);
    STATIC_REQUIRE(constructible_from<invocables>);
    STATIC_REQUIRE(!invocable<invocables, int>);
}
