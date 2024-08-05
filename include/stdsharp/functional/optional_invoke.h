#pragma once

#include "sequenced_invocables.h"

namespace stdsharp
{
    inline constexpr sequenced_invocables optional_invoke{invoke, empty};

    template<typename... Args>
    concept nothrow_optional_invocable = noexcept(optional_invoke(std::declval<Args>()...));
}
