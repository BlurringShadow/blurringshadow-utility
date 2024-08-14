#include "stdsharp/utility/forward_cast.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

struct t0
{
};

struct t1 : t0
{
    int v;
};

class t2 : t1
{
    float v;
};

SCENARIO("forward cast", "[utility][forward cast]")
{
    STATIC_REQUIRE(same_as<forward_cast_t<int, int>, int&&>);
    STATIC_REQUIRE(same_as<forward_cast_t<int&, int>, int&>);
    STATIC_REQUIRE(same_as<forward_cast_t<const int&, int>, const int&>);
    STATIC_REQUIRE(same_as<forward_cast_t<const int&, int&>, const int&>);
    STATIC_REQUIRE(same_as<forward_cast_t<const volatile int&, int&&>, const volatile int&>);

    GIVEN("t0")
    {
        [[maybe_unused]] t0 v{};

        STATIC_REQUIRE(same_as<decltype(forward_cast<t0&, t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&, t0>(as_const(v))), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&, t0&>(as_const(v))), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<t0&&, t0>(v)), t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&&, t0>(as_const(v))), const t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&&, t0&>(as_const(v))), const t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t0&&, t0&&>(as_const(v))), const t0&&>);
    }

    GIVEN("t1")
    {
        [[maybe_unused]] t1 v{};

        STATIC_REQUIRE(same_as<decltype(forward_cast<t1&, t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t1&, t0>(as_const(v))), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<t1&&, t0>(v)), t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t1&&, t0>(as_const(v))), const t0&&>);
    }

    GIVEN("t2")
    {
        [[maybe_unused]] t2 v{};
        STATIC_REQUIRE(same_as<decltype(forward_cast<t2&, t1>(v)), t1&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t2&, t1>(v)), const t1&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<t2&&, t1>(v)), t1&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t2&&, t1>(v)), const t1&&>);

        STATIC_REQUIRE(same_as<decltype(forward_cast<t2&, t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t2&, t0>(v)), const t0&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<t2&&, t0>(v)), t0&&>);
        STATIC_REQUIRE(same_as<decltype(forward_cast<const t2&&, t0>(v)), const t0&&>);
    }
}
