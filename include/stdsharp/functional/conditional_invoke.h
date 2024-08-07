#pragma once

#include "../type_traits/type.h"

namespace stdsharp
{
    template<bool Condition>
    struct conditional_invoke_fn
    {
        template<std::invocable Func>
            requires Condition
        constexpr decltype(auto) operator()(Func&& func, const auto& /*unused*/ = empty) //
            const noexcept(nothrow_invocable<Func>)
        {
            return invoke(cpp_forward(func));
        }

        template<std::invocable Func = empty_t>
        constexpr decltype(auto) operator()(const auto& /*unused*/, Func&& func = empty) //
            const noexcept(nothrow_invocable<Func>)
        {
            return invoke(cpp_forward(func));
        }
    };

    template<bool Condition>
    inline constexpr conditional_invoke_fn<Condition> conditional_invoke{};

    template<bool Condition, typename T, typename U>
    concept conditional_invocable = std::invocable<conditional_invoke_fn<Condition>, T, U>;

    template<bool Condition, typename T, typename U>
    concept nothrow_conditional_invocable =
        nothrow_invocable<conditional_invoke_fn<Condition>, T, U>;
}
