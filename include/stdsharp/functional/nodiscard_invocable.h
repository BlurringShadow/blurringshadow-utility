#pragma once

#include "../utility/value_wrapper.h"
#include "invoke.h"

namespace stdsharp
{
    template<typename Func>
    struct nodiscard_invocable : value_wrapper<Func>
    {
        template<
            typename Self,
            typename... Args,
            std ::invocable<Args...> Fn = forward_cast_t<Self, Func>>
        [[nodiscard]] constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            return invoke(
                forward_cast<Self, nodiscard_invocable>(self).get(),
                cpp_forward(args)...
            );
        }
    };

    template<typename Func>
    nodiscard_invocable(Func&&) -> nodiscard_invocable<std::decay_t<Func>>;
}
