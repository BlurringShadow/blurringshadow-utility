#include "stdsharp/functional/pipeable.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("pipeable, [functional][pipeable]")
{
    {
        constexpr auto fn = make_pipeable<pipe_mode::left>(identity{});

        fn(1);

        STATIC_REQUIRE((1 | fn) == 1);
    }

    {
        constexpr auto fn = make_pipeable<pipe_mode::right>(identity{});

        STATIC_REQUIRE((fn | 1) == 1);
    }
}
