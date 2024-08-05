#pragma once

#include "../compare/compare.h"
#include "../functional/operations.h"

#include <gsl/gsl>

#include <algorithm>

namespace stdsharp
{
    inline constexpr struct set_if_fn
    {
        template<typename T, typename U, std::predicate<U, T> Comp>
            requires std::assignable_from<T&, U>
        constexpr T& operator()(T& left, U&& right, Comp comp = {}) const
            noexcept(nothrow_predicate<Comp, U, T> && nothrow_assignable_from<T&, U>)
        {
            if(invoke(cpp_move(comp), right, left)) left = cpp_forward(right);
            return left;
        }
    } set_if{};

    inline constexpr struct set_if_greater_fn
    {
        template<typename T, typename U>
            requires std::invocable<set_if_fn, T&, U, std::ranges::greater>
        constexpr T& operator()(T& left, U&& right) const
            noexcept(nothrow_invocable<set_if_fn, T&, U, std::ranges::greater>)
        {
            return set_if(left, cpp_forward(right), greater_v);
        }
    } set_if_greater{};

    inline constexpr struct set_if_less_fn
    {
        template<typename T, typename U>
            requires std::invocable<set_if_fn, T&, U, std::ranges::less>
        constexpr T& operator()(T& left, U&& right) const
            noexcept(nothrow_invocable<set_if_fn, T&, U, std::ranges::less>)
        {
            return set_if(left, cpp_forward(right), less_v);
        }
    } set_if_less{};

    inline constexpr struct is_between_fn
    {
        template<
            typename T,
            typename Proj = std::identity,
            std::indirect_strict_weak_order<std::projected<const T*, Proj>> Compare = std::ranges::
                less>
        [[nodiscard]] constexpr auto operator()(
            const T& t,
            decltype(t) min,
            decltype(t) max,
            Compare cmp = {},
            Proj proj = {}
        ) const noexcept( //
            nothrow_predicate<
                Compare,
                std::projected<const T*, Proj>,
                std::projected<const T*, Proj>> //
        )
        {
            const auto& proj_max = invoke(proj, max);
            const auto& proj_min = invoke(proj, min);
            const auto& proj_t = invoke(proj, t);

            Expects(!invoke(cmp, proj_max, proj_min));

            return !invoke(cmp, proj_t, proj_min) && !invoke(cmp, proj_max, proj_t);
        }
    } is_between{};

    inline constexpr struct strict_compare_fn
    {
    private:
        using ordering = std::partial_ordering;

        static constexpr auto cmp_impl(ordering& pre, const ordering next) noexcept
        {
            if(is_eq(pre))
            {
                pre = next;
                return;
            }

            if(pre != next && is_neq(next))
            {
                pre = ordering::unordered;
                return;
            }
        };

    public:
        template<
            std::input_iterator I1,
            std::sentinel_for<I1> S1,
            std::input_iterator I2,
            std::sentinel_for<I2> S2,
            typename Cmp = std::compare_three_way>
            requires ordering_predicate<Cmp&, std::iter_reference_t<I1>, std::iter_reference_t<I2>>
        constexpr auto operator()(I1 i1, const S1 s1, I2 i2, const S2 s2, Cmp cmp = {}) const
        {
            auto pre = ordering::equivalent;

            for(; !is_ud(pre); ++i1, ++i2)
            {
                if(i1 == s1)
                {
                    if(i2 != s2) pre = ordering::unordered;
                    break;
                }

                if(i2 == s2)
                {
                    pre = ordering::unordered;
                    break;
                }

                cmp_impl(pre, invoke(cmp, *i1, *i2));
            }

            return pre;
        }

        template<
            std::ranges::input_range R1,
            std::ranges::input_range R2,
            typename Cmp = std::compare_three_way>
            requires ordering_predicate<
                Cmp&,
                std::ranges::range_reference_t<R1>,
                std::ranges::range_reference_t<R2>>
        constexpr auto operator()(R1&& r1, R2&& r2, Cmp cmp = {}) const
        {
            return (*this)(
                std::ranges::begin(r1),
                std::ranges::end(r1),
                std::ranges::begin(r2),
                std::ranges::end(r2),
                cmp
            );
        }
    } strict_compare{};

    template<typename In, typename Out>
    using move_n_result = std::ranges::in_out_result<In, Out>;

    inline constexpr struct move_n_fn
    {
        template<std::input_iterator In, std::weakly_incrementable Out>
            requires std::indirectly_movable<In, Out>
        constexpr move_n_result<In, Out>
            operator()(In in, const std::iter_difference_t<In> n, Out out) const
        {
            auto&& r = std::ranges::copy_n(std::move_iterator{cpp_move(in)}, n, cpp_move(out));

            return {cpp_move(r).in.base(), cpp_move(r).out};
        }
    } move_n{};
}
