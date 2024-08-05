#pragma once

#include "concepts/object.h"

namespace stdsharp::default_operator
{
    struct increase
    {
        template<std::copy_constructible T>
        [[nodiscard]] constexpr auto operator++(this T& t, int)
            noexcept(nothrow_copy_constructible<T> && noexcept(++t))
            requires requires { ++t; }
        {
            auto copied = t;
            ++t;
            return cpp_move(copied);
        }

        template<std::copy_constructible T>
        [[nodiscard]] constexpr auto operator--(this T& t, int)
            noexcept(nothrow_copy_constructible<T> && noexcept(--t))
            requires requires { --t; }
        {
            auto copied = t;
            --t;
            return cpp_move(copied);
        }
    };

    struct arithmetic : increase
    {
#define STDSHARP_ARITH_OP(op)                                                        \
    template<std::move_constructible T>                                              \
    [[nodiscard]] constexpr T operator op(this T t, auto&& u) /**/                   \
        noexcept(noexcept(t op## = cpp_forward(u)) && nothrow_move_constructible<T>) \
        requires requires { t op## = cpp_forward(u); }                               \
    {                                                                                \
        t op## = cpp_forward(u);                                                     \
        return cpp_move(t);                                                          \
    }

        STDSHARP_ARITH_OP(+)
        STDSHARP_ARITH_OP(-)
        STDSHARP_ARITH_OP(*)
        STDSHARP_ARITH_OP(/)
        STDSHARP_ARITH_OP(%)
        STDSHARP_ARITH_OP(&)
        STDSHARP_ARITH_OP(|)
        STDSHARP_ARITH_OP(^)
        STDSHARP_ARITH_OP(<<)
        STDSHARP_ARITH_OP(>>)

#undef STDSHARP_ARITH_OP
    };

    struct unary_plus
    {
        [[nodiscard]] constexpr decltype(auto) operator+(this auto&& t) noexcept
        {
            return cpp_forward(t);
        }
    };

#define STDSHARP_ARITH_OP(name, op)                                      \
    struct name##_commutative                                            \
    {                                                                    \
        template<not_decay_derived<name##_commutative> T>                \
        [[nodiscard]] friend constexpr decltype(auto                     \
        ) operator op(T&& u, decay_derived<name##_commutative> auto&& t) \
            noexcept(noexcept(cpp_forward(t) op cpp_forward(u)))         \
            requires requires { cpp_forward(t) op cpp_forward(u); }      \
        {                                                                \
            return cpp_forward(t) op cpp_forward(u);                     \
        }                                                                \
    }

    STDSHARP_ARITH_OP(plus, +);
    STDSHARP_ARITH_OP(minus, -);
    STDSHARP_ARITH_OP(multiply, *);
    STDSHARP_ARITH_OP(divide, /);
    STDSHARP_ARITH_OP(modulus, %);
    STDSHARP_ARITH_OP(bitwise_and, &);
    STDSHARP_ARITH_OP(bitwise_or, |);
    STDSHARP_ARITH_OP(bitwise_xor, ^);
    STDSHARP_ARITH_OP(bitwise_left_shift, <<);
    STDSHARP_ARITH_OP(bitwise_right_shift, >>);

#undef STDSHARP_ARITH_OP

    struct subscript
    {
        // TODO: multidimensional subscript
#if __cpp_multidimensional_subscript >= 202110L
        [[nodiscard]] constexpr decltype(auto) operator[](
            this auto&& t,
            auto&& first_arg,
            auto&&... args //
        ) noexcept(noexcept(cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...]))
            requires requires {
                requires sizeof...(args) > 0;
                cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...];
            }
        {
            return cpp_forward(t)[cpp_forward(first_arg)][cpp_forward(args)...];
        }
#else
        void operator[](const auto&) = delete;

#endif
    };

    struct arrow
    {
        template<typename T>
        [[nodiscard]] constexpr auto* operator->(this T&& t)
            noexcept(noexcept(std::addressof(*cpp_forward(t))))
            requires requires { std::addressof(*cpp_forward(t)); }
        {
            return std::addressof(*cpp_forward(t));
        }

        template<typename T>
        [[nodiscard]] constexpr auto operator->*(this T&& t, auto&& ptr)
            noexcept(noexcept((*cpp_forward(t)).*(cpp_forward(ptr))))
            requires requires { (*cpp_forward(t)).*(cpp_forward(ptr)); }
        {
            return (*cpp_forward(t)).*(cpp_forward(ptr));
        }
    };
}
