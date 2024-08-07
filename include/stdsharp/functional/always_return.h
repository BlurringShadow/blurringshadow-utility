#pragma once

#include "../macros.h"
#include "../namespace_alias.h"

namespace stdsharp
{
    inline constexpr struct always_return_fn
    {
        [[nodiscard]] constexpr decltype(auto
        ) operator()(auto&& v, const auto&... /*unused*/) const noexcept
        {
            return cpp_forward(v);
        }
    } always_return{};

    inline constexpr struct always_true_fn
    {
        [[nodiscard]] constexpr bool operator()(const auto&... /*unused*/) const noexcept
        {
            return true;
        }
    } always_true{};

    inline constexpr struct always_false_fn
    {
        [[nodiscard]] constexpr bool operator()(const auto&... /*unused*/) const noexcept
        {
            return false;
        }
    } always_false{};

    template<typename T>
    struct always_default_fn
    {
        [[nodiscard]] constexpr auto operator()(const auto&... /*unused*/) const noexcept
        {
            return T{};
        }
    };

    template<typename T>
    inline constexpr always_default_fn<T> always_default{};
}
