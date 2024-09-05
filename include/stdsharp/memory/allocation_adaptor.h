#pragma once

#include "../utility/dispatcher.h"
#include "allocation_traits.h"
#include "launder_iterator.h"

namespace stdsharp
{
    template<allocator_req Allocator, typename T = Allocator::value_type>

}

namespace stdsharp::details
{
    template<typename Alloc, lifetime_req Req>
    class allocation_box_adaptor
    {
    public:
        using allocation_traits = allocation_traits<Alloc>;

        using allocator_traits = allocation_traits::allocator_traits;

        using allocation_type = allocation_traits::allocation;

        using callocation_type = allocation_traits::callocation;

        using fake_type = fake_type<Req>;

        static constexpr auto req = allocator_traits::template type_req<fake_type>;

        using dispatchers_t = stdsharp::invocables<
            dispatcher<
                req.default_construct,
                void,
                Alloc&,
                const allocation_type&,
                allocation_emplace_t>,
            dispatcher<
                req.copy_construct,
                void,
                Alloc&,
                const callocation_type&,
                const allocation_type&>,
            dispatcher<
                req.move_construct,
                void,
                Alloc&,
                const allocation_type&,
                const allocation_type&>,
            dispatcher<req.copy_assign, void, const callocation_type&, const allocation_type&>,
            dispatcher<req.move_assign, void, const allocation_type&, const allocation_type&>,
            dispatcher<expr_req::no_exception, void, Alloc&, const allocation_type&>>;

    private:
        indexed_values<dispatchers_t, std::size_t> values_{};
        cttid_t type_;

        constexpr decltype(auto) dispatchers(this auto&& self) noexcept
        {
            return cpp_forward(self).values_.template get<0>();
        }

        static constexpr auto self_cast = fwd_cast<allocation_box_adaptor>;

    public:
        allocation_box_adaptor() = default;

        template<
            typename Self,
            typename... Args,
            std::invocable<Args...> Dispatchers = forward_like_t<Self, dispatchers_t>>
        constexpr void operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Dispatchers, Args...>)
        {
            self_cast(cpp_forward(self)).dispatchers()(cpp_forward(args)...);
        }

        [[nodiscard]] auto& type() const noexcept { return type_; }

        constexpr decltype(auto) dispatchers() const noexcept { return values_.template get<0>(); }

        constexpr bool operator==(const allocation_box_adaptor& other) const noexcept
        {
            return type_ == other.type_;
        }

        template<typename T, typename Op = allocation_adaptor<Alloc, T>>
            requires(allocator_traits::template type_req<T> >= req)
        explicit constexpr allocation_box_adaptor(const std::in_place_type_t<T> /*unused*/) noexcept
            :
            values_(dispatchers_t{Op{}, Op{}, Op{}, Op{}, Op{}}, std::size_t{sizeof(T)}),
            type_(cttid<T>)
        {
        }

        template<lifetime_req OtherReq>
            requires std::constructible_from<
                         dispatchers_t,
                         typename allocation_box_adaptor<Alloc, OtherReq>::dispatchers_t>
        explicit constexpr allocation_adaptor(const allocation_box_adaptor<Alloc, OtherReq> other
        ) noexcept:
            values_(other.values_), type_(other.type_)
        {
        }

        [[nodiscard]] constexpr auto value_size() const noexcept
        {
            return values_.template get<1>();
        }
    };
}
