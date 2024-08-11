#include "stdsharp/cstdint/cstdint.h"
#include "stdsharp/memory/allocation_adaptor.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

template<typename ValueType>
struct test_data
{
    using allocator_type = allocator<stdsharp::byte>;
    using allocation_traits = allocation_traits<allocator_type>;
    static constexpr array<int, 3> data{1, 2, 3};

    using allocation_adaptor_t = allocation_adaptor<allocator_type, ValueType>;
};

struct vector_data : test_data<vector<int>>
{
    static constexpr allocation_adaptor_t allocation_adaptor{};
    static constexpr size_t allocation_size = sizeof(vector<int>);
    static constexpr std::ranges::empty_view<int> default_value{};

    static constexpr void set_value(const auto& allocation_adaptor, const auto& allocation)
    {
        allocation_adaptor.get(allocation) = {data.begin(), data.end()};
    }

    static constexpr void
        construct_value(auto& allocator, const auto& allocation_adaptor, const auto& allocation)
    {
        allocation_adaptor(allocator, allocation, {}, data.begin(), data.end());
    }
};

struct array_data : test_data<int[]> // NOLINT(*-arrays)
{
    static constexpr allocation_adaptor_t allocation_adaptor{3};
    static constexpr size_t allocation_size = data.size() * sizeof(int);
    static constexpr std::ranges::repeat_view<int, int> default_value{0, 3};

    static constexpr void set_value(const auto& allocation_adaptor, const auto& allocation)
    {
        std::ranges::copy(data, allocation_adaptor.data(allocation));
    }

    static constexpr void
        construct_value(auto& allocator, const auto& allocation_adaptor, const auto& allocation)
    {
        allocation_adaptor(allocator, allocation, {}, data.begin());
    }
};

TEMPLATE_TEST_CASE(
    "Scenario: allocation value",
    "[memory][allocation_adaptor]",
    vector_data,
    array_data
)
{
    using allocation_traits = TestType::allocation_traits;

    typename TestType::allocator_type allocator;
    auto allocation_adaptor = TestType::allocation_adaptor;
    const auto allocation_1 = allocation_traits::allocate(allocator, TestType::allocation_size);
    typename allocation_traits::callocation callocation_1 = allocation_1;
    const auto allocation_2 = allocation_traits::allocate(allocator, TestType::allocation_size);

    WHEN("default construct")
    {
        allocation_adaptor(allocator, allocation_1, {});

        THEN("value is default")
        {
            REQUIRE_THAT(
                allocation_adaptor.get(allocation_1),
                Catch::Matchers::RangeEquals(TestType::default_value)
            );
        }

        THEN("set the value to int sequence")
        {
            TestType::set_value(allocation_adaptor, allocation_1);
            REQUIRE_THAT(
                allocation_adaptor.get(allocation_1),
                Catch::Matchers::RangeEquals(TestType::data)
            );
        }

        allocation_adaptor(allocator, allocation_1);
    }

    WHEN("constructs and set value to seq")
    {
        TestType::construct_value(allocator, allocation_adaptor, allocation_1);

        THEN("copy construct")
        {
            allocation_adaptor(allocator, callocation_1, allocation_2);
            THEN("value is seq")
            {
                REQUIRE_THAT(
                    allocation_adaptor.get(allocation_2),
                    Catch::Matchers::RangeEquals(TestType::data)
                );
            }
            allocation_adaptor(allocator, allocation_2);
        }

        THEN("move construct")
        {
            allocation_adaptor(allocator, allocation_1, allocation_2);
            REQUIRE_THAT(
                allocation_adaptor.get(allocation_2),
                Catch::Matchers::RangeEquals(TestType::data)
            );
            allocation_adaptor(allocator, allocation_2);
        }

        THEN("copy assign")
        {
            allocation_adaptor(allocator, allocation_2, {});
            allocation_adaptor(callocation_1, allocation_2);
            REQUIRE_THAT(
                allocation_adaptor.get(allocation_2),
                Catch::Matchers::RangeEquals(TestType::data)
            );
            allocation_adaptor(allocator, allocation_2);
        }

        THEN("move assign")
        {
            allocation_adaptor(allocator, allocation_2, {});
            allocation_adaptor(allocation_1, allocation_2);
            REQUIRE_THAT(
                allocation_adaptor.get(allocation_2),
                Catch::Matchers::RangeEquals(TestType::data)
            );
            allocation_adaptor(allocator, allocation_2);
        }

        allocation_adaptor(allocator, allocation_1);
    }

    allocation_traits::deallocate(allocator, array{allocation_1, allocation_2});
}
