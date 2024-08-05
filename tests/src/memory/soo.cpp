#include "box.h"
#include "stdsharp/memory/soo.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("soo allocation basic requirements", "[memory][small object optimization]")
{
    using normal_t = normal_soo_box<>;
    using unique_t = unique_soo_box<>;
    using worst_t = soo_box_for<test_worst_type>;

    STATIC_REQUIRE(constructible_from<trivial_soo_box<>, trivial_soo_box<>::allocator_type>);
    STATIC_REQUIRE(constructible_from<normal_t, normal_t::allocator_type>);
    STATIC_REQUIRE(constructible_from<unique_t, normal_t::allocator_type>);
    STATIC_REQUIRE(constructible_from<worst_t, normal_t::allocator_type>);

    ALLOCATION_TYPE_REQUIRE(normal_t, unique_t, worst_t);
}

struct vector_test_data
{
    vector<unsigned> value{1, 2, 3};
};

TEMPLATE_TEST_CASE(
    "Scenario: soo box emplace value",
    "[memory][small object optimization]",
    vector_test_data
)
{
    fixed_single_resource<> buffer{};

    BOX_EMPLACE_TEST(normal_soo_box<>{make_soo_allocator(buffer, {})})
}
