#pragma once

#include "../concepts/object.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    inline constexpr struct auto_cast_fn
    {
    private:
        template<typename T>
        struct auto_cast_operator
        {
            T&& t; // NOLINT(*-avoid-const-or-ref-data-members)

            template<typename U>
                requires explicitly_convertible<T, U>
            STDSHARP_INTRINSIC constexpr operator U() const&& //
                noexcept(nothrow_explicitly_convertible<T, U>)
            {
                return static_cast<U>(cpp_forward(t));
            }
        };

    public:
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
        {
            return auto_cast_operator<T>{cpp_forward(t)};
        }
    } auto_cast{};
}

#include "../compilation_config_out.h"
