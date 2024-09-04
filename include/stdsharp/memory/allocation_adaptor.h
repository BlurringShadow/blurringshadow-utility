#pragma once

#include "../utility/dispatcher.h"
#include "allocation_traits.h"

namespace stdsharp
{
    template<allocator_req Allocator, typename T = Allocator::value_type>
    struct allocation_object_traits
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocator_traits<Allocator>;
        using allocator_type = Allocator;
        using allocation = allocation_traits::allocation;
        using callocation = allocation_traits::callocation;

        static constexpr auto type_size = sizeof(T);

    private:
        static constexpr void size_validate(const auto&... allocations) noexcept
        {
            ( //
                Expects(
                    allocations.size() * sizeof(typename allocation_traits::value_type) >= type_size
                ),
                ...
            );
        }

        using cref = const T&;

        using allocation_cref = const allocation&;
        using callocation_cref = const callocation&;

    public:
        static constexpr struct data_fn
        {
            constexpr decltype(auto) operator()(callocation_cref allocation) const noexcept
            {
                return pointer_cast<T>(allocation.begin());
            }

            constexpr decltype(auto) operator()(allocation_cref allocation) const noexcept
            {
                return pointer_cast<T>(allocation.begin());
            }
        } data{};

        static constexpr struct cdata_fn
        {
            constexpr decltype(auto) operator()(callocation_cref allocation) const noexcept
            {
                return pointer_cast<T>(allocation.cbegin());
            }

            constexpr decltype(auto) operator()(allocation_cref allocation) const noexcept
            {
                return pointer_cast<T>(allocation.cbegin());
            }
        } cdata{};

        static constexpr struct get_fn
        {
            constexpr decltype(auto) operator()(allocation_cref allocation) const noexcept
            {
                return *std::launder(data_fn{}(allocation));
            }

            constexpr decltype(auto) operator()(callocation_cref allocation) const noexcept
            {
                return *std::launder(data_fn{}(allocation));
            }
        } get{};

        static constexpr struct cget_fn
        {
            constexpr decltype(auto) operator()(allocation_cref allocation) const noexcept
            {
                return std::as_const(get(allocation));
            }

            constexpr decltype(auto) operator()(callocation_cref allocation) const noexcept
            {
                return get(allocation);
            }
        } cget{};

        static constexpr struct destructor
        {
            constexpr void operator()(
                allocator_type& allocator,
                allocation_cref allocation //
            ) const noexcept
            {
                size_validate(allocation);
                typename allocator_traits::destructor{}(allocator, data(allocation));
            }
        } destroy{};

        static constexpr struct inplace_constructor
        {
            template<
                typename... Args,
                std::invocable<allocator_type&, T*, Args...> Ctor = allocator_traits::constructor>
            constexpr void operator()(
                allocator_type& allocator,
                allocation_cref allocation,
                Args&&... args
            ) const noexcept(nothrow_invocable<Ctor, allocator_type&, T*, Args...>)
            {
                size_validate(allocation);
                Ctor{}(allocator, data(allocation), cpp_forward(args)...);
            }
        } inplace_construct{};

        static constexpr struct copy_constructor
        {
            constexpr void operator()(
                allocator_type& allocator,
                callocation_cref src_allocation,
                allocation_cref dst_allocation
            ) const noexcept( //
                nothrow_invocable<
                    inplace_constructor,
                    allocator_type&,
                    allocation_cref,
                    cref> //
            )
                requires std::invocable<inplace_constructor, allocator_type&, allocation_cref, cref>
            {
                size_validate(src_allocation);
                inplace_construct(allocator, dst_allocation, get(src_allocation));
            }

        } copy_construct{};

        static constexpr struct move_constructor
        {
            constexpr void operator()(
                allocator_type& allocator,
                allocation_cref src_allocation,
                allocation_cref dst_allocation
            ) const noexcept( //
                nothrow_invocable<
                    inplace_constructor,
                    allocator_type&,
                    allocation_cref,
                    T> //
            )
                requires std::invocable<inplace_constructor, allocator_type&, allocation_cref, T>
            {
                size_validate(src_allocation);
                inplace_construct(allocator, dst_allocation, cpp_move(get(src_allocation)));
            }
        } move_construct{};

        static constexpr struct copy_assign_fn
        {
            constexpr void operator()(
                callocation_cref src_allocation,
                allocation_cref dst_allocation //
            ) const noexcept(nothrow_copy_assignable<T>)
                requires copy_assignable<T>
            {
                size_validate(src_allocation, dst_allocation);
                get(dst_allocation) = get(src_allocation);
            }

        } copy_assign{};

        static constexpr struct move_assign_fn
        {
            constexpr void operator()(
                allocation_cref src_allocation,
                allocation_cref dst_allocation //
            ) const noexcept(nothrow_move_assignable<T>)
                requires move_assignable<T>
            {
                size_validate(src_allocation, dst_allocation);
                get(dst_allocation) = cpp_move(get(src_allocation));
            }
        } move_assign{};
    };
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
