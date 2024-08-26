#include "stdsharp/macros.h"
#include "stdsharp/utility/fwd_cast.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

struct m0
{
    bool operator==(const m0&) const = default;
};

struct t0
{
    m0 v;

    bool operator==(const t0&) const = default;

    decltype(auto) get(this auto&& self) noexcept { return (cpp_forward(self).v); }
};

struct t1 : t0
{
};

class t2 : t1
{
};

SCENARIO("forward cast", "[utility][forward cast]")
{
    STATIC_REQUIRE(same_as<forward_like_t<m0, const m0>, m0&&>);
    STATIC_REQUIRE(same_as<forward_like_t<m0&, m0>, m0&>);
    STATIC_REQUIRE(same_as<forward_like_t<const m0&, m0>, const m0&>);
    STATIC_REQUIRE(same_as<forward_like_t<const m0&, m0&>, const m0&>);
    STATIC_REQUIRE(same_as<forward_like_t<const volatile m0&, m0&&>, const volatile m0&>);

    GIVEN("t0")
    {
        [[maybe_unused]] t0 v{};

        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(as_const(v))), const t0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0&>(as_const(v))), const t0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(std::move(v))), t0&&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(std::move(as_const(v)))), const t0&&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0&>(std::move(as_const(v)))), const t0&&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0&&>(std::move(as_const(v)))), const t0&&>);

        {
            constexpr t0 v1{};

            STATIC_REQUIRE(fwd_cast<t0>(v1) == v1);
        }
    }

    GIVEN("t1")
    {
        [[maybe_unused]] t1 v{};

        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(as_const(v))), const t0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(std::move(v))), t0&&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(std::move(as_const(v)))), const t0&&>);
    }

    GIVEN("t2")
    {
        [[maybe_unused]] t2 v{};
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t1>(v)), t1&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t1>(as_const(v))), const t1&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t1>(std::move(v))), t1&&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t1>(std::move(as_const(v)))), const t1&&>);

        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(v)), t0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(as_const(v))), const t0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(std::move(v))), t0&&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(std::move(as_const(v)))), const t0&&>);

        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(v).get()), m0&>);
        STATIC_REQUIRE(same_as<decltype(fwd_cast<t0>(std::move(v)).get()), m0&&>);
    }
}
