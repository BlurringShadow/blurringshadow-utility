#pragma once

#include "../concepts/object.h"
#include "forward_cast.h"

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

        template<typename Self>
        [[nodiscard]] constexpr decltype(auto) get(this Self&& self) noexcept
        {
            return (forward_cast<Self, value_wrapper>(self).v);
        }
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

        template<typename Self>
        [[nodiscard]] constexpr decltype(auto) get(this Self&& self) noexcept
        {
            return forward_cast<Self, value_wrapper>(self);
        }
    };
}

namespace stdsharp
{
    template<typename T = void>
    struct value_wrapper : details::value_wrapper<T>
    {
    private:
        template<typename From>
        static constexpr auto self_cast = forward_cast<From, value_wrapper>;

    public:
        using value_type = T;

        value_wrapper() = default;

        using details::value_wrapper<T>::value_wrapper;

        constexpr value_wrapper(T&& t)
            noexcept(nothrow_constructible_from<details::value_wrapper<T>, T>)
            requires std::constructible_from<details::value_wrapper<T>, T>
            : details::value_wrapper<T>(cpp_move(t))
        {
        }

        template<typename Self, typename SelfT = const Self>
        [[nodiscard]] constexpr decltype(auto) cget(this const Self&& self) noexcept
        {
            return self_cast<Self>(self).get();
        }

        template<typename Self, typename SelfT = const Self&>
        [[nodiscard]] constexpr decltype(auto) cget(this const Self& self) noexcept
        {
            return self_cast<Self>(self).get();
        }
    };

    template<void_ T>
    struct value_wrapper<T>
    {
        constexpr void get() const noexcept {}

        constexpr void cget() const noexcept {}
    };

    template<typename T>
    value_wrapper(T&&) -> value_wrapper<std::decay_t<T>>;

    value_wrapper() -> value_wrapper<>;
}

#include "../compilation_config_out.h"
