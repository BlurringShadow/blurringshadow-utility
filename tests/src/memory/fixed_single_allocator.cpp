#include "stdsharp/memory/fixed_single_allocator.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("allocator", "[memory][fixed single allocator]")
{
    struct base
    {
        base() = default;
        base(const base&) = default;
        base(base&&) = default;
        base& operator=(const base&) = default;
        base& operator=(base&&) = default;
        virtual ~base() = default;
        [[nodiscard]] constexpr virtual int foo() const = 0;
    };

    struct derived : base
    {
        int v = 0;

        constexpr derived(const int v): v(v) {}

        [[nodiscard]] constexpr int foo() const override { return v; }
    };

    fixed_single_resource<sizeof(derived)> rsc;

    GIVEN("allocator with " << decltype(rsc)::size << " bytes")
    {
        auto allocator = make_fixed_single_allocator<derived>(rsc);

        THEN("constructing a derived type at allocator")
        {
            const auto p1 = allocator.allocate(1);

            const base* const ptr = p1;
            constexpr int v = 42;

            construct_at(p1, v);

            REQUIRE(ptr->foo() == v);

            allocator.deallocate(p1, 1);
        }

        THEN("constructing a derived type at allocator")
        {
            const auto p1 = allocator.allocate(1);

            const derived* const ptr = p1;
            constexpr int v = 42;

            construct_at(p1, v);

            REQUIRE(ptr->foo() == v);

            allocator.deallocate(p1, 1);
        }

        THEN("allocate memory more then its size should throws")
        {
            REQUIRE_THROWS_AS(allocator.allocate(decltype(allocator)::size + 1), bad_alloc);
        }
    }
}
