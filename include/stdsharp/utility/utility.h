#pragma once

#include "adl_proof.h" // IWYU pragma: export
#include "auto_cast.h" // IWYU pragma: export
#include "cast_to.h" // IWYU pragma: export
#include "constructor.h" // IWYU pragma: export
#include "forward_cast.h" // IWYU pragma: export
#include "to_lvalue.h" // IWYU pragma: export
#include "value_wrapper.h" // IWYU pragma: export

#include <utility> // IWYU pragma: export

#include "../compilation_config_in.h"

namespace stdsharp
{
    inline constexpr struct as_lvalue_fn
    {
        STDSHARP_INTRINSIC constexpr auto& operator()(auto&& t) const noexcept { return t; }

        void operator()(const auto&& t) = delete;
    } as_lvalue{};
}

#include "../compilation_config_out.h"
