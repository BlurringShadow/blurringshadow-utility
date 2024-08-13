#pragma once // TODO: __cpp_pack_indexing >= 202311L

#include "../utility/value_wrapper.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<size_t>
    struct invalid_type_indexer
    {
    };

    template<typename... T>
    struct type_indexer
    {
        using size_t = std::size_t;

        template<size_t I, typename = std::index_sequence_for<T...>>
        struct t;

        template<size_t I, size_t... J>
        struct t<I, std::index_sequence<J...>> :
            std::conditional_t<I == J, std::type_identity<T>, invalid_type_indexer<J>>...
        {
        };

    public:
        template<size_t I>
        using type = t<I>::type;
    };
}

namespace stdsharp
{
    template<std::size_t I, typename T>
    struct indexed_value : value_wrapper<T>
    {
    };

    template<std::size_t I, typename... T>
    using type_at = details::type_indexer<T...>::template type<I>;
}

namespace stdsharp::details
{
    template<typename...>
    struct indexed_values;

    template<typename... T, std::size_t... I>
    struct STDSHARP_EBO indexed_values<std::index_sequence<I...>, T...> : indexed_value<I, T>...
    {
        static constexpr auto size() noexcept { return sizeof...(T); }

        template<std::size_t J>
        using type = type_at<J, T...>;

        using index_sequence = std::index_sequence<I...>;

    private:
        template<typename Self, auto J>
        static constexpr decltype(auto) self_cast_at(Self&& self)
        {
            return forward_cast<indexed_values, indexed_value<J, type<J>>>(
                forward_cast<Self, indexed_values>(self)
            );
        }

    public:
        template<std::size_t J, typename Self>
        constexpr decltype(auto) get(this Self&& self) noexcept
        {
            return self_cast_at<Self, J>(self).get();
        }

        template<std::size_t J, typename Self>
        constexpr decltype(auto) cget(this const Self&& self) noexcept
        {
            return self_cast_at<const Self, J>(self).get();
        }

        template<std::size_t J, typename Self>
        constexpr decltype(auto) cget(this const Self& self) noexcept
        {
            return self_cast_at<const Self&, J>(self).get();
        }
    };
}

namespace stdsharp
{
    template<typename... T>
    struct indexed_values : details::indexed_values<std::make_index_sequence<sizeof...(T)>, T...>
    {
    private:
        using m_base = details::indexed_values<std::make_index_sequence<sizeof...(T)>, T...>;

    public:
        indexed_values() = default;

        constexpr indexed_values(T&&... t) noexcept(nothrow_list_initializable_from<m_base, T...>)
            requires list_initializable_from<m_base, T...>
            : m_base{cpp_move(t)...}
        {
        }

        template<typename... U>
            requires list_initializable_from<m_base, U...>
        constexpr indexed_values(U&&... t) noexcept(nothrow_list_initializable_from<m_base, U...>):
            m_base{cpp_forward(t)...}
        {
        }
    };

    template<typename... T>
    indexed_values(T&&...) -> indexed_values<std::decay_t<T>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::indexed_values<T...>>
    {
        static constexpr auto value = ::stdsharp::indexed_values<T...>::size();
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::indexed_values<T...>>
    {
        using type = typename ::stdsharp::indexed_values<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"
