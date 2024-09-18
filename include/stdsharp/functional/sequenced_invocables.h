#pragma once

#include "../tuple/get.h"
#include "invocables.h"

namespace stdsharp
{
    template<typename... Func>
    struct sequenced_invocables : invocables<Func...>
    {
    private:
        using m_base = invocables<Func...>;

        template<typename Self, std::size_t I, typename... Args>
        static consteval auto first_invocable() noexcept
        {
            if constexpr(I >= m_base::size()) return I;
            else
            {
                using fn = forward_like_t<Self, typename m_base::template type<I>>;

                if constexpr(std::invocable<fn, Args...>) return I;
                else return first_invocable<Self, I + 1, Args...>();
            }
        }

    public:
        using m_base::m_base;

        template<
            typename Self,
            typename... Args,
            auto I = first_invocable<Self, 0, Args...>(),
            std::invocable<Args...> Fn =
                get_element_t<I, forward_like_t<Self, sequenced_invocables>>>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            return invoke(
                get_element<I>(fwd_cast<sequenced_invocables>(cpp_forward(self))),
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
