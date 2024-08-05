#pragma once

#include "../concepts/object.h"

namespace stdsharp::cpo::inline cpo_impl
{
    void get(auto&&) = delete;

    template<std::size_t I>
    struct get_element_fn
    {
        [[nodiscard]] constexpr auto operator()(auto&& t) const
            noexcept(noexcept(cpp_forward(t).template get<I>())) -> //
            decltype(cpp_forward(t).template get<I>())
        {
            return cpp_forward(t).template get<I>();
        }

        [[nodiscard]] constexpr auto operator()(auto&& t) const
            noexcept(noexcept(get<I>(cpp_forward(t)))) -> decltype(get<I>(cpp_forward(t)))
            requires(!requires { cpp_forward(t).template get<I>(); })
        {
            return get<I>(cpp_forward(t));
        }
    };

    template<std::size_t I>
    inline constexpr get_element_fn<I> get_element;

    template<std::size_t I>
    struct cget_element_fn
    {
        template<typename T>
        [[nodiscard]] constexpr decltype(auto) operator()(const T& t) const
            noexcept(nothrow_invocable<get_element_fn<I>, T>)
            requires std::invocable<get_element_fn<I>, T>
        {
            return get_element<I>(t);
        }

        template<typename T>
        [[nodiscard]] constexpr decltype(auto) operator()(const T&& t) const
            noexcept(nothrow_invocable<get_element_fn<I>, T>)
            requires std::invocable<get_element_fn<I>, T>
        {
            return get_element<I>(cpp_move(t));
        }
    };

    template<std::size_t I>
    inline constexpr cget_element_fn<I> cget_element;
}

namespace stdsharp
{
    template<std::size_t I, typename T>
    using get_element_t = decltype(cpo::get_element<I>(std::declval<T>()));

    template<std::size_t I, typename T>
    using cget_element_t = decltype(cpo::cget_element<I>(std::declval<T>()));
}
