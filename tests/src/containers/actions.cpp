#include "stdsharp/containers/actions.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

using namespace stdsharp::containers;

template<associative_like_container Container>
consteval auto erase_req_f()
{
    return requires(Container container) {
        containers::erase(container, declval<typename Container::key_type>());
    };
}

template<sequence_container Container>
consteval auto erase_req_f()
{
    return requires(Container container) {
        containers::erase(container, declval<typename Container::value_type>());
    };
}

template<typename Container>
concept erase_req = requires(
    Container container,
    Container::const_iterator iter,
    bool (&predicate)(Container::const_reference)
) {
    requires erase_req_f<Container>();
    containers::erase(container, iter);
    containers::erase(container, iter, iter);
    containers::erase_if(container, predicate);
};

TEMPLATE_TEST_CASE(
    "Scenario: erase actions",
    "[containers][actions]",
    vector<int>,
    set<int>,
    (map<int, int>),
    (unordered_map<int, int>)
)
{
    STATIC_REQUIRE(erase_req<TestType>);
}

template<typename Container>
    requires associative_container<Container> || unordered_associative_container<Container>
consteval auto emplace_req_f()
{
    return requires(Container container) { emplace(container, *container.cbegin()); };
}

template<sequence_container Container>
consteval auto emplace_req_f()
{
    return requires(Container container, Container::value_type v) {
        emplace(container, container.cbegin(), v);
    };
}

template<typename Container>
concept emplace_req = emplace_req_f<Container>();

TEMPLATE_TEST_CASE(
    "Scenario: emplace actions",
    "[containers][actions]",
    vector<int>,
    set<int>,
    (map<int, int>),
    (unordered_map<int, int>)
)
{
    STATIC_REQUIRE(emplace_req<TestType>);
}

TEMPLATE_TEST_CASE("Scenario: emplace where actions", "[containers][actions]", vector<int>, deque<int>, list<int>)
{
    STATIC_REQUIRE(requires(TestType v, TestType::value_type value) {
        emplace_back(v, value);
        emplace_front(v, value);
    });
}

TEMPLATE_TEST_CASE("Scenario: pop where actions", "[containers][actions]", vector<int>, deque<int>, list<int>)
{
    STATIC_REQUIRE(requires(TestType v) {
        pop_back(v);
        pop_front(v);
    });
}

TEMPLATE_TEST_CASE("Scenario: resize actions", "[containers][actions]", vector<int>, list<int>)
{
    STATIC_REQUIRE(requires(TestType v) { resize(v, 5); });
}
