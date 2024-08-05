#pragma once

#include "../concepts/object.h"

namespace stdsharp
{
    inline constexpr struct invoke_fn
    {
        template<typename... Args, std::invocable<Args...> Fn>
        constexpr decltype(auto) operator()(Fn&& fn, Args&&... args) const
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            return std::invoke(cpp_forward(fn), cpp_forward(args)...);
        }
    } invoke{};
}
