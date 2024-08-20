#pragma once

#include "../namespace_alias.h"

#include <utility>

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T>
    struct forward_like_fn
    {
        [[nodiscard]] constexpr decltype(auto) operator()(auto&& u) const noexcept
        {
            return std::forward_like<T>(u);
        }
    };

    template<typename T>
    inline constexpr forward_like_fn<T> forward_like{};

    template<typename From, typename To>
    using forward_like_t = decltype(forward_like<From>(std::declval<To>()));

    template<typename To>
    struct forward_cast_fn
    {
        template<typename From, typename NoRef = std::remove_reference_t<From>> // c-style cast allows cast to inaccessible base
            requires(std::derived_from<NoRef, To> || std::derived_from<To, NoRef>)
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(From&& from) const noexcept
        {
            return (forward_like_t<From, To>)from; // NOLINT
        }
    };

    template<typename To>
    inline constexpr forward_cast_fn<To> fwd_cast{};
}

#include "../compilation_config_out.h"
