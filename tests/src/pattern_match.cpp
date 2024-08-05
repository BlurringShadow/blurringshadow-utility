#include "stdsharp/pattern_match.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("pattern match", "[pattern match]")
{
    enum class my_enum : uint8_t
    {
        one,
        two,
        three
    };

    GIVEN(
        R"(enum class has three values: "one", "two", "three" and three cases matches separate
        value)"
    )
    {
        THEN("case 1 match one, case 2 match two, case 3 match three")
        {
            constexpr auto match = []() consteval
            {
                my_enum matched{};
                const auto& matched_assign = [&matched](const my_enum value) { matched = value; };

                pattern_match(
                    my_enum::two,
                    pair{my_enum::one, matched_assign},
                    pair{my_enum::two, matched_assign},
                    pair{my_enum::three, matched_assign}
                );

                return matched;
            }();

            STATIC_REQUIRE(match == my_enum::two);
        }

        AND_THEN("same for constexpr pattern match")
        {
            STATIC_REQUIRE( //
                requires //
                {
                    constexpr_pattern_match::
                        from_constant<my_enum::two>([](const constant<my_enum::two>) {});
                }
            );
        }
    }
}
