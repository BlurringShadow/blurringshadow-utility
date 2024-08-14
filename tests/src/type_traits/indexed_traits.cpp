#include "stdsharp/tuple/get.h"
#include "stdsharp/type_traits/indexed_traits.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("indexed traits", "[type traits][indexed traits]")
{
    GIVEN("type at")
    {
        STATIC_REQUIRE(same_as<type_at<0, int, float, char, int>, int>);
        STATIC_REQUIRE(same_as<type_at<1, int, float, char, int>, float>);
    }

    GIVEN("indexed values type")
    {
        using indexed_values = indexed_values<int, float>;

        THEN("type is constructible")
        {
            STATIC_REQUIRE(default_initializable<indexed_values>);
            STATIC_REQUIRE(constructible_from<indexed_values, int, float>);
            STATIC_REQUIRE(constructible_from<indexed_values, const int&, const float&>);
        }

        [[maybe_unused]] indexed_values values;

        THEN("indexed value getter type")
        {
            STATIC_REQUIRE(same_as<decltype(values.get<0>()), int&>);
            STATIC_REQUIRE(same_as<decltype(values.cget<0>()), const int&>);
            STATIC_REQUIRE(same_as<decltype(as_const(values).get<0>()), const int&>);
            STATIC_REQUIRE(same_as<decltype(cpp_move(values).get<0>()), int&&>);
            STATIC_REQUIRE(same_as<decltype(cpp_move(values).cget<0>()), const int&&>);
            STATIC_REQUIRE(same_as<decltype(cpp_move(as_const(values)).get<0>()), const int&&>);

            STATIC_REQUIRE(same_as<decltype(cpo::get_element<0>(values)), int&>);
            STATIC_REQUIRE(same_as<get_element_t<0, indexed_values&>, int&>);
            STATIC_REQUIRE(same_as<decltype(cpo::cget_element<0>(values)), const int&>);
            STATIC_REQUIRE(same_as<cget_element_t<0, indexed_values&>, const int&>);
        }
    }
}
