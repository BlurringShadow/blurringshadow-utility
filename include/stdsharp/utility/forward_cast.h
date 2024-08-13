#pragma once

#include "../type_traits/type.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename From, typename To>
    using forward_cast_t = apply_qualifiers<
        To,
        const_<std::remove_reference_t<From>>,
        volatile_<std::remove_reference_t<From>>,
        ref_qualifier_v<From&&>>;

    template<typename, typename>
    struct forward_cast_fn;

    template<typename From, not_decay_derived<From> To>
        requires not_decay_derived<From, To> && (!decay_same_as<From, To>)
    struct forward_cast_fn<From, To>;

    template<typename From, typename To>
    struct forward_cast_fn
    {
        using from_t = std::remove_reference_t<From>;
        using cast_t = forward_cast_t<From, To>;

        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(from_t& from) //
            const noexcept
        {
            // c-style cast allow us cast to inaccessible base
            if constexpr(decay_derived<From, To>) return (cast_t)from; // NOLINT
            else return static_cast<cast_t>(from);
        }

        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(from_t&& from) //
            const noexcept
        {
            // c-style cast allow us cast to inaccessible base
            if constexpr(decay_derived<From, To>) return (cast_t)from; // NOLINT
            else return static_cast<cast_t>(from);
        }
    };

    template<typename From, typename To>
    inline constexpr forward_cast_fn<From, To> forward_cast{};
}

#include "../compilation_config_out.h"
