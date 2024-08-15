#pragma once

#include "../functional/invoke.h"
#include "../type_traits/indexed_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename Func>
    struct invocable_t
    {
        Func fn;

        template<
            typename Self,
            typename... Args,
            std::invocable<Args...> Fn = forward_cast_t<Self, Func>>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            return invoke(forward_cast<Self, invocable_t>(self).fn, cpp_forward(args)...);
        }
    };
}

namespace stdsharp
{
    template<typename... Func>
    struct invocables : indexed_values<invocable_t<Func>...>
    {
    private:
        using m_base = indexed_values<invocable_t<Func>...>;

    public:
        using m_base::m_base;
        using invocable_t<Func>::operator()...;
    };

    template<typename... T>
    invocables(T&&...) -> invocables<std::decay_t<T>...>;
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
