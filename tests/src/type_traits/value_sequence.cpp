#include "stdsharp/functional/sequenced_invocables.h"
#include "stdsharp/type_traits/value_sequence.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

void foo() { value_sequence<0, 1>::for_each(stdsharp::empty); }

// using test_seq = value_sequence<0, 1, size_t{7}, 1, to_array("my literal")>;

// TEMPLATE_TEST_CASE("Scenario: value sequence default initializable", "[type traits][value
// sequence]", test_seq, value_sequence<>)
// {
//     STATIC_REQUIRE(default_initializable<TestType>);
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence get",
//     "[type traits][value sequence]",
//     ((auto Index, auto Expect), Index, Expect),
//     (0, 0),
//     (1, 1),
//     (2, 7),
//     (3, 1)
// )
// {
//     STATIC_REQUIRE(test_seq::get<Index>() == Expect);
// }

// namespace // Escape Catch2 special characters like '[' and ']'
// {
//     constexpr auto transform_functor_1 = [](const int v) mutable { return v + 1; };
//     constexpr auto transform_functor_2 = [](const int v) { return v + 42; };
//     constexpr auto transform_functor_3 = [](const size_t v) { return v + 42; };
//     constexpr auto transform_functor_4 = [](const int v) { return v + 6; };
//     constexpr auto transform_functor_5 = [](const array<char, 11>& str) { return str[0]; };
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence transform",
//     "[type traits][value sequence]",
//     ((auto... Functor), Functor...),
//     identity{},
//     ( //
//         transform_functor_1,
//         transform_functor_2,
//         transform_functor_3,
//         transform_functor_4,
//         transform_functor_5
//     )
// )
// {
//     STATIC_REQUIRE(default_initializable<test_seq::template transform_t<Functor...>>);
// }

// TEMPLATE_TEST_CASE(
//     "Scenario: value sequence invoke",
//     "[type traits][value sequence]",
//     identity,
//     decltype([](same_as_any<int, size_t, array<char, 11>> auto) {})
// )
// {
//     STATIC_REQUIRE(invocable<test_seq::invoke_fn<>, TestType>);
// }

// SCENARIO("apply_t", "[type traits][value sequence]")
// {
//     STATIC_REQUIRE(default_initializable<test_seq::apply_t<value_sequence>>);
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence append",
//     "[type traits][value sequence]",
//     ( //
//         (typename Expect, typename FrontExpect, auto... V),
//         Expect,
//         FrontExpect,
//         V...
//     ),
//     ( //
//         (regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 1, 2>),
//         regular_value_sequence<1, 2, 0, 1, size_t{7}, 1, to_array("my literal")>,
//         1,
//         2
//     ),
//     ( //
//         (regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 2, 4>),
//         regular_value_sequence<2, 4, 0, 1, size_t{7}, 1, to_array("my literal")>,
//         2,
//         4
//     )
// )
// {
//     STATIC_REQUIRE(same_as<test_seq::append_t<V...>, Expect>);
//     STATIC_REQUIRE(same_as<test_seq::append_front_t<V...>, FrontExpect>);
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence insert",
//     "[type traits][value sequence]",
//     ((auto Index, typename Expect, auto... Element), Index, Expect, Element...),
//     ( //
//         3,
//         regular_value_sequence<0, 1, size_t{7}, 1, 2, 1, to_array("my literal")>,
//         1,
//         2
//     ),
//     ( //
//         5,
//         regular_value_sequence<0, 1, size_t{7}, 1, to_array("my literal"), 2, 4>,
//         2,
//         4
//     )
// )
// {
//     STATIC_REQUIRE(same_as<test_seq::insert_t<Index, Element...>, Expect>);
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence remove at",
//     "[type traits][value sequence]",
//     ((typename Expect, auto I), Expect, I),
//     (regular_value_sequence<0, 1, 1, to_array("my literal")>, 2),
//     (regular_value_sequence<0, 1, size_t{7}, 1>, 4)
// )
// {
//     STATIC_REQUIRE(same_as<test_seq::remove_at_t<I>, Expect>);
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence find",
//     "[type traits][value sequence]",
//     ((auto V, auto Expect), V, Expect),
//     (0, 0),
//     (1, 1),
//     ('!', test_seq::size())
// )
// {
//     STATIC_REQUIRE(
//         value_sequence_algo::find<test_seq>(V, sequenced_invocables{equal_to{}, always_false}) ==
//         Expect
//     );
// }

// namespace // Escape Catch2 special characters like '[' and ']'
// {
//     struct find_if_functor_3_t
//     {
//         static constexpr sequenced_invocables value{
//             [](const size_t v) { return v == 7; },
//             always_false
//         };
//     };
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence find if",
//     "[type traits][value sequence]",
//     ((auto Fn, auto Expect), Fn, Expect),
//     (always_true, 0),
//     (always_false, test_seq::size()),
//     (find_if_functor_3_t{}, 2)
// )
// {
//     if constexpr(constant_value<decltype(Fn)>)
//     {
//         STATIC_REQUIRE(value_sequence_algo::find_if<test_seq>(Fn.value) == Expect);
//     }
//     else { STATIC_REQUIRE(value_sequence_algo::find_if<test_seq>(Fn) == Expect); }
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence count",
//     "[type traits][value sequence]",
//     ((auto V, auto Expect), V, Expect),
//     (0, 1),
//     (1, 2),
//     ('?', 0)
// )
// {
//     STATIC_REQUIRE(
//         value_sequence_algo::count<test_seq>(V, sequenced_invocables{equal_to{}, always_false})
//         == Expect
//     );
// }

// namespace // Escape Catch2 special characters like '[' and ']'
// {
//     struct count_if_functor_3_t
//     {
//         static constexpr sequenced_invocables value{
//             [](const size_t) { return true; },
//             always_false
//         };
//     };
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: value sequence count if",
//     "[type traits][value sequence]",
//     ((auto Fn, auto Expect), Fn, Expect),
//     (always_true, test_seq::size()),
//     (always_false, 0),
//     (count_if_functor_3_t{}, 4)
// )
// {
//     if constexpr(constant_value<decltype(Fn)>)
//     {
//         STATIC_REQUIRE(value_sequence_algo::count_if<test_seq>(Fn.value) == Expect);
//     }
//     else STATIC_REQUIRE(value_sequence_algo::count_if<test_seq>(Fn) == Expect);
// }

// SCENARIO("Scenario: value adjacent find", "[type traits]")
// {
//     STATIC_REQUIRE( //
//         invocable<
//             value_sequence_algo::adjacent_find_fn<test_seq>,
//             sequenced_invocables<equal_to<>, always_false_fn>> //
//     );
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: unique value sequence",
//     "[type traits][value sequence]",
//     ((auto V, typename Seq, typename Expect), V, Seq, Expect),
//     (0, value_sequence<>, regular_value_sequence<>),
//     (0, value_sequence<0, 10, 1, 5, 10, 1>, regular_value_sequence<0, 10, 1, 5>) //
// )
// {
//     STATIC_REQUIRE( //
//         same_as<
//             value_sequence_algo::unique_t<Seq, sequenced_invocables<equal_to<>,
//             always_false_fn>>, Expect> //
//     );
// }

// TEMPLATE_TEST_CASE_SIG(
//     "Scenario: reverse value sequence",
//     "[type traits][value sequence]",
//     ((auto V, typename Seq, typename Expect), V, Seq, Expect),
//     (0, value_sequence<>, regular_value_sequence<>),
//     (0, value_sequence<1>, regular_value_sequence<1>),
//     (0, value_sequence<1, 2, 3, 4>, regular_value_sequence<4, 3, 2, 1>),
//     (0, value_sequence<1, 2>, regular_value_sequence<2, 1>) //
// )
// {
//     STATIC_REQUIRE(same_as<value_sequence_algo::reverse_t<Seq>, Expect>);
// }

// SCENARIO("tuple traits for regular value sequence", "[type traits][value sequence]")
// {
//     STATIC_REQUIRE(tuple_size_v<test_seq> == 5);
//     STATIC_REQUIRE(same_as<tuple_element_t<0, test_seq>, int>);
//     STATIC_REQUIRE(same_as<tuple_element_t<2, test_seq>, size_t>);
// }

// SCENARIO("tuple traits for regular type sequence", "[type traits][value sequence]")
// {
//     {
//         using type_seq = basic_type_sequence<int, char, float>;
//         STATIC_REQUIRE(tuple_size_v<type_seq> == 3);
//     }

//     {
//         using type_seq = regular_type_sequence<int, char, float>;
//         STATIC_REQUIRE(adl_proofed_for<type_seq, basic_type_sequence>);
//         STATIC_REQUIRE(tuple_size_v<type_seq> == 3);
//     }
// }
