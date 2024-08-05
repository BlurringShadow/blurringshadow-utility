#pragma once

#include "../macros.h"
#include "../namespace_alias.h"

#include <functional>

namespace stdsharp
{
    inline constexpr struct bind_front_fn
    {
        [[nodiscard]] constexpr auto operator()(auto&&... args) const
            noexcept(noexcept(std::bind_front(cpp_forward(args)...))) //
            -> decltype(std::bind_front(cpp_forward(args)...))
        {
            return std::bind_front(cpp_forward(args)...);
        }
    } bind_front{};

#if __cpp_lib_bind_back >= 202202L
    inline constexpr struct bind_back_fn
    {
        [[nodiscard]] constexpr auto operator()(auto&&... args) const
            noexcept(noexcept(std::bind_back(cpp_forward(args)...))) //
            -> decltype(std::bind_back(cpp_forward(args)...))
        {
            return std::bind_back(cpp_forward(args)...);
        }
    } bind_back{};
#endif
}
