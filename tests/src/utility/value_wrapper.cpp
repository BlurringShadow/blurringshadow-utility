#include "stdsharp/utility/value_wrapper.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("value_wrapper", "[utility][value wrapper]")
{
    struct empty_type
    {
        empty_type() = default;

        empty_type(int /*unused*/) {}
    };

    STATIC_REQUIRE(default_initializable<value_wrapper<int>>);
    STATIC_REQUIRE(default_initializable<value_wrapper<empty_type>>);

    STATIC_REQUIRE(constructible_from<value_wrapper<int>, int>);
    STATIC_REQUIRE(constructible_from<value_wrapper<int>, const int&>);

    STATIC_REQUIRE(constructible_from<value_wrapper<string>, const char*>);
    STATIC_REQUIRE(constructible_from<value_wrapper<string>, string>);

    STATIC_REQUIRE(constructible_from<value_wrapper<empty_type>, int>);
    STATIC_REQUIRE(constructible_from<value_wrapper<empty_type>, empty_type>);
    STATIC_REQUIRE(constructible_from<value_wrapper<empty_type>, const empty_type&>);

    STATIC_REQUIRE(nothrow_copyable<value_wrapper<int>>);
    STATIC_REQUIRE(nothrow_swappable<value_wrapper<int>>);

    THEN("Set the wrapper value")
    {
        [[maybe_unused]] value_wrapper<int> wrapper{};

        STATIC_REQUIRE(same_as<decltype(wrapper.get()), int&>);
        STATIC_REQUIRE(same_as<decltype(cpp_move(wrapper).get()), int&&>);
        STATIC_REQUIRE(same_as<decltype(wrapper.cget()), const int&>);
        STATIC_REQUIRE(same_as<decltype(cpp_move(wrapper).cget()), const int&&>);

        STATIC_REQUIRE(
            []
            {
                value_wrapper<int> wrapper{};
                wrapper.get() = 42;
                return wrapper.get() == 42;
            }()
        );
    }
}
