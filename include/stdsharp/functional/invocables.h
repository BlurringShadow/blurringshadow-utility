#pragma once

#include "../type_traits/indexed_traits.h"
#include "invoke.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename...>
    struct invocables;

    template<typename Base, std::size_t I>
    struct invoke_operator
    {
        template<
            typename Self,
            typename... Args,
            std::invocable<Args...> Fn = forward_like_t<
                Self,
                typename stdsharp::dependent_type<Base, Self>::template type<I>>>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            return invoke(
                fwd_cast<Base>(cpp_forward(self)).template get<I>(),
                cpp_forward(args)...
            );
        }
    };

    template<typename... Func, std::size_t... I>
    struct STDSHARP_EBO invocables<std::index_sequence<I...>, Func...> :
        stdsharp::indexed_values<Func...>,
        private invoke_operator<invocables<std::index_sequence<I...>, Func...>, I>...
    {
    private:
        using m_base = stdsharp::indexed_values<Func...>;

    public:
        using m_base::m_base;

        using invoke_operator<invocables, I>::operator()...;
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
