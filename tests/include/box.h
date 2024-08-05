#pragma once

#include "test.h"

#include <stdsharp/concepts/concepts.h>

struct test_worst_type
{
    test_worst_type() = default;
    ~test_worst_type() = default;

private:
    test_worst_type(const test_worst_type&) = default;
    test_worst_type(test_worst_type&&) = default;
    test_worst_type& operator=(const test_worst_type&) = default;
    test_worst_type& operator=(test_worst_type&&) = default;
};

#define ALLOCATION_TYPE_REQUIRE(Normal, Unique, Worst) \
    STATIC_REQUIRE(copyable<Normal>);                  \
    STATIC_REQUIRE(nothrow_movable<Unique>);           \
    STATIC_REQUIRE(nothrow_swappable<Unique>);         \
    STATIC_REQUIRE(nothrow_movable<Worst>);            \
    STATIC_REQUIRE(nothrow_swappable<Worst>)

#define BOX_EMPLACE_TEST(...)                                        \
    GIVEN("a box")                                                   \
    {                                                                \
        TestType data;                                               \
        auto box_v = __VA_ARGS__;                                    \
        WHEN("emplace the value")                                    \
        {                                                            \
            const auto& const_value = data.value;                    \
            const auto& res = box_v.emplace(const_value);            \
            THEN("check the value") { REQUIRE(res == const_value); } \
            AND_THEN("type should be expected")                      \
            {                                                        \
                REQUIRE(box_v.is_type<decltype(data.value)>());      \
            }                                                        \
        }                                                            \
    }
