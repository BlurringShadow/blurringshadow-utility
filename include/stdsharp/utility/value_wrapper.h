#pragma once

#include "../concepts/object.h"
#include "fwd_cast.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename T>
    class value_wrapper
    {
        T v;

    public:
        template<typename... U>
            requires std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            v(cpp_forward(u)...)
        {
        }

        [[nodiscard]] constexpr decltype(auto) get(this auto&& self) noexcept
        {
            return (fwd_cast<value_wrapper>(cpp_forward(self)).v);
        }
    };

    template<typename T>
    class value_wrapper<T&> : std::reference_wrapper<T>
    {
        using m_base = std::reference_wrapper<T>;

    public:
        using m_base::m_base;

        [[nodiscard]] constexpr decltype(auto) get() const noexcept { return m_base::get(); }
    };

    template<empty_type T>
    class value_wrapper<T> : T
    {
    public:
        value_wrapper() = default;

        using T::T;

        template<typename... U>
            requires(sizeof...(U) > 0) && std::constructible_from<T, U...>
        constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
            T(cpp_forward(u)...)
        {
        }

        [[nodiscard]] constexpr decltype(auto) get(this auto&& self) noexcept
        {
            return fwd_cast<T>(fwd_cast<value_wrapper>(cpp_forward(self)));
        }
    };

    template<void_ T>
    struct value_wrapper<T>
    {
        constexpr void get() const noexcept {}

        constexpr void cget() const noexcept {}
    };
}

namespace stdsharp
{
    template<typename T = void>
    struct value_wrapper : details::value_wrapper<T>
    {
    private:
        static constexpr auto self_cast = fwd_cast<value_wrapper>;

        using m_base = details::value_wrapper<T>;

    public:
        using value_type = T;

        using m_base::m_base;

        [[nodiscard]] constexpr decltype(auto) cget(this const auto&& self) noexcept
        {
            return self_cast(cpp_forward(self)).get();
        }

        [[nodiscard]] constexpr decltype(auto) cget(this const auto& self) noexcept
        {
            return self_cast(self).get();
        }
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<std::decay_t<T>>;

    value_wrapper() -> value_wrapper<>;
}

#include "../compilation_config_out.h"
