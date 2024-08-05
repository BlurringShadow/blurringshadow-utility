#include "stdsharp/cstdint/cstdint.h"
#include "stdsharp/memory/allocation_value.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

template<typename ValueType>
struct test_data
{
    using allocator_type = allocator<stdsharp::byte>;
    using allocation_traits = allocation_traits<allocator_type>;
    static constexpr array<int, 3> data{1, 2, 3};

    using allocation_value_t = allocation_value<allocator_type, ValueType>;
};

struct vector_data : test_data<vector<int>>
{
    static constexpr allocation_value_t allocation_value{};
    static constexpr size_t allocation_size = sizeof(vector<int>);
    static constexpr std::ranges::empty_view<int> default_value{};

    static constexpr void set_value(const auto& allocation_value, const auto& allocation)
    {
        allocation_value.get(allocation) = {data.begin(), data.end()};
    }

    static constexpr void
        construct_value(auto& allocator, const auto& allocation_value, const auto& allocation)
    {
        allocation_value(allocator, allocation, {}, data.begin(), data.end());
    }
};

struct array_data : test_data<int[]> // NOLINT(*-arrays)
{
    static constexpr allocation_value_t allocation_value{3};
    static constexpr size_t allocation_size = data.size() * sizeof(int);
    static constexpr std::ranges::repeat_view<int, int> default_value{0, 3};

    static constexpr void set_value(const auto& allocation_value, const auto& allocation)
    {
        std::ranges::copy(data, allocation_value.data(allocation));
    }

    static constexpr void
        construct_value(auto& allocator, const auto& allocation_value, const auto& allocation)
    {
        allocation_value(allocator, allocation, {}, data.begin());
    }
};

TEMPLATE_TEST_CASE(
    "Scenario: allocation value",
    "[memory][allocation_value]",
    vector_data,
    array_data
)
{
    using allocation_traits = TestType::allocation_traits;

    typename TestType::allocator_type allocator;
    auto allocation_value = TestType::allocation_value;
    const auto allocation_1 = allocation_traits::allocate(allocator, TestType::allocation_size);
    typename allocation_traits::callocation_result callocation_1 = allocation_1;
    const auto allocation_2 = allocation_traits::allocate(allocator, TestType::allocation_size);

    WHEN("default construct")
    {
        allocation_value(allocator, allocation_1, {});

        THEN("value is default")
        {
            REQUIRE_THAT(
                allocation_value.get(allocation_1),
                Catch::Matchers::RangeEquals(TestType::default_value)
            );
        }

        THEN("set the value to int sequence")
        {
            TestType::set_value(allocation_value, allocation_1);
            REQUIRE_THAT(
                allocation_value.get(allocation_1),
                Catch::Matchers::RangeEquals(TestType::data)
            );
        }

        allocation_value(allocator, allocation_1);
    }

    WHEN("constructs and set value to seq")
    {
        TestType::construct_value(allocator, allocation_value, allocation_1);

        THEN("copy construct")
        {
            allocation_value(allocator, callocation_1, allocation_2);
            THEN("value is seq")
            {
                REQUIRE_THAT(
                    allocation_value.get(allocation_2),
                    Catch::Matchers::RangeEquals(TestType::data)
                );
            }
            allocation_value(allocator, allocation_2);
        }

        THEN("move construct")
        {
            allocation_value(allocator, allocation_1, allocation_2);
            REQUIRE_THAT(
                allocation_value.get(allocation_2),
                Catch::Matchers::RangeEquals(TestType::data)
            );
            allocation_value(allocator, allocation_2);
        }

        THEN("copy assign")
        {
            allocation_value(allocator, allocation_2, {});
            allocation_value(callocation_1, allocation_2);
            REQUIRE_THAT(
                allocation_value.get(allocation_2),
                Catch::Matchers::RangeEquals(TestType::data)
            );
            allocation_value(allocator, allocation_2);
        }

        THEN("move assign")
        {
            allocation_value(allocator, allocation_2, {});
            allocation_value(allocation_1, allocation_2);
            REQUIRE_THAT(
                allocation_value.get(allocation_2),
                Catch::Matchers::RangeEquals(TestType::data)
            );
            allocation_value(allocator, allocation_2);
        }

        allocation_value(allocator, allocation_1);
    }

    allocation_traits::deallocate(allocator, array{allocation_1, allocation_2});
}
