#pragma once

#include "adl_proof.h" // IWYU pragma: export
#include "auto_cast.h" // IWYU pragma: export
#include "cast_to.h" // IWYU pragma: export
#include "constructor.h" // IWYU pragma: export
#include "forward_cast.h" // IWYU pragma: export
#include "to_lvalue.h" // IWYU pragma: export
#include "value_wrapper.h" // IWYU pragma: export

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

    template<typename T, typename U>
    using forward_like_t = decltype(forward_like<T>(std::declval<U>()));

    inline constexpr struct as_lvalue_fn
    {
        STDSHARP_INTRINSIC constexpr auto& operator()(auto&& t) const noexcept { return t; }

        void operator()(const auto&& t) = delete;
    } as_lvalue{};
}

#include "../compilation_config_out.h"
