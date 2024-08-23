#include "stdsharp/functional/invocables.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

struct f0
{
    constexpr int operator()(same_as<int> auto /*unused*/) const { return 0; }
};

struct f1
{
    constexpr int operator()(same_as<char> auto /*unused*/) const { return 1; }
};

struct f2
{
    constexpr int operator()(const char* /*unused*/) const { return 2; }
};

struct f3
{
    constexpr int operator()(const int* /*unused*/) const { return 3; }
};

struct f4
{
    constexpr auto operator()() const && { return 4; }
};

using invocables_t = invocables<f0, f1, f2, f3, f4>;

SCENARIO("invocables", "[function][invocables]")
{
    constexpr invocables_t fn;

    STATIC_REQUIRE(fn(0) == 0);
    STATIC_REQUIRE(fn("1") == 2);
    STATIC_REQUIRE(fn(static_cast<int*>(nullptr)) == 3);
    STATIC_REQUIRE(fn.get<0>()(1) == 0);
    STATIC_REQUIRE(fn.cget<1>()('1') == 1);
    STATIC_REQUIRE(invocables_t{}() == 4);

    STATIC_REQUIRE(constructible_from<invocables_t, f0, f1, f2, f3>);
    STATIC_REQUIRE(constructible_from<invocables_t, f0, f1, f2>);
    STATIC_REQUIRE(constructible_from<invocables_t, f0, f1>);
    STATIC_REQUIRE(constructible_from<invocables_t, f0>);
    STATIC_REQUIRE(constructible_from<invocables_t>);
    STATIC_REQUIRE(!invocable<invocables_t, nullptr_t>);
}
