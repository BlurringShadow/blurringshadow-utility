#pragma once

#include "../type_traits/indexed_traits.h"
#include "invoke.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename... Func>
    struct invocables_traits
    {
    private:
        using indexed = stdsharp::indexed_values<Func...>;

        template<std::size_t I>
        struct invocable_t
        {
        private:
            using func_t = typename indexed::template type<I>;

        public:
            template<
                typename Self,
                typename... Args,
                std::invocable<Args...> Fn = forward_cast_t<Self, func_t>>
            constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
                noexcept(nothrow_invocable<Fn, Args...>)
            {
                return invoke(
                    ((forward_cast_t<Self, indexed>)self).template get<I>(), // NOLINT
                    cpp_forward(args)...
                );
            }
        };

    public:
        template<typename = std::index_sequence_for<Func...>>
        struct impl;

        template<std::size_t... I>
        struct STDSHARP_EBO impl<std::index_sequence<I...>> : indexed, invocable_t<I>...
        {
            using indexed::indexed;
            using invocable_t<I>::operator()...;

            template<
                std::size_t J,
                typename Self,
                typename... Args,
                typename Invocable = invocable_t<J>,
                std::invocable<Args...> Fn = forward_cast_t<Self, Invocable>>
            constexpr decltype(auto) invoke_at(this Self&& self, Args&&... args)
                noexcept(nothrow_invocable<Fn, Args...>)
            {
                auto&& impl_v = forward_cast<Self, impl>(self);
                return forward_cast<decltype(impl_v), Invocable>(impl_v)(cpp_forward(args)...);
            }
        };
    };
}

namespace stdsharp
{
    template<typename... Func>
    struct invocables : details::invocables_traits<Func...>::template impl<>
    {
    private:
        using m_base = details::invocables_traits<Func...>::template impl<>;

    public:
        using m_base::m_base;
    };

    template<typename... Func>
    invocables(Func&&...) -> invocables<std::decay_t<Func>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::invocables<T...>>
    {
        static constexpr auto value = ::stdsharp::invocables<T...>::size();
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::invocables<T...>>
    {
        using type = typename ::stdsharp::invocables<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"
