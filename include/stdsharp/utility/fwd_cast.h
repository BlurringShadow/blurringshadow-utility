#pragma once

#include "../type_traits/type.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename From, typename To>
    using forward_like_t = apply_qualifiers<
        To,
        const_<std::remove_reference_t<From>>,
        volatile_<std::remove_reference_t<From>>,
        ref_qualifier_v<From&&>>;

    template<typename T>
    struct forward_like_fn
    {
        template<typename U>
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(U&& u) const noexcept
        {
            return static_cast<forward_like_t<T, U>>(u);
        }
    };

    template<typename T>
    inline constexpr forward_like_fn<T> forward_like{};

    template<typename To>
    struct fwd_cast_fn
    {
        using no_ref_to = std::remove_reference_t<To>;

        template<typename From, typename NoRefFrom = std::remove_reference_t<From>>
        // c-style cast allows access inaccessible base
            requires(std::is_base_of_v<NoRefFrom, no_ref_to> || std::is_base_of_v<no_ref_to, NoRefFrom>)
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(From&& from) const noexcept
        {
            return (forward_like_t<From, To>)from; // NOLINT
        }
    };

    template<typename To>
    inline constexpr fwd_cast_fn<To> fwd_cast{};
}

#include "../compilation_config_out.h"
