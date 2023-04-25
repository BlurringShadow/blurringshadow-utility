#pragma once

#include <span>

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../cassert/cassert.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename>
    struct allocator_aware_traits;

    template<allocator_req Alloc>
    using allocation = typename allocator_aware_traits<Alloc>::allocation;

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
        [[nodiscard]] constexpr allocation<allocator_type>
            operator()(allocator_type& alloc, Args&&... args)
        {
            using traits = allocator_traits<allocator_type>;

            const auto& allocation = make_allocation(alloc, sizeof(T));
            traits::construct(
                alloc,
                pointer_cast<T>(allocation.begin()),
                ::std::forward<Args>(args)...
            );
            return allocation;
        }
    };

    template<typename T>
    inline constexpr make_allocation_by_obj_fn<T> make_allocation_by_obj{};

    template<typename Alloc>
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
        using typename traits::propagate_on_container_copy_assignment;
        using typename traits::propagate_on_container_move_assignment;
        using typename traits::propagate_on_container_swap;
        using typename traits::is_always_equal;

        class [[nodiscard]] allocation
        {
            friend details::allocation_access;

            pointer ptr_ = nullptr;
            size_type size_ = 0;

        public:
            template<typename T>
            [[nodiscard]] constexpr ::std::span<T>
                as_span(const difference_type offset, const size_type count) const noexcept
            {
                precondition<::std::out_of_range>(
                    [condition = (offset * sizeof(value_type) + count * sizeof(T)) <=
                         (size_ * sizeof(value_type))] { return condition; },
                    "pointer out of range"
                );

                return {pointer_cast<T>(ptr_ + offset), count};
            }

            template<typename T>
            [[nodiscard]] constexpr auto as_span(const difference_type offset = 0) const noexcept
            {
                precondition<::std::out_of_range>(
                    [condition = offset <= size_] { return condition; },
                    "pointer out of range"
                );

                return ::std::span<T>{
                    pointer_cast<T>(ptr_ + offset),
                    pointer_cast<T>(ptr_ + size_) //
                };
            }

            [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

            [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

            [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return begin(); }

            [[nodiscard]] constexpr const_pointer cend() const noexcept { return end(); }

            [[nodiscard]] constexpr auto size() const noexcept { return size_; }

            [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ != nullptr; }

            constexpr operator bool() const noexcept { return empty(); }

            [[nodiscard]] constexpr allocation
                sub(const size_type offset, const size_type size) const noexcept
            {
                precondition<::std::out_of_range>(
                    [condition = offset + size <= size_] { return condition; },
                    "pointer out of range"
                );

                return {ptr_ + offset, size};
            }

            [[nodiscard]] constexpr allocation sub(const size_type size) const noexcept
            {
                return sub(0, size);
            }

            constexpr void allocate(allocator_type& alloc, const size_type size) noexcept
            {
                if(size_ >= size) return;

                if(ptr_ != nullptr) deallocate(alloc);

                *this = make_allocation(size);
            }

            constexpr void deallocate(allocator_type& alloc) noexcept
            {
                traits::deallocate(alloc, ptr_, size_);
                ptr_ = nullptr;
                size_ = 0;
            }
        };

    private:
        template<typename T, auto Impl>
        struct use_alloc_do_as_fn
        {
            using span_t = const ::std::span<T>&;
            using fn = decltype(Impl);

            static constexpr bool is_invocable =
                ::std::invocable<fn, allocator_type&, span_t, span_t>;

            static constexpr bool is_noexcept =
                nothrow_invocable<fn, allocator_type&, span_t, span_t>;

            template<typename Rng>
            constexpr void
                operator()(allocator_type& alloc, const allocation& dest, span_t src) const
                noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(alloc, dest.template as_span<T>(), src);
            }

            constexpr void
                operator()(allocator_type& alloc, const allocation& dest, const allocation& src)
                    const noexcept(is_noexcept)
                requires is_invocable
            {
                (*this)(alloc, dest, src.template as_span<T>());
            }

            constexpr void operator()(
                allocator_type& alloc,
                const allocation& dest,
                const allocation& src,
                const size_type count
            ) const noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(alloc, dest.template as_span<T>(count), src.template as_span<T>(count));
            }
        };

        template<typename T, auto Impl, typename Fn = decltype(Impl)>
        struct do_as_fn
        {
            using span_t = const ::std::span<T>&;

            static constexpr bool is_invocable = ::std::invocable<Fn, span_t, span_t>;

            static constexpr bool is_noexcept = nothrow_invocable<Fn, span_t, span_t>;

            constexpr void operator()(const allocation& dest, span_t src) const
                noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(dest.template as_span<T>(), src);
            }

            constexpr void operator()(const allocation& dest, const allocation& src) const
                noexcept(is_noexcept)
                requires is_invocable
            {
                (*this)(dest, src.template as_span<T>());
            }

            constexpr void
                operator()(const allocation& dest, const allocation& src, const size_type count)
                    const noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(dest.template as_span<T>(count), src.template as_span<T>(count));
            }
        };

        template<typename T>
        struct copy_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            // NOLINTNEXTLINE(*-swappable-parameters)
            constexpr void operator()(allocator_type& alloc, span_t dest, span_t src) const
                noexcept(nothrow_copy_constructible<T>)
                requires ::std::copy_constructible<T>
            {
                for(auto d_first = dest.begin(); const auto& v : src)
                    traits::construct(alloc, d_first++, v);
            }
        };

        template<typename T>
        struct move_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            // NOLINTNEXTLINE(*-swappable-parameters)
            constexpr void operator()(allocator_type& alloc, span_t dest, span_t src) const
                noexcept(nothrow_move_constructible<T>)
                requires ::std::move_constructible<T>
            {
                for(auto d_first = dest.begin(); const auto& v : src)
                    traits::construct(alloc, d_first++, ::std::move(v));
            }
        };

        template<typename T>
        struct copy_assign_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            constexpr void operator()(span_t dest, span_t src_span) const
                noexcept(nothrow_copy_assignable<T>)
                requires copy_assignable<T>
            {
                for(auto d_first = dest.begin(); const auto& v : src_span) *(d_first++) = v;
            }
        };

        template<typename T>
        struct move_assign_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            constexpr void operator()(span_t dest, span_t src) const
                noexcept(nothrow_move_constructible<T>)
                requires ::std::move_constructible<T>
            {
                for(auto d_first = dest.begin(); auto&& v : src) *(d_first++) = ::std::move(v);
            }
        };

        template<typename T>
        struct swap_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            constexpr void operator()(span_t dest, span_t src) const noexcept(nothrow_swappable<T>)
                requires ::std::swappable<T>
            {
                for(auto d_first = dest.begin(); auto& v : src)
                    ::std::ranges::swap(*(d_first++), v);
            }
        };

    public:
        template<typename T = value_type>
        struct delete_as_fn
        {
        private:
            constexpr void
                operator()(allocator_type& alloc, const ::std::span<T>& dest) const noexcept
            {
                for(auto& v : dest) traits::destroy(alloc, &v);
            }

        public:
            constexpr void operator()(allocator_type& alloc, const allocation& dest) const noexcept
            {
                (*this)(alloc, dest.template as_span<T>());
            }

            constexpr void
                operator()(allocator_type& alloc, const allocation& dest, const size_type count)
                    const noexcept
            {
                (*this)(alloc, dest.template as_span<T>(count));
            }
        };

        template<typename T = value_type>
        static constexpr delete_as_fn<T> delete_as{};

        template<typename T = value_type>
        using copy_as_fn = use_alloc_do_as_fn<T, copy_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr copy_as_fn<T> copy_as{};

        template<typename T = value_type>
        using move_as_fn = use_alloc_do_as_fn<T, move_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr move_as_fn<T> move_as{};

        template<typename T = value_type>
        using copy_assign_as_fn = do_as_fn<T, copy_assign_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr copy_assign_as_fn<T> copy_assign_as{};

        template<typename T = value_type>
        using move_assign_as_fn = do_as_fn<T, move_assign_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr move_assign_as_fn<T> move_assign_as{};

        template<typename T = value_type>
        using swap_as_fn = do_as_fn<T, swap_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr swap_as_fn<T> swap_as{};

        using allocation_ref = const allocation&;
        using allocation_pair = ::std::pair<allocator_type&, allocation&>;
        using const_allocation_pair = const ::std::pair<const allocator_type&, allocation_ref>;

        template<
            ::std::invocable<allocator_type&, allocation_ref, allocation_ref> Copy = copy_as_fn<>>
        static constexpr allocation
            copy_construct(allocator_type& alloc, allocation_ref other, Copy&& copy = {})
        {
            if(!other) [[unlikely]]
                return {};

            const auto& res = make_allocation(alloc, other.size());
            ::std::invoke(::std::forward<Copy>(copy), alloc, res, other);
            return res;
        }

        static constexpr allocation move_construct(allocation_ref other) noexcept { return other; }

    private:
        static constexpr void release(const allocation_pair dest, auto& fn) noexcept
        {
            ::std::invoke(fn, dest.first, dest.second);
            dest.second.deallocate(dest.first);
        }

    public:
        template<
            ::std::invocable<allocation_ref, allocation_ref> CopyFn =
                invocables<copy_assign_as_fn<>, delete_as_fn<>, copy_as_fn<>>>
            requires ::std::invocable<CopyFn, allocator_type&, allocation_ref> &&
            ::std::invocable<CopyFn, allocator_type&, allocation_ref, allocation_ref>
        static constexpr void
            copy_assign(allocation_pair dest, const_allocation_pair src, CopyFn copy = {})
        {
            if(!src) [[unlikely]]
            {
                release(dest, copy);
                return;
            }

            if(dest.second.size() >= src.second.size())
            {
                ::std::invoke(copy, dest.second, src.second);
                return;
            }

            release(dest, copy);
            dest.second = copy_construct(dest.first, src, ::std::move(copy));
        }

        template<
            ::std::invocable<allocation_ref, allocation_ref> CopyFn =
                invocables<copy_assign_as_fn<>, delete_as_fn<>, copy_as_fn<>>>
            requires ::std::invocable<CopyFn, allocator_type&, allocation_ref> &&
            ::std::invocable<CopyFn, allocator_type&, allocation_ref, allocation_ref> &&
            propagate_on_container_copy_assignment::value
        static constexpr void
            copy_assign(const allocation_pair dest, const_allocation_pair src, CopyFn copy = {})
        {
            if(!src) [[unlikely]]
            {
                release(dest, copy);
                dest.first = src.first;
                return;
            }

            if((is_always_equal::value || dest.first == src.first) &&
               dest.second.size() >= src.second.size())
            {
                dest.first = src.first;
                ::std::invoke(copy, dest.second, src.second);
                return;
            }

            release(dest, copy);
            dest.first = src.first;
            dest.second = copy_construct(dest.first, src.second, ::std::move(copy));
        }

        template<
            ::std::invocable<allocation_ref, allocation_ref> MoveFn =
                invocables<move_assign_as_fn<>, delete_as_fn<>, move_as_fn<>>>
            requires ::std::invocable<MoveFn, allocator_type&, allocation_ref> &&
            ::std::invocable<MoveFn, allocator_type&, allocation_ref, allocation_ref>
        static constexpr void
            move_assign(const allocation_pair dest, allocation_pair src, MoveFn move = {})
        {
            if(!src) [[unlikely]]
            {
                release(dest, move);
                return;
            }

            if(dest.first == src.first)
            {
                release(dest, move);
                dest.second = src.second;
                return;
            }

            if(dest.second.size() >= src.second.size())
            {
                ::std::invoke(move, dest.second, src.second);
                return;
            }

            release(dest, move);

            dest.second = make_allocation(dest.first, src.size());
            ::std::invoke(::std::move(move), dest.first, dest.second, src.second);
        }

        template<::std::invocable<allocator_type&, allocation_ref> MoveFn = delete_as_fn<>>
            requires propagate_on_container_move_assignment::value
        static constexpr void move_assign(
            const allocation_pair dest,
            const allocation_pair src,
            MoveFn&& move = {}
        ) noexcept
        {
            if(!src) [[unlikely]]
            {
                release(dest, move);
                dest.first = ::std::move(src.first);
                return;
            }

            if(!(is_always_equal::value || dest.first == src.first))
            {
                ::std::invoke(::std::forward<MoveFn>(move), dest.first, dest.second);
                dest.second.deallocate(dest.first);
            }

            dest.first = ::std::move(src.first);
            dest.second = src.second;
        }

    private:
        static constexpr void
            swap_impl(const allocation_pair lhs, const allocation_pair rhs, auto& swap)
        {
            STDSHARP_ASSUME(lhs.second.size() > rhs.second.size());

            const auto& rhs_allocation = make_allocation(rhs.first, lhs.second.size());
            ::std::invoke(swap, rhs.first, rhs_allocation, lhs.second);

            ::std::invoke(swap, lhs.first, lhs.second);
            ::std::invoke(swap, lhs.first, lhs.second, rhs.second);

            release(rhs, swap);

            rhs.second = rhs_allocation;
        }

    public:
        template<
            ::std::invocable<allocation_ref, allocation_ref> SwapFn =
                invocables<move_assign_as_fn<>, move_as_fn<>, delete_as_fn<>>>
            requires ::std::invocable<SwapFn, allocator_type&, allocation_ref, allocation_ref>
        static constexpr void
            swap(const allocation_pair lhs, const allocation_pair rhs, SwapFn swap = {})
        {
            if(is_always_equal::value || lhs.first == rhs.first)
            {
                ::std::swap(lhs.second, rhs.second);
                return;
            }

            const auto cmp = lhs.second.size() <=> rhs.second.size();

            if(cmp == 0) ::std::invoke(swap, lhs.second, rhs.second);
            else if(cmp > 0) swap_impl(lhs, rhs, swap);
            else swap_impl(rhs, lhs, swap);
        }

        static constexpr void
            swap(const allocation_pair lhs, const allocation_pair rhs, const auto& = 0)
            requires propagate_on_container_swap::value
        {
            ::std::swap(lhs.second, rhs.second);
            ::std::ranges::swap(lhs.first, rhs.first);
        }
    };
}

#include "../compilation_config_out.h"