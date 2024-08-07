#include "stdsharp/memory/allocator_reference.h"
#include "stdsharp/memory/composed_allocator.h"
#include "stdsharp/memory/fixed_single_allocator.h"
#include "test.h"

#include <ranges>
#include <vector>

STDSHARP_TEST_NAMESPACES;

template<typename T>
struct test_allocator : allocator<T>
{
    vector<void*> allocated;
    vector<void*> deallocated;

    T* allocate(const size_t n)
    {
        const auto ptr = allocator<T>::allocate(n);
        const auto void_p = to_void_pointer(ptr);

        allocated.emplace_back(void_p);
        return ptr;
    }

    void deallocate(T* p, const size_t n) noexcept
    {
        const auto void_p = to_void_pointer(p);

        allocator<T>::deallocate(p, n);
        deallocated.emplace_back(void_p);
    }

    void leak_check() const noexcept
    {
        REQUIRE_THAT(allocated, Catch::Matchers::RangeEquals(deallocated));
    }
};

SCENARIO("allocate memory", "[memory][composed_allocator]")
{
    STATIC_REQUIRE(allocator_req<test_allocator<int>>);

    GIVEN("an allocator tuple")
    {
        fixed_single_resource<sizeof(int) * 4> rsc;
        test_allocator<int> test_alloc;

        composed_allocator alloc{
            make_fixed_single_allocator<int>(rsc),
            allocator_reference{test_alloc}
        };

        WHEN("allocate a int")
        {
            const auto ptr = alloc.allocate(1);

            *ptr = 42;

            REQUIRE(*ptr == 42);

            alloc.deallocate(ptr, 1);
        }

        constexpr auto count = 5;
        WHEN("allocate ints")
        {
            const auto ptr = alloc.allocate(count);

            THEN("write values")
            {
                constexpr auto data = views::iota(0, count);
                std::ranges::copy(data, ptr);
                REQUIRE_THAT(data, Catch::Matchers::RangeEquals(span(ptr, count)));
            }

            alloc.deallocate(ptr, count);

            test_alloc.leak_check();
        }
    }
}
