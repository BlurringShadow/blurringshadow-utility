#pragma once

#include "../cstdint/cstdint.h"
#include "../iterator/iterator.h"
#include "sequenced_invocables.h"

namespace stdsharp
{
    inline constexpr struct copy_fn
    {
        template<typename T>
            requires std::constructible_from<std::decay_t<T>, T>
        [[nodiscard]] constexpr std::decay_t<T> operator()(T&& t) const
            noexcept(nothrow_constructible_from<std::decay_t<T>>)
        {
            return t;
        }
    } copy{};

    template<typename Func>
    using not_fn_t = decltype(std::not_fn(std::declval<Func>()));

    namespace details
    {
        struct assign
        {
            template<typename T, typename U = T>
                requires std::assignable_from<T&, U>
            constexpr decltype(auto) operator()(T& left, U&& right) const
                noexcept(nothrow_assignable_from<T&, U>)
            {
                return left = cpp_forward(right);
            }
        };

        struct assign_by_construct
        {
            template<typename T, typename... U, typename DecayT = std::remove_reference_t<T>>
                requires std::constructible_from<DecayT, U...>
            constexpr decltype(auto) operator()(T& left, U&&... right) const
                noexcept(noexcept(left = DecayT{cpp_forward(right)...}))
            {
                return left = DecayT{cpp_forward(right)...};
            }
        };
    }

    inline constexpr struct assign :
        sequenced_invocables<details::assign, details::assign_by_construct>
    {
    } assign_v{};

    inline constexpr std::ranges::equal_to equal_to_v{};
    inline constexpr std::ranges::not_equal_to not_equal_to_v{};
    inline constexpr std::ranges::less less_v{};
    inline constexpr std::ranges::greater greater_v{};
    inline constexpr std::ranges::less_equal less_equal_v{};
    inline constexpr std::ranges::greater_equal greater_equal_v{};
    inline constexpr std::compare_three_way compare_three_way_v{};
    inline constexpr std::plus<> plus_v{};
    inline constexpr std::minus<> minus_v{};
    inline constexpr std::divides<> divides_v{};
    inline constexpr std::multiplies<> multiplies_v{};
    inline constexpr std::modulus<> modulus_v{};
    inline constexpr std::negate<> negate_v{};
    inline constexpr std::logical_and<> logical_and_v{};
    inline constexpr std::logical_not<> logical_not_v{};
    inline constexpr std::logical_or<> logical_or_v{};

    inline constexpr std::bit_and<> bit_and_v{};
    inline constexpr std::bit_not<> bit_not_v{};
    inline constexpr std::bit_or<> bit_or_v{};
    inline constexpr std::bit_xor<> bit_xor_v{};

    inline constexpr struct bit_xnor
    {
        template<typename T, typename U = T>
        constexpr decltype(auto) operator()(T&& t, U&& u) const
            noexcept(noexcept(bit_not_v(bit_xor_v(cpp_forward(t), cpp_forward(u)))))
            requires requires { bit_not_v(bit_xor_v(cpp_forward(t), cpp_forward(u))); }
        {
            return bit_not_v(bit_xor_v(cpp_forward(t), cpp_forward(u)));
        }
    } bit_xnor_v{};

#define SHARP_SHIFT_OPERATE(direction, operate)                                      \
    inline constexpr struct direction##_shift                                        \
    {                                                                                \
        template<typename T, typename U = T>                                         \
        [[nodiscard]] constexpr decltype(auto) operator()(T&& left, U&& right) const \
            noexcept(noexcept(cpp_forward(left) operate cpp_forward(right)))         \
            requires requires { cpp_forward(left) operate cpp_forward(right); }      \
        {                                                                            \
            return cpp_forward(left) operate cpp_forward(right);                     \
        }                                                                            \
    } direction##_shift_v{};

    SHARP_SHIFT_OPERATE(left, <<)
    SHARP_SHIFT_OPERATE(right, >>)

#undef SHARP_SHIFT_OPERATE

#define STDSHARP_ASSIGN_OPERATE(operator_type, op)                                               \
                                                                                                 \
    template<typename T, typename U>                                                             \
    concept operator_type##_assignable_from = requires(T t, U&& u) { t op## = cpp_forward(u); }; \
                                                                                                 \
    namespace details                                                                            \
    {                                                                                            \
        struct operator_type##_assign                                                            \
        {                                                                                        \
            template<typename T, typename U = T>                                                 \
                requires(operator_type##_assignable_from<T, U>)                                  \
            constexpr decltype(auto) operator()(T& t, U&& u) const                               \
                noexcept(noexcept((t op## = cpp_forward(u))))                                    \
            {                                                                                    \
                return t op## = cpp_forward(u);                                                  \
            }                                                                                    \
        };                                                                                       \
                                                                                                 \
        struct indirect_##operator_type##_assign                                                 \
        {                                                                                        \
            template<typename T, typename U = T>                                                 \
                requires requires(T t, U&& u) { t = operator_type##_v(t, cpp_forward(u)); }      \
            constexpr decltype(auto) operator()(T& t, U&& u) const                               \
                noexcept(noexcept((t = operator_type##_v(t, cpp_forward(u)))))                   \
            {                                                                                    \
                return t = operator_type##_v(t, cpp_forward(u));                                 \
            }                                                                                    \
        };                                                                                       \
    }                                                                                            \
                                                                                                 \
    using operator_type##_assign = sequenced_invocables<                                         \
        details::operator_type##_assign,                                                         \
        details::indirect_##operator_type##_assign>;                                             \
                                                                                                 \
    inline constexpr operator_type##_assign operator_type##_assign_v{};

    STDSHARP_ASSIGN_OPERATE(plus, +)
    STDSHARP_ASSIGN_OPERATE(minus, -)
    STDSHARP_ASSIGN_OPERATE(divides, /)
    STDSHARP_ASSIGN_OPERATE(multiplies, *)
    STDSHARP_ASSIGN_OPERATE(modulus, %)
    STDSHARP_ASSIGN_OPERATE(bit_and, &)
    STDSHARP_ASSIGN_OPERATE(bit_or, |)
    STDSHARP_ASSIGN_OPERATE(bit_xor, ^)
    STDSHARP_ASSIGN_OPERATE(left_shift, <<)
    STDSHARP_ASSIGN_OPERATE(right_shift, >>)

#undef STDSHARP_ASSIGN_OPERATE

#define STDSHARP_ASSIGN_OPERATE(operator_type)                                          \
    inline constexpr struct operator_type##_assign                                      \
    {                                                                                   \
        template<typename T, typename U = T>                                            \
            requires requires(T t, U&& u) { t = operator_type##_v(t, cpp_forward(u)); } \
        constexpr decltype(auto) operator()(T& t, U&& u) const                          \
            noexcept(noexcept((t = operator_type##_v(t, cpp_forward(u)))))              \
        {                                                                               \
            return t = operator_type##_v(t, cpp_forward(u));                            \
        }                                                                               \
    } operator_type##_assign_v{};

    STDSHARP_ASSIGN_OPERATE(negate)
    STDSHARP_ASSIGN_OPERATE(logical_and)
    STDSHARP_ASSIGN_OPERATE(logical_not)
    STDSHARP_ASSIGN_OPERATE(logical_or)

#undef STDSHARP_ASSIGN_OPERATE

    template<typename T>
    struct identity_with_fn
    {
        constexpr decltype(auto) operator()(T&& t) const noexcept { return cpp_forward(t); }
    };

    template<typename T>
    inline constexpr identity_with_fn<T> identity_with_v{};

    inline constexpr std::identity identity_v{};

#define STDSHARP_INC_DEC_OPERATE(operator_prefix, op, al_op)                                       \
    inline constexpr struct pre_##operator_prefix##crease                                          \
    {                                                                                              \
        template<typename T>                                                                       \
            requires requires(T t) { op##op t; }                                                   \
        constexpr decltype(auto) operator()(T& v) const noexcept(noexcept(op##op v))               \
        {                                                                                          \
            return op##op v;                                                                       \
        }                                                                                          \
    } pre_##operator_prefix##crease_v{};                                                           \
                                                                                                   \
    inline constexpr struct post_##operator_prefix##crease                                         \
    {                                                                                              \
        template<weakly_decrementable T>                                                           \
            requires requires(T t) { t op##op; }                                                   \
        [[nodiscard]] constexpr decltype(auto) operator()(T& v) const noexcept(noexcept(v op##op)) \
        {                                                                                          \
            return v op##op;                                                                       \
        }                                                                                          \
    } post_##operator_prefix##crease_v{};

    STDSHARP_INC_DEC_OPERATE(in, +, plus)
    STDSHARP_INC_DEC_OPERATE(de, -, minus)

#undef STDSHARP_INC_DEC_OPERATE

    namespace details
    {
        struct advance_by_op
        {
            template<typename T, std::unsigned_integral Distance = std::iter_difference_t<T>>
                requires std::invocable<pre_increase, T>
            constexpr decltype(auto) operator()(T& v, Distance distance) const
                noexcept(nothrow_invocable<pre_increase, T>)
            {
                if(distance == 0) return v;
                for(; distance > 0; --distance) pre_increase_v(v);
                return v;
            }

            template<typename T, std::signed_integral Distance = std::iter_difference_t<T>>
                requires(
                    std::invocable<advance_by_op, T, std::make_unsigned_t<Distance>> &&
                    std::invocable<pre_decrease, T> //
                )
            constexpr decltype(auto) operator()(T& v, Distance distance) const noexcept( //
                noexcept(
                    nothrow_invocable<advance_by_op, T, std::make_unsigned_t<Distance>> &&
                    nothrow_invocable<pre_decrease, T> //
                )
            )
            {
                if(distance >= 0) return (*this)(v, make_unsigned(distance));

                for(; distance < 0; ++distance) pre_decrease_v(v);
                return v;
            }
        };
    }

    inline constexpr struct advance : sequenced_invocables<plus_assign, details::advance_by_op>
    {
    } advance_v{};

    inline constexpr struct logical_imply_fn
    {
        constexpr auto operator()(const bool first_cond, const bool second_cond) const noexcept
        {
            return first_cond ? second_cond : true;
        }
    } logical_imply;
}
