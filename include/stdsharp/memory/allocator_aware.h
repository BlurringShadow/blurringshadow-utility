#pragma once

#include <span>

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../cassert/cassert.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<allocator_req>
    struct allocator_aware_traits;

    template<allocator_req Alloc>
    using allocation = allocator_aware_traits<Alloc>::allocation;

    template<allocator_req Alloc, typename ValueType = Alloc::value_type>
    using allocation_for = allocator_aware_traits<Alloc>::template allocation_for<ValueType>;

    namespace details
    {
        struct allocation_access
        {
            template<typename allocator_type>
            static constexpr allocation<allocator_type> make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type>& hint = nullptr
            )
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::allocate(alloc, size, hint), size};
            }

            template<typename allocator_type>
            static constexpr allocation<allocator_type> try_make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type>& hint = nullptr
            ) noexcept
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::try_allocate(alloc, size, hint), size};
            }
        };
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type>& hint = nullptr
    )
    {
        return details::allocation_access::make_allocation(alloc, size, hint);
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> try_make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type>& hint = nullptr
    ) noexcept
    {
        return details::allocation_access::try_make_allocation(alloc, size, hint);
    }

    template<typename T>
    struct make_allocation_by_obj_fn
    {
        template<allocator_req allocator_type, typename... Args>
        [[nodiscard]] constexpr allocation_for<allocator_type, T>
            operator()(allocator_type& alloc, Args&&... args)
        {
            using traits = allocator_traits<allocator_type>;

            const auto& allocation = make_allocation(alloc, sizeof(T));
            traits::construct(alloc, pointer_cast<T>(allocation.begin()), cpp_forward(args)...);
            return allocation;
        }
    };

    template<typename T>
    inline constexpr make_allocation_by_obj_fn<T> make_allocation_by_obj{};

    template<allocator_req Alloc>
    struct allocator_aware_traits : allocator_traits<Alloc>
    {
        using traits = allocator_traits<Alloc>;

        using typename traits::value_type;
        using typename traits::pointer;
        using typename traits::const_pointer;
        using typename traits::const_void_pointer;
        using typename traits::size_type;
        using typename traits::difference_type;
        using typename traits::allocator_type;

        using traits::propagate_on_copy_v;
        using traits::propagate_on_move_v;
        using traits::propagate_on_swap_v;
        using traits::always_equal_v;

        class allocation;

        template<typename ValueType = value_type>
        class allocation_for;

        static constexpr auto simple_movable = propagate_on_move_v || always_equal_v;

        static constexpr auto simple_swappable = propagate_on_swap_v || always_equal_v;

        template<typename ValueType>
        static constexpr allocation_for<ValueType>
            copy_construct(allocator_type& alloc, const allocation_for<ValueType>& other)
        {
            allocation_for<ValueType> allocation = make_allocation(alloc, sizeof(ValueType));
            allocation.construct(alloc, other.get_const());
            return allocation;
        }

        template<typename ValueType>
        static constexpr allocation_for<ValueType>
            move_construct(allocator_type&, const allocation_for<ValueType>& other) noexcept
        {
            return other;
        }

        template<typename ValueType>
        static constexpr void
            destroy(allocator_type& alloc, allocation_for<ValueType>& allocation) noexcept
        {
            allocation.deallocate(alloc);
        }

    private:
        template<typename ValueType>
        static constexpr void copy_impl(
            allocator_type& dst_alloc,
            const allocation_for<ValueType>& dst_allocation,
            const allocation_for<ValueType>& src_allocation
        )
        {
            if(src_allocation.has_value())
            {
                if(dst_allocation.has_value()) dst_allocation.get() = src_allocation.get_const();
                else dst_allocation.construct(dst_alloc, src_allocation.get_const());
            }
            else dst_allocation.destroy(dst_alloc);
        }

    public:
        template<typename ValueType>
        static constexpr void copy_assign(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            const allocator_type&,
            const allocation_for<ValueType>& src_allocation
        ) noexcept(nothrow_copyable<value_type>)
        {
            copy_impl(dst_alloc, dst_allocation, src_allocation);
        }

        template<typename ValueType>
        static constexpr void copy_assign(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            const allocator_type& src_alloc,
            const allocation_for<ValueType>& src_allocation
        ) noexcept(nothrow_copyable<value_type>)
            requires propagate_on_copy_v
        {
            if constexpr(!always_equal_v)
                if(dst_alloc != src_alloc)
                {
                    dst_allocation.destroy(dst_alloc);
                    dst_allocation.deallocate(dst_alloc);
                }

            dst_alloc = src_alloc;
            copy_impl(dst_alloc, dst_allocation, src_allocation);
        }

        template<typename ValueType>
        static constexpr void move_assign(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            const allocator_type&,
            allocation_for<ValueType>& src_allocation
        ) noexcept(nothrow_movable<value_type>)
        {
            if(src_allocation.has_value())
            {
                if(dst_allocation.has_value())
                    dst_allocation.get() = cpp_move(src_allocation.get());
                else dst_allocation.construct(dst_alloc, cpp_move(src_allocation.get()));
            }
            else dst_allocation.destroy(dst_alloc);

            destroy(src_allocation);
        }

        template<typename ValueType>
        static constexpr void move_assign(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            const allocator_type& src_alloc,
            allocation_for<ValueType>& src_allocation
        ) noexcept
            requires propagate_on_move_v
        {
            if constexpr(!always_equal_v)
                if(dst_alloc != src_alloc)
                {
                    dst_allocation.destroy(dst_alloc);
                    dst_allocation.deallocate(dst_alloc);
                }

            dst_alloc = src_alloc;
            dst_allocation = ::std::exchange(src_allocation, {});
        }

    private:
        template<typename ValueType>
        static constexpr void swap_impl(
            [[maybe_unused]] allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            [[maybe_unused]] allocator_type& src_alloc,
            allocation_for<ValueType>& src_allocation
        )
        {
            if constexpr(!always_equal_v)
                if(dst_alloc != src_alloc)
                {
                    ::std::ranges::swap(dst_allocation.get(), src_allocation.get());
                    return;
                }
            ::std::swap(dst_allocation, src_allocation);
        }

    public:
        template<typename ValueType>
        static constexpr void swap(
            allocator_type& dst_alloc,
            allocation_for<ValueType>& dst_allocation,
            allocator_type& src_alloc,
            allocation_for<ValueType>& src_allocation
        ) noexcept(always_equal_v || nothrow_swappable<value_type>)
        {
            swap_impl(dst_alloc, dst_allocation, src_alloc, src_allocation);
            if constexpr(propagate_on_swap_v) ::std::ranges::swap(dst_alloc, src_alloc);
        }
    };

    template<allocator_req Allocator>
    class [[nodiscard]] allocator_aware_traits<Allocator>::allocation
    {
        friend details::allocation_access;

        pointer ptr_ = nullptr;
        size_type size_ = 0;

    protected:
        constexpr allocation(pointer ptr, const size_type size) noexcept: ptr_(ptr), size_(size) {}

    public:
        allocation() = default;

        [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

        [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

        [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_pointer cend() const noexcept { return end(); }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ != nullptr; }

        constexpr operator bool() const noexcept { return empty(); }

        constexpr void allocate(allocator_type& alloc, const size_type size) noexcept
        {
            if(size_ >= size) return;

            if(ptr_ != nullptr) deallocate(alloc);

            *this = make_allocation(size);
        }

        constexpr void deallocate(allocator_type& alloc) noexcept
        {
            traits::deallocate(alloc, ptr_, size_);
            *this = {};
        }
    };

    template<allocator_req Allocator>
    template<typename ValueType>
    class [[nodiscard]] allocator_aware_traits<Allocator>::allocation_for
    {
    public:
        using value_type = ValueType;

    private:
        allocation allocation_{};
        bool has_value_ = false;

    public:
        allocation_for() = default;

        allocation_for(const allocation& allocation) noexcept(!is_debug): allocation_(allocation)
        {
            precondition<::std::invalid_argument>( //
                [condition = allocation_.size() >= sizeof(value_type)] { return condition; }
            );
        }

        [[nodiscard]] constexpr decltype(auto) get() const noexcept
        {
            return pointer_cast<value_type>(base::begin());
        }

        [[nodiscard]] constexpr const auto& get_const() const noexcept { return get(); }

        [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }

        constexpr void allocate(
            traits::allocator_type& alloc,
            const traits::const_void_pointer& hint = nullptr
        )
        {
            if(allocation_) return;

            allocation_ = make_allocation(alloc, sizeof(value_type), hint);
        }

        constexpr void deallocate(traits::allocator_type& alloc) noexcept
        {
            if(!allocation_) return;
            if(has_value()) destroy(alloc);
            allocation_.deallocate(alloc);
        }

        template<
            typename... Args,
            auto Req = traits::template construct_req<value_type, Args...> // clang-format off
        > requires(Req >= expr_req::well_formed)
        [[nodiscard]] constexpr decltype(auto) construct(traits::allocator_type& alloc, Args&&... args)
            noexcept(Req >= expr_req::no_exception) // clang-format on
        {
            if(!allocation_) allocate(alloc);
            else if(has_value()) destroy(alloc);

            decltype(auto) res = traits::construct(alloc, get(), cpp_forward(args)...);
            has_value_ = true;
            return res;
        }

        constexpr void destroy(traits::allocator_type& alloc) noexcept
        {
            if(!has_value()) return;
            traits::destroy(alloc, get());
            has_value_ = false;
        }
    };

    template<typename T>
        requires requires(const T t, typename T::allocator_type alloc) //
    {
        requires ::std::derived_from<T, allocator_aware_traits<decltype(alloc)>>;
        // clang-format off
        { t.get_allocator() } noexcept ->
            ::std::same_as<decltype(alloc)>; // clang-format on
    }
    struct allocator_aware_ctor : T
    {
        using T::T;
        using typename T::allocator_type;
        using allocator_traits = allocator_traits<allocator_type>;

    private:
        template<typename... Args>
        static constexpr auto alloc_last_ctor = constructible_from_test<T, Args..., allocator_type>;

        template<typename... Args>
        static constexpr auto alloc_arg_ctor =
            constructible_from_test<T, ::std::allocator_arg_t, allocator_type, Args...>;

    public:
        template<typename... Args>
            requires ::std::constructible_from<allocator_type> &&
            (alloc_last_ctor<Args...> >= expr_req::well_formed)
        constexpr allocator_aware_ctor(Args&&... args) //
            noexcept(alloc_last_ctor<Args...> >= expr_req::no_exception):
            T(cpp_forward(args)..., allocator_type{})
        {
        }

        template<typename... Args>
            requires ::std::constructible_from<allocator_type> &&
            (alloc_arg_ctor<Args...> >= expr_req::well_formed)
        constexpr allocator_aware_ctor(Args&&... args) //
            noexcept(alloc_arg_ctor<Args...> >= expr_req::no_exception):
            T(::std::allocator_arg, allocator_type{}, cpp_forward(args)...)
        {
        }

        template<typename... Args>
            requires ::std::constructible_from<T, Args..., allocator_type>
        constexpr allocator_aware_ctor(
            const ::std::allocator_arg_t,
            const allocator_type& alloc,
            Args&&... args
        ) noexcept(nothrow_constructible_from<T, Args..., allocator_type>):
            T(cpp_forward(args)..., alloc)
        {
        }

        constexpr allocator_aware_ctor(const allocator_aware_ctor& other) //
            noexcept(nothrow_constructible_from<T, const T&, allocator_type>)
            requires ::std::constructible_from<T, const T&, allocator_type>
            :
            T( //
                static_cast<const T&>(other),
                allocator_traits::select_on_container_copy_construction(other.get_allocator())
            )
        {
        }

        constexpr allocator_aware_ctor(allocator_aware_ctor&& other) //
            noexcept(nothrow_constructible_from<T, T, allocator_type>)
            requires ::std::constructible_from<T, T, allocator_type>
            : T(static_cast<T&&>(other), other.get_allocator())
        {
        }

        allocator_aware_ctor& operator=(const allocator_aware_ctor&) = default;
        allocator_aware_ctor&
            operator=(allocator_aware_ctor&&) = default; // NOLINT(*-noexcept-move-constructor)
        ~allocator_aware_ctor() = default;
    };
}

#include "../compilation_config_out.h"