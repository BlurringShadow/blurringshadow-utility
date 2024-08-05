#pragma once

#include "functional/operations.h"

namespace stdsharp
{
    inline constexpr struct
    {
    private:
        static constexpr auto impl(const auto& condition, auto& cases)
        {
            auto&& [first, second] = cases;

            if(invoke(first, condition))
            {
                invoke(second, condition);
                return true;
            }
            return false;
        }

    public:
        template<
            typename Condition,
            std::predicate<const Condition&>... Predicate,
            std::invocable<const Condition&>... Func>
        constexpr void operator()(
            const Condition& condition,
            std::pair<Predicate, Func>... cases //
        ) const
            noexcept(
                ((nothrow_predicate<Predicate, Condition> && nothrow_invocable<Func, Condition>) &&
                 ...)
            )
        {
            (impl(condition, cases) || ...);
        }

        template<typename Condition, std::invocable<const Condition>... Func>
        constexpr void operator()(
            const Condition& condition,
            std::pair<Condition, Func>... cases //
        ) const noexcept((nothrow_invocable<Func, Condition> && ...))
        {
            (*this)(
                condition,
                std::make_pair(
                    std::bind_front(equal_to_v, cpp_move(cases.first)),
                    cpp_move(cases.second)
                )...
            );
        }
    } pattern_match{};

    namespace constexpr_pattern_match
    {
        namespace details
        {
            template<typename T>
            struct impl
            {
            private:
                static constexpr auto invoke(std::invocable<T> auto& c)
                {
                    stdsharp::invoke(c, T{});
                    return true;
                }

                static constexpr auto invoke(const auto& /*unused*/) { return false; }

            public:
                template<typename... Cases>
                constexpr void operator()(Cases... cases) const noexcept(
                    (logical_imply(std::invocable<Cases, T>, nothrow_invocable<Cases, T>) && ...)
                )
                {
                    (invoke(cases) || ...);
                }
            };
        }

        template<typename ConditionT>
        using from_type_fn = details::impl<std::type_identity<ConditionT>>;

        template<auto Condition>
        using from_constant_fn = details::impl<constant<Condition>>;

        template<typename ConditionT>
        inline constexpr from_type_fn<ConditionT> from_type{};

        template<auto Condition>
        inline constexpr from_constant_fn<Condition> from_constant{};
    }
}
