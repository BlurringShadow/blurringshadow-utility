#pragma once

#include "../concepts/object.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename U>
    struct cast_to_fn
    {
        STDSHARP_INTRINSIC constexpr U operator()(explicitly_convertible<U> auto&& t) const
            noexcept(noexcept(static_cast<U>(cpp_forward(t))))
        {
            return static_cast<U>(cpp_forward(t));
        }
    };

    template<typename U>
    inline constexpr cast_to_fn<U> cast_to{};
}

#include "../compilation_config_out.h"
