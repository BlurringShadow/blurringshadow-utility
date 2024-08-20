#pragma once

#include "../type_traits/indexed_traits.h"
#include "invoke.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename...>
    struct invocables;

    template<typename Base, std::size_t I>
    struct invocable_t
    {
        template<typename... Args, auto FwdCast = fwd_cast<Base>>
        constexpr decltype(auto) operator()(this auto&& self, Args&&... args) noexcept(
            noexcept(invoke(FwdCast(cpp_forward(self)).template get<I>(), cpp_forward(args)...))
        )
            requires requires {
                invoke(FwdCast(cpp_forward(self)).template get<I>(), cpp_forward(args)...);
            }
        {
            return invoke(FwdCast(cpp_forward(self)).template get<I>(), cpp_forward(args)...);
        }
    };

    template<typename... Func, std::size_t... I>
    struct STDSHARP_EBO invocables<std::index_sequence<I...>, Func...> :
        stdsharp::indexed_values<Func...>,
        invocable_t<invocables<std::index_sequence<I...>, Func...>, I>...
    {
    private:
        using m_base = stdsharp::indexed_values<Func...>;

    public:
        using m_base::m_base;

        using invocable_t<invocables, I>::operator()...;

        template<
            std::size_t J,
            typename Self,
            typename... Args,
            std::invocable<Args...> Fn = forward_like_t<Self, invocable_t<invocables, J>>>
        constexpr decltype(auto) invoke_at(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            auto&& impl_v = fwd_cast<invocables>(cpp_forward(self));
            return fwd_cast<Fn>(cpp_forward(impl_v))(cpp_forward(args)...);
        }
    };
}

namespace stdsharp
{
    template<typename... Func>
    struct invocables : details::invocables<std::index_sequence_for<Func...>, Func...>
    {
    private:
        using m_base = details::invocables<std::index_sequence_for<Func...>, Func...>;

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
