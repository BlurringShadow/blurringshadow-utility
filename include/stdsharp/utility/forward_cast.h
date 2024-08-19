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

    template<typename From, typename To>
    struct forward_cast_fn
    {
        STDSHARP_INTRINSIC constexpr decltype(auto) operator()(From&& from)
            const noexcept // c-style cast allows cast to inaccessible base
        {
            return (forward_cast_t<From, To>)from; // NOLINT
        }
    };

    template<typename From, not_decay_derived<From> To>
        requires not_decay_derived<From, To> && (!decay_same_as<From, To>)
    struct forward_cast_fn<From, To>;

    template<typename From, typename To>
    inline constexpr forward_cast_fn<From, To> forward_cast{};
}

#include "../compilation_config_out.h"
