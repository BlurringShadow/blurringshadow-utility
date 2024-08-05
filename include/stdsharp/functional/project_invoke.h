#pragma once

#include "invoke.h"

namespace stdsharp
{
    inline constexpr struct project_invoke_fn
    {
        template<typename Fn, typename Projector, typename... Args>
            requires requires {
                requires(std::invocable<Projector, Args> && ...);
                requires std::invocable<Fn, std::invoke_result_t<Projector, Args>...>;
            }
        constexpr decltype(auto) operator()(Fn&& fn, Projector projector, Args&&... args) const
            noexcept(
                (nothrow_invocable<Projector, Args> && ...) &&
                nothrow_invocable<Fn, std::invoke_result_t<Projector, Args>...> //
            )
        {
            return invoke(cpp_forward(fn), invoke(projector, cpp_forward(args))...);
        }
    } project_invoke{};

    template<typename... Args>
    concept project_invocable = std::invocable<project_invoke_fn, Args...>;

    template<typename... Args>
    concept project_nothrow_invocable = nothrow_invocable<project_invoke_fn, Args...>;
}
