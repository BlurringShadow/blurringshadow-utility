#include "stdsharp/utility/utility.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

TEMPLATE_TEST_CASE_SIG(
    "Scenario: forward like",
    "[utility]",
    ((typename T, typename U, typename Expect, auto V), T, U, Expect, V),
    (int&, int&, int&, 0),
    (int&, int&&, int&, 0),
    (int&&, const int&, const int&&, 0),
    (int&, const int&&, const int&, 0),
    (const int&, int&, const int&, 0),
    (const int&&, int&, const int&&, 0)
)
{
    using res_t = forward_like_t<T, U>;

    STATIC_REQUIRE(same_as<res_t, Expect>);
}
