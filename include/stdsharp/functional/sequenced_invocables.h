#pragma once

#include "invocables.h"

namespace stdsharp
{
    template<typename... Func>
    struct sequenced_invocables : invocables<Func...>
    {
    private:
        using m_base = invocables<Func...>;

        template<typename Self, std::size_t J, typename... Args>
        static consteval auto first_invocable() noexcept
        {
            if constexpr(J >= sequenced_invocables::size()) return J;
            else
            {
                using fn = forward_cast_t<Self, typename sequenced_invocables::template type<J>>;

                if constexpr(std::invocable<fn, Args...>) return J;
                else return first_invocable<Self, J + 1, Args...>();
            }
        }

    public:
        using m_base::m_base;

        template<
            typename Self,
            typename... Args,
            auto J = first_invocable<Self, 0, Args...>(),
            std::invocable<Args...> Fn =
                forward_cast_t<Self, typename sequenced_invocables::template type<J>>>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            return invoke(
                forward_cast<Self, sequenced_invocables>(self).template get<J>(),
                cpp_forward(args)...
            );
        }
    };

    template<typename... Func>
    sequenced_invocables(Func&&...) -> sequenced_invocables<std::decay_t<Func>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::sequenced_invocables<T...>>
    {
        static constexpr auto value = ::stdsharp::sequenced_invocables<T...>::size();
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::sequenced_invocables<T...>>
    {
        using type = typename ::stdsharp::sequenced_invocables<T...>::template type<I>;
    };
}
