#pragma once

#include "../concepts/object.h"

namespace stdsharp
{
    template<typename T>
    struct constructor
    {
        template<typename... Args>
            requires std::constructible_from<T, Args...>
        [[nodiscard]] constexpr auto operator()(Args&&... args) const
            noexcept(nothrow_constructible_from<T, Args...>)
        {
            return T(cpp_forward(args)...);
        }

        template<typename... Args>
            requires(!std::constructible_from<T, Args...> && list_initializable_from<T, Args...>)
        [[nodiscard]] constexpr auto operator()(Args&&... args) const
            noexcept(nothrow_list_initializable_from<T, Args...>)
        {
            return T{cpp_forward(args)...};
        }
    };

    template<typename T>
    inline constexpr constructor<T> construct{};
}
