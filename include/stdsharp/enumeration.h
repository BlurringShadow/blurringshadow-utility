#pragma once

#include "utility/auto_cast.h"

namespace stdsharp
{
    template<enum_ T>
    struct enumeration
    {
        using enum_type = T;
        using underlying_type = std::underlying_type_t<T>;

        T value{};

        explicit constexpr operator underlying_type() const noexcept { return auto_cast(value); }

        explicit constexpr operator T&() noexcept { return value; }

        explicit constexpr operator T() const noexcept { return value; }

        constexpr auto operator==(const T other) const noexcept { return value == other; }

        constexpr auto operator!=(const T other) const noexcept { return !(*this == other); }

        friend constexpr auto operator==(const T v, const enumeration e) noexcept { return e == v; }

        friend constexpr auto operator!=(const T v, const enumeration e) noexcept { return e != v; }
    };

    template<typename T>
    enumeration(T) -> enumeration<std::remove_cv_t<T>>;

    template<enum_ T>
    struct flag : enumeration<T>
    {
        using base = enumeration<T>;
        using base::base;
        using base::value;
        using typename base::underlying_type;

        template<typename U>
        constexpr flag(const U t) noexcept: base{t}
        {
        }

        [[nodiscard]] constexpr bool contains(const T other) const noexcept
        {
            const underlying_type v = auto_cast(*this);
            return (v & static_cast<underlying_type>(other)) == v;
        }

        [[nodiscard]] constexpr bool contains(const flag other) const noexcept
        {
            return contains(other.value);
        } // NOLINTBEGIN(hicpp-signed-bitwise)

        [[nodiscard]] constexpr auto operator|(const T other) const noexcept
        {
            return flag{value | other};
        }

        [[nodiscard]] friend constexpr auto operator|(const T v, const flag e) noexcept
        {
            return flag{e | v};
        }

        [[nodiscard]] constexpr flag operator|(const flag other) const noexcept
        {
            return flag{value | other};
        }

        [[nodiscard]] constexpr flag operator&(const T other) const noexcept
        {
            return flag{value & other};
        }

        [[nodiscard]] friend constexpr auto operator&(const T v, const flag e) noexcept
        {
            return flag{e & v};
        }

        [[nodiscard]] constexpr auto operator&(const flag other) const noexcept
        {
            return flag{value & other};
        } // NOLINTEND(hicpp-signed-bitwise)
    };

    template<typename T>
    flag(T) -> flag<std::remove_cv_t<T>>;
}
