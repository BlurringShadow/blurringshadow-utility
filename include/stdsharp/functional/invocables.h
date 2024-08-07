#pragma once

#include "../functional/invoke.h"
#include "../type_traits/indexed_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<std::size_t I, typename Func>
    struct indexed_invocable : value_wrapper<Func>
    {
        template<
            typename Self,
            typename... Args,
            std::invocable<Args...> Fn = forward_cast_t<Self, Func>>
        constexpr decltype(auto) operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Fn, Args...>)
        {
            return invoke(forward_cast<Self, indexed_invocable>(self).get(), cpp_forward(args)...);
        }
    };
}

namespace stdsharp::details
{
    template<typename...>
    struct invocables;

    template<typename... Func, std::size_t... I>
    struct STDSHARP_EBO invocables<std::index_sequence<I...>, Func...> :
        indexed_invocable<I, Func>...
    {
        using indexed_invocable<I, Func>::operator()...;

        static constexpr auto size() noexcept { return sizeof...(Func); }

        template<std::size_t J>
        using type = type_at<J, Func...>;

        template<std::size_t J, typename Self>
        constexpr decltype(auto) get(this Self&& self) noexcept
        {
            return forward_cast<Self, invocables, indexed_invocable<J, type<J>>>(self).get();
        }

        template<std::size_t J, typename Self>
        constexpr decltype(auto) cget(this const Self&& self) noexcept
        {
            return forward_cast<const Self, invocables, indexed_invocable<J, type<J>>>(self).cget();
        }

        template<std::size_t J, typename Self>
        constexpr decltype(auto) cget(this const Self& self) noexcept
        {
            return forward_cast<const Self&, invocables, indexed_invocable<J, type<J>>>(self).cget(
            );
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
        invocables() = default;

        constexpr invocables(Func&&... func)
            noexcept(nothrow_list_initializable_from<m_base, Func...>)
            requires list_initializable_from<m_base, Func...>
            : m_base{cpp_move(func)...}
        {
        }

        template<typename... T>
            requires list_initializable_from<m_base, T...>
        constexpr invocables(T&&... t) noexcept(nothrow_list_initializable_from<m_base, T...>):
            m_base{cpp_forward(t)...}
        {
        }
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
