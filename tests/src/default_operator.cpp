#include "stdsharp/default_operator.h"
#include "test.h"

STDSHARP_TEST_NAMESPACES;

using namespace default_operator;

template<typename T>
void arithmetic_test([[maybe_unused]] T v = {})
{
    STATIC_REQUIRE(requires { v += 1; });
    STATIC_REQUIRE(requires { v -= 1; });
    STATIC_REQUIRE(requires { v *= 1; });
    STATIC_REQUIRE(requires { v /= 1; });
    STATIC_REQUIRE(requires { v %= 1; });
    STATIC_REQUIRE(requires { v &= 1; });
    STATIC_REQUIRE(requires { v |= 1; });
    STATIC_REQUIRE(requires { v ^= 1; });
    STATIC_REQUIRE(requires { v <<= 1; });
    STATIC_REQUIRE(requires { v >>= 1; });

    STATIC_REQUIRE(requires { v + 1; });
    STATIC_REQUIRE(requires { v - 1; });
    STATIC_REQUIRE(requires { v * 1; });
    STATIC_REQUIRE(requires { v / 1; });
    STATIC_REQUIRE(requires { v % 1; });
    STATIC_REQUIRE(requires { v & 1; });
    STATIC_REQUIRE(requires { v | 1; });
    STATIC_REQUIRE(requires { v ^ 1; });
    STATIC_REQUIRE(requires { v << 1; });
    STATIC_REQUIRE(requires { v >> 1; });
}

template<typename T>
void commutative_arithmetic_test([[maybe_unused]] T v = {})
{
    STATIC_REQUIRE(requires { 1 + v; });
    STATIC_REQUIRE(requires { 1 - v; });
    STATIC_REQUIRE(requires { 1 * v; });
    STATIC_REQUIRE(requires { 1 / v; });
    STATIC_REQUIRE(requires { 1 % v; });
    STATIC_REQUIRE(requires { 1 & v; });
    STATIC_REQUIRE(requires { 1 | v; });
    STATIC_REQUIRE(requires { 1 ^ v; });
    STATIC_REQUIRE(requires { 1 << v; });
    STATIC_REQUIRE(requires { 1 >> v; });
    STATIC_REQUIRE(requires { +v; });
}

struct increase_t : increase
{
    using increase::operator++;
    using increase::operator--;

    increase_t& operator++();

    increase_t& operator--();
};

SCENARIO("incrementable", "[default operator]")
{
    STATIC_REQUIRE(requires(increase_t v) { v++; });
    STATIC_REQUIRE(requires(increase_t v) { v--; });
}

struct arith : arithmetic
{
    arith& operator+=(int);
    arith& operator-=(int);
    arith& operator*=(int);
    arith& operator/=(int);
    arith& operator%=(int);
    arith& operator&=(int);
    arith& operator|=(int);
    arith& operator^=(int);
    arith& operator<<=(int);
    arith& operator>>=(int);
};

SCENARIO("arithmetic", "[default operator]") { arithmetic_test<arith>(); }

struct comm :
    arith,
    unary_plus,
    plus_commutative,
    minus_commutative,
    multiply_commutative,
    divide_commutative,
    modulus_commutative,
    bitwise_and_commutative,
    bitwise_or_commutative,
    bitwise_xor_commutative,
    bitwise_left_shift_commutative,
    bitwise_right_shift_commutative
{
    using arith::operator+;
    using unary_plus::operator+;
};

SCENARIO("commutative arithmetic", "[default operator]")
{
    arithmetic_test<comm>();
    commutative_arithmetic_test<comm>();
}

SCENARIO("commutative arithmetic with int", "[default operator]")
{
    struct t : comm
    {
        int v;
    };

    arithmetic_test<t>();
    commutative_arithmetic_test<t>();
}

SCENARIO("arrow", "[default operator]")
{
    struct t0
    {
        int v;
    };

    [[maybe_unused]] struct t1 : arrow
    {
        t0* p;

        [[nodiscard]] auto& operator*() const { return *p; }
    } v{};

    [[maybe_unused]] auto mem_ptr = &t0::v;

    STATIC_REQUIRE(requires { v->v; });
    STATIC_REQUIRE(requires { v->*mem_ptr; });
}

// TODO: multidimensional subscript
#if __cpp_multidimensional_subscript >= 202110L
struct subscript_t : subscript
{
    using subscript::operator[];

    subscript_t operator[](int) const;
};

SCENARIO("subscript", "[default operator]")
{
    STATIC_REQUIRE(requires(subscript_t v) { v[0, 1]; });
}
#endif
