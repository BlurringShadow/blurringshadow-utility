#include "stdsharp/utility/adl_proof.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

template<typename>
struct t0
{
};

template<typename T>
using t1 = adl_proof_t<t0, T>;

template<typename T>
void foo(t1<T>);

template<typename T>
concept adl_invoke_test = !requires { foo(T{}); };

SCENARIO("adl proof", "[utility][adl proof]")
{
    using type = t1<void>;

    STATIC_REQUIRE(adl_invoke_test<type>);
    STATIC_REQUIRE(same_as<adl_proof_inner_t<type>, t0<void>>);
    STATIC_REQUIRE(adl_proofed_for<type, t0>);
}
