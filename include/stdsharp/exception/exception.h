#pragma once

#include "../namespace_alias.h"

#include <array>
#include <exception>

namespace stdsharp
{
    template<std::size_t I>
    class aggregate_exceptions : public std::exception
    {
        std::array<std::exception_ptr, I> exceptions_;

    public:
        aggregate_exceptions(const std::array<std::exception_ptr, I>& exceptions) noexcept:
            exceptions_(exceptions)
        {
        }

        aggregate_exceptions(const auto&... exceptions) noexcept
            requires requires { decltype(exceptions_){exceptions...}; }
            : exceptions_{exceptions...}
        {
        }

        [[nodiscard]] const char* what() const noexcept override { return "Aggregate exceptions"; }

        [[nodiscard]] const auto& exceptions() const noexcept { return exceptions_; }
    };

    template<std::size_t I>
    aggregate_exceptions(const std::array<std::exception_ptr, I>&) -> aggregate_exceptions<I>;

    template<typename... T>
    aggregate_exceptions(const T&...) -> aggregate_exceptions<sizeof...(T)>;
}
