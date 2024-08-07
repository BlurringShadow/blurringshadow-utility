#include "stdsharp/memory/pointer_traits.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

SCENARIO("pointer traits for void fancy pointer", "[memory][pointer traits]")
{
    struct fancy_pointer
    {
        using element_type = void;
    };

    using traits = stdsharp::pointer_traits<fancy_pointer>;

    STATIC_REQUIRE(same_as<traits::pointer, fancy_pointer>);
    STATIC_REQUIRE(same_as<traits::difference_type, ptrdiff_t>);
    STATIC_REQUIRE(same_as<traits::element_type, void>);
    STATIC_REQUIRE( //
        !requires {
            requires( //
                []<typename T = traits> { return requires { typename T::raw_pointer; }; }()
            );
        }
    );
}

struct fancy_pointer
{
    int* ptr;

    [[nodiscard]] int& operator*() const noexcept { return *ptr; }
};

struct fancy_const_pointer
{
    const int* ptr;

    [[nodiscard]] const int& operator*() const noexcept { return *ptr; }
};

TEMPLATE_TEST_CASE_SIG(
    "Scenario: pointer traits for different int pointer types",
    "[memory][pointer traits]",
    ((typename Ptr, typename Diff, typename Element, typename RawPtr, int V),
     Ptr,
     Diff,
     Element,
     RawPtr,
     V),
    (fancy_const_pointer, ptrdiff_t, const int, const int*, 42),
    (fancy_pointer, ptrdiff_t, int, int*, 42),
    (int*, ptrdiff_t, int, int*, 42)
)
{
    using traits = stdsharp::pointer_traits<Ptr>;

    STATIC_REQUIRE(dereferenceable<Ptr>);
    STATIC_REQUIRE(same_as<typename traits::pointer, Ptr>);
    STATIC_REQUIRE(same_as<typename traits::difference_type, Diff>);
    STATIC_REQUIRE(same_as<typename traits::element_type, Element>);
    STATIC_REQUIRE(same_as<typename traits::raw_pointer, RawPtr>);

    GIVEN("a int and pointer")
    {
        int value = V;
        int* raw_ptr = &value;
        Ptr ptr{raw_ptr};

        WHEN("pointer to") { REQUIRE(*traits::pointer_to(value) == V); }
        WHEN("to pointer") { REQUIRE(*traits::to_pointer(raw_ptr) == V); }
        WHEN("to address") { REQUIRE(*traits::to_address(ptr) == V); }
        WHEN("to void pointer") { REQUIRE(to_void_pointer(raw_ptr) == to_void_pointer(ptr)); }
    }
}
