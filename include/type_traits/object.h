﻿// Created by BlurringShadow at 2021-03-05-下午 11:53

#pragma once

#include "concepts/concepts.h"

namespace stdsharp::type_traits
{
    struct unique_object
    {
        unique_object() noexcept = default;
        unique_object(const unique_object&) noexcept = delete;
        constexpr unique_object(unique_object&&) noexcept {};
        unique_object& operator=(const unique_object&) noexcept = delete;
        constexpr unique_object& operator=(unique_object&&) noexcept { return *this; };
        ~unique_object() = default;
    };

    template<typename T>
    struct private_object
    {
        friend T;

    protected:
        private_object() noexcept = default;
        private_object(const private_object&) noexcept = default;
        private_object& operator=(const private_object&) noexcept = default;
        private_object(private_object&&) noexcept = default;
        private_object& operator=(private_object&&) noexcept = default;
        ~private_object() = default;
    };

    template<typename Base, typename... T>
    struct inherited : Base, T...
    {
        inherited() = default;

        template<typename... U>
            requires(
                ::std::constructible_from<Base, U...> && (::std::constructible_from<T> && ...) //
            )
        constexpr explicit inherited(U&&... u) //
            noexcept(
                concepts::nothrow_constructible_from<Base, U...> &&
                (concepts::nothrow_constructible_from<T> && ...) // clang-format off
            ):
            Base(::std::forward<U>(u)...) // clang-format on
        {
        }

        template<typename BaseT, typename... U>
            requires(
                ::std::constructible_from<Base, BaseT> &&
                (::std::constructible_from<T, U> && ...) //
            )
        constexpr explicit inherited(BaseT&& base, U&&... u) //
            noexcept(
                concepts::nothrow_constructible_from<Base, BaseT> &&
                (concepts::nothrow_constructible_from<T, U> && ...) // clang-format off
                ):
            Base(::std::forward<BaseT>(base)), T(::std::forward<U>(u))... // clang-format on
        {
        }

        template<typename U>
            requires ::std::assignable_from<Base, U>
        constexpr inherited& operator=(U&& u) noexcept(concepts::nothrow_assignable_from<Base, U>)
        {
            Base::operator=(::std::forward<U>(u));
            return *this;
        }

        constexpr explicit operator Base&() noexcept { return *this; }
        constexpr explicit operator const Base&() const noexcept { return *this; }
    };

    template<typename... T>
    inherited(T&&...) -> inherited<::std::decay_t<T>...>;

    template<typename... T>
    struct make_inherited_fn
    {
        template<
            typename Base,
            typename... U,
            typename Inherited = inherited<::std::decay_t<Base>, T...>>
            requires ::std::constructible_from<Inherited, Base, U...>
        [[nodiscard]] constexpr auto operator()(Base&& base, U&&... u) const
            noexcept(concepts::nothrow_constructible_from<Inherited, Base, U...>)
        {
            return Inherited{::std::forward<Base>(base), ::std::forward<U>(u)...};
        }
    };

    template<typename... T>
    inline constexpr make_inherited_fn<T...> make_inherited{};
}
