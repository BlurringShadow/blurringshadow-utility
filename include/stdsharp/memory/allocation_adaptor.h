#pragma once

#include "../utility/dispatcher.h"
#include "allocation_traits.h"
#include "launder_iterator.h"

#include <algorithm>

namespace stdsharp
{
    inline constexpr struct allocation_emplace_t
    {
    } allocation_emplace{};

    template<allocator_req Allocator, typename = Allocator::value_type>
    struct allocation_adaptor;
}

namespace stdsharp::details
{
    template<typename Allocator, typename T>
    struct allocation_adaptor_traits
    {
        using allocation_traits = allocation_traits<Allocator>;
        using allocator_traits = allocation_traits::allocator_traits;
        using allocator_type = allocation_traits::allocator_type;
        using allocation = allocation_traits::allocation;
        using callocation = allocation_traits::callocation;
        using size_type = allocator_traits::size_type;

        using ctor = allocator_traits::constructor;
        using dtor = allocator_traits::destructor;

        template<typename>
        struct forward_value;

        template<>
        struct forward_value<allocation>
        {
            using type = T&&;
        };

        template<>
        struct forward_value<callocation>
        {
            using type = const T&;
        };

        template<typename Allocation>
        using forward_value_t = forward_value<Allocation>::type;

        static constexpr void size_validate(const auto& t, const auto&... allocations) noexcept
        {
            ( //
                Expects(
                    allocations.size() * sizeof(typename allocation_traits::value_type) >=
                    t.value_size()
                ),
                ...
            );
        }

        [[nodiscard]] constexpr auto data(this const auto& t, const auto& allocation) noexcept
        {
            size_validate(t, allocation);
            return allocation_traits::template data<T>(allocation);
        }

        [[nodiscard]] constexpr decltype(auto) get(this const auto& t, const auto& allocation) //
            noexcept
        {
            size_validate(t, allocation);
            return allocation_traits::template get<T>(allocation);
        }

        template<typename... Args>
        constexpr void operator()(
            this const auto& fn,
            allocator_type& allocator,
            const allocation& allocation,
            const allocation_emplace_t /*unused*/,
            Args&&... args
        ) noexcept(nothrow_invocable<ctor, allocator_type&, T*, Args...>)
            requires std::invocable<ctor, allocator_type&, T*, Args...>
        {
            fn.do_emplace_construct(allocator, allocation, cpp_forward(args)...);
        }

        template<typename Allocation, typename Value = forward_value_t<Allocation>>
        constexpr void operator()(
            this const auto& fn,
            allocator_type& allocator,
            const Allocation& src_allocation,
            const allocation& dst_allocation
        ) noexcept(nothrow_invocable<ctor, allocator_type&, T*, Value>)
            requires std::invocable<ctor, allocator_type&, T*, Value>
        {
            fn.do_construct(allocator, src_allocation, dst_allocation);
        }

        template<typename Allocation, typename Value = forward_value_t<Allocation>>
        constexpr void operator()(
            this const auto& fn,
            const Allocation& src_allocation,
            const allocation& dst_allocation
        ) noexcept(nothrow_assignable_from<T&, Value>)
            requires std::assignable_from<T&, Value>
        {
            fn.do_assign(src_allocation, dst_allocation);
        }

        constexpr void operator()(
            this const auto& fn,
            allocator_type& allocator,
            const allocation& allocation
        ) noexcept
        {
            fn.do_destruct(allocator, allocation);
        }
    };

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

namespace stdsharp
{
    template<allocator_req Allocator, typename T>
    struct allocation_adaptor : private details::allocation_adaptor_traits<Allocator, T>
    {
    private:
        using m_base = details::allocation_adaptor_traits<Allocator, T>;

        friend m_base;

    public:
        using typename m_base::allocator_type;
        using m_base::data;
        using m_base::get;
        using m_base::operator();

        template<typename Allocation>
        using forward_value_t = m_base::template forward_value_t<Allocation>;

        static constexpr auto always_equal = true;

        [[nodiscard]] static constexpr auto value_size() noexcept { return sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_adaptor&) const = default;

    private:
        using typename m_base::ctor;
        using typename m_base::dtor;

        template<typename Allocation>
        [[nodiscard]] constexpr decltype(auto) forward_value(const Allocation& src) const noexcept
        {
            return static_cast<forward_value_t<Allocation>>(get(src));
        }

        constexpr void do_emplace_construct(auto& allocator, auto& allocation, auto&&... args) const
        {
            ctor{}(allocator, data(allocation), cpp_forward(args)...);
        }

        constexpr void do_construct(auto& allocator, auto& src_allocation, auto& dst_allocation) //
            const
        {
            ctor{}(allocator, data(dst_allocation), forward_value(src_allocation));
        }

        constexpr void do_assign(auto& src_allocation, auto& dst_allocation) const
        {
            get(dst_allocation) = forward_value(src_allocation);
        }

        constexpr void do_destruct(auto& allocator, auto& allocation) const noexcept
        {
            dtor{}(allocator, data(allocation));
        }
    };

    template<allocator_req Allocator, typename T>
    struct allocation_adaptor<Allocator, T[]> :// NOLINT(*-c-arrays)
        private details::allocation_adaptor_traits<Allocator, T>
    {
    private:
        using m_base = details::allocation_adaptor_traits<Allocator, T>;

        friend m_base;

    public:
        using typename m_base::allocator_type;
        using typename m_base::allocation;
        using typename m_base::callocation;
        using m_base::operator();

    private:
        using typename m_base::ctor;
        using typename m_base::dtor;
        using size_t = std::size_t;

        size_t size_;

    public:
        constexpr explicit allocation_adaptor(const m_base::size_type size) noexcept: size_(size) {}

        [[nodiscard]] constexpr auto data(const auto& allocation) const noexcept
        {
            return launder_iterator{m_base::data(allocation)};
        }

        [[nodiscard]] constexpr auto get(const auto& allocation) const noexcept
        {
            return std::views::counted(data(allocation), size());
        }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto value_size() const noexcept { return size() * sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_adaptor&) const = default;

    private:
        template<typename Allocation>
        using forward_value_t = m_base::template forward_value_t<Allocation>;

        [[nodiscard]] constexpr auto forward_data(const callocation& allocation) //
            const noexcept
        {
            return data(allocation);
        }

        [[nodiscard]] constexpr auto forward_data(const allocation& allocation) //
            const noexcept
        {
            return std::move_iterator{data(allocation)};
        }

        constexpr void do_emplace_construct(auto& allocator, auto& allocation, auto&&... args) const
        {
            auto begin = m_base::data(allocation);
            for(const auto end = begin + size(); begin < end; ++begin)
                ctor{}(allocator, begin, cpp_forward(args)...);
        }

        constexpr void
            do_construct(auto& allocator, auto& src_allocation, auto& dst_allocation) const
        {
            const auto& dst_begin = m_base::data(dst_allocation);
            const auto& src_begin = forward_data(src_allocation);
            const auto count = size();

            for(size_t i = 0; i < count; ++i) ctor{}(allocator, dst_begin + i, src_begin[i]);
        }

        constexpr void do_assign(auto& src_allocation, auto& dst_allocation) const
        {
            std::ranges::copy_n(forward_data(src_allocation), size(), data(dst_allocation));
        }

        constexpr void do_destruct(auto& allocator, auto& allocation) const noexcept
        {
            auto&& begin = m_base::data(allocation);
            for(const auto& end = begin + size(); begin < end; ++begin)
                dtor{}(allocator, std::launder(begin));
        }

    public:
        static constexpr struct iter_construct_t
        {
        } iter_construct{};

        template<std::forward_iterator Iter, typename ValueT = std::iter_reference_t<Iter>>
        constexpr void operator()(
            allocator_type& allocator,
            const allocation& allocation,
            const iter_construct_t /*unused*/,
            Iter iter
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, ValueT>)
            requires std::invocable<ctor, allocator_type&, T*, ValueT>
        {
            auto begin = m_base::data(allocation);
            for(const auto end = begin + size(); begin < end; ++begin, ++iter)
                ctor{}(allocator, begin, *iter);
        }
    };

    template<allocator_req Allocator, typename T>
    struct allocation_adapto<Allocator, T> :
        private details::allocation_adaptor_traits<Allocator, T>
    {
    private:
        using m_base = details::allocation_adaptor_traits<Allocator, T>;

        friend m_base;

    public:
        using typename m_base::allocator_type;
        using m_base::data;
        using m_base::get;
        using m_base::operator();

        template<typename Allocation>
        using forward_value_t = m_base::template forward_value_t<Allocation>;

        static constexpr auto always_equal = true;

        [[nodiscard]] static constexpr auto value_size() noexcept { return sizeof(T); }

        [[nodiscard]] bool operator==(const allocation_adaptor&) const = default;

    private:
        using typename m_base::ctor;
        using typename m_base::dtor;

        template<typename Allocation>
        [[nodiscard]] constexpr decltype(auto) forward_value(const Allocation& src) const noexcept
        {
            return static_cast<forward_value_t<Allocation>>(get(src));
        }

        constexpr void do_emplace_construct(auto& allocator, auto& allocation, auto&&... args) const
        {
            ctor{}(allocator, data(allocation), cpp_forward(args)...);
        }

        constexpr void do_construct(auto& allocator, auto& src_allocation, auto& dst_allocation) //
            const
        {
            ctor{}(allocator, data(dst_allocation), forward_value(src_allocation));
        }

        constexpr void do_assign(auto& src_allocation, auto& dst_allocation) const
        {
            get(dst_allocation) = forward_value(src_allocation);
        }

        constexpr void do_destruct(auto& allocator, auto& allocation) const noexcept
        {
            dtor{}(allocator, data(allocation));
        }
    };
}
