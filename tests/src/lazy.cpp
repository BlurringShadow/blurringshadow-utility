#include "stdsharp/lazy.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("lazy", "[lazy]")
{
    GIVEN("a lazy value")
    {
        lazy lazy{[]() { return string{"foo"}; }};

        WHEN("get value")
        {
            auto value = lazy.get();

            THEN("value is 1") { REQUIRE(value == "foo"); }
        }
    }

    GIVEN("constexpr test")
    {
        constexpr auto value = 43;

        STATIC_REQUIRE(
            []
            {
                lazy lazy{[]() { return value; }};
                return lazy.get();
            }() == value
        );
    }
}
