#pragma once

#include "../concepts/object.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    inline constexpr struct to_lvalue_fn
    {
        STDSHARP_INTRINSIC constexpr auto& operator()(auto& t) const noexcept { return t; }

        template<typename T, std::move_constructible U = std::decay_t<T>>
            requires std::constructible_from<U, T>
        [[nodiscard]] constexpr auto operator()(T&& t) const
            noexcept(nothrow_constructible_from<U, T> && nothrow_move_constructible<U>)
        {
            return U{cpp_move(t)};
        }
    } to_lvalue{};

    template<typename T>
    using to_lvalue_t = std::invoke_result_t<to_lvalue_fn, T>;
}

#include "../compilation_config_out.h"
