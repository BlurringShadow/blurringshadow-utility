#include "stdsharp/utility/dispatcher.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("dispatcher basic requirements", "[utility][dispatcher]")
{
    GIVEN("well formed dispatcher")
    {
        using dispatcher = dispatcher<expr_req::well_formed, int, int>;

        dispatcher d = [](int) noexcept { return 0; };

        REQUIRE(d(0) == 0);
    }

    GIVEN("ill formed dispatcher")
    {
        using dispatcher = dispatcher<expr_req::ill_formed, int, int>;

        dispatcher d{};

        STATIC_REQUIRE(!invocable<dispatcher, int>);
    }

    GIVEN("noexcept dispatcher")
    {
        using dispatcher = dispatcher<expr_req::no_exception, int, int>;

        dispatcher d = [](int) noexcept { return 0; };

        STATIC_REQUIRE(noexcept(d(0)));
    }
}
