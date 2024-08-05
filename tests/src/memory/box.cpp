#include "box.h"
#include "stdsharp/memory/box.h"

STDSHARP_TEST_NAMESPACES;

using allocator_t = allocator<unsigned char>;

SCENARIO("box basic requirements", "[memory][box]")
{
    using normal_t = normal_box<allocator_t>;
    using unique_t = unique_box<allocator_t>;
    using worst_t = box_for<test_worst_type, allocator_t>;

    STATIC_REQUIRE(default_initializable<box_for<int, allocator_t>>);
    STATIC_REQUIRE(default_initializable<trivial_box<allocator_t>>);
    STATIC_REQUIRE(default_initializable<normal_t>);
    STATIC_REQUIRE(default_initializable<unique_t>);
    STATIC_REQUIRE(default_initializable<worst_t>);

    ALLOCATION_TYPE_REQUIRE(normal_t, unique_t, worst_t);
}

struct int_test_data
{
    int value = 42;
};

struct vector_test_data
{
    vector<unsigned> value{1, 2, 3};
};

struct array_test_data
{
    array<unsigned, 3> value{1, 2, 3};
};

TEMPLATE_TEST_CASE(
    "Scneario: box emplace value",
    "[memory][box]",
    int_test_data,
    vector_test_data,
    array_test_data
)
{
    BOX_EMPLACE_TEST(normal_box<allocator_t>{});
}

SCENARIO("box assign value", "[memory][box]")
{
    GIVEN("an object allocation")
    {
        normal_box<allocator_t> box_v;
        auto invoked = 0u;

        struct local : reference_wrapper<unsigned>
        {
            local(unsigned& value): reference_wrapper(value) { ++get(); }
        };

        WHEN("assign custom type twice")
        {
            box_v.emplace<local>(invoked);
            box_v.emplace<local>(invoked);

            THEN("assign operator should be invoked") { REQUIRE(invoked == 2); }

            AND_THEN("destroy allocation and check content")
            {
                box_v.reset();
                REQUIRE(!box_v.has_value());
            }
        }
    }
}

SCENARIO("constexpr box", "[memory][box]")
{
    STATIC_REQUIRE(
        []
        {
            trivial_box<allocator<int>> b{};
            auto& value = b.emplace(1);
            value = 42;
            return b.get<int>();
        }() == 42
    );
}
