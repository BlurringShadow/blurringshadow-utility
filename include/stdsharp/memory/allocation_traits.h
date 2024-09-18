#pragma once

#include "../ranges/ranges.h"
#include "../utility/dispatcher.h"
#include "allocator_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    struct allocation_traits;

    template<typename Rng, typename Alloc>
    concept callocations = std::ranges::input_range<Rng> &&
        std::convertible_to<range_const_reference_t<Rng>,
                            const typename allocation_traits<Alloc>::callocation&>;

    template<typename Rng, typename Alloc>
    concept allocations = requires(const allocation_traits<Alloc>::allocation& alloction) {
        requires callocations<Rng, Alloc>;
        requires std::ranges::output_range<Rng, decltype(alloction)>;
        requires std::convertible_to<range_const_reference_t<Rng>, decltype(alloction)>;
    };

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_constructible =
        std::invocable<typename allocation_traits<Alloc>::on_construct, Alloc&, Src, Dst, Fn>;

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_assignable =
        std::invocable<typename allocation_traits<Alloc>::on_assign, Alloc&, Dst, Fn>;

    template<typename Alloc, typename Dst, typename Fn>
    concept allocation_destructible =
        std::invocable<typename allocation_traits<Alloc>::on_destroy, Alloc&, Dst, Fn>;

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_nothrow_constructible =
        nothrow_invocable<typename allocation_traits<Alloc>::on_construct_fn, Alloc&, Src, Dst, Fn>;

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_nothrow_assignable =
        nothrow_invocable<typename allocation_traits<Alloc>::on_assign_fn, Alloc&, Dst, Fn>;

    template<typename Alloc, typename Dst, typename Fn>
    concept allocation_nothrow_destructible =
        nothrow_invocable<typename allocation_traits<Alloc>::on_destroy_fn, Alloc&, Dst, Fn>;

    template<allocator_req Alloc>
    using callocation = allocation_traits<Alloc>::callocation;

    template<allocator_req Alloc>
    using allocation = allocation_traits<Alloc>::allocation;

    template<allocator_req Alloc>
    inline constexpr auto empty_allocation = allocation_traits<Alloc>::empty;

    template<allocator_req Alloc>
    struct allocation_traits
    {
        using allocator_type = Alloc;
        using allocator_traits = allocator_traits<allocator_type>;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using const_pointer = allocator_traits::const_pointer;
        using cvp = allocator_traits::const_void_pointer;

    private:
        template<lifetime_req>
        struct dynamic;

        template<
            typename Ptr = const_pointer,
            typename Base =
                std::ranges::subrange<Ptr, const_pointer, std::ranges::subrange_kind::sized>>
        struct basic_allocation : Base
        {
            using Base::Base;

            using allocator_type = Alloc;
            using traits = allocation_traits;

            basic_allocation() = default;

            constexpr basic_allocation(const Ptr& p, const size_type& diff) noexcept:
                Base(p, p + diff)
            {
            }

            [[nodiscard]] constexpr auto cbegin() const noexcept
            {
                return std::ranges::cbegin(*this);
            }

            [[nodiscard]] constexpr auto cend() const noexcept { return std::ranges::cend(*this); }
        };

        using ctor = allocator_traits::constructor;

        using dtor = allocator_traits::destructor;

    public:
        struct callocation : basic_allocation<>
        {
        private:
            using m_base = basic_allocation<>;

        public:
            using m_base::m_base;
        };

        static constexpr struct allocation : basic_allocation<allocator_pointer<Alloc>>
        {
        private:
            using m_base = basic_allocation<allocator_pointer<Alloc>>;

        public:
            using m_base::m_base;

            constexpr operator callocation() const noexcept
            {
                return callocation{this->begin(), this->end()};
            }
        } empty;

        [[nodiscard]] static constexpr allocation
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return {allocator_traits::allocate(alloc, size, hint), size};
        }

        [[nodiscard]] static constexpr allocation try_allocate(
            allocator_type& alloc,
            const size_type size,
            const cvp hint = nullptr
        ) noexcept
        {
            return {allocator_traits::try_allocate(alloc, size, hint), size};
        }

        static constexpr void deallocate(allocator_type& alloc, allocation& dst_allocation) noexcept
        {
            allocator_traits::deallocate(alloc, data<>(dst_allocation), dst_allocation.size());
            dst_allocation = empty;
        }

        static constexpr struct object_construct_t
        {
        } object_construct{};

        static constexpr struct object_assign_t
        {
        } object_assign{};

        static constexpr struct object_destroy_t
        {
        } object_destroy{};

        static constexpr struct object_swap_t
        {
        } object_swap{};

        template<typename T>
        struct object;

        template<lifetime_req Req>
        using dynamic_object = object<dynamic<allocator_traits::template type_req<fake_type<Req>>>>;

        template<>
        class object<dynamic<lifetime_req::ill_formed()>>
        {
            allocation allocation_;
            cttid_t type_;

        public:
            [[nodiscard]] constexpr callocation allocation() const noexcept { return allocation_; }

            [[nodiscard]] constexpr cttid_t type() const noexcept { return type_; }

            [[nodiscard]] constexpr bool has_value() const noexcept { return type() != cttid_t{}; }

            template<typename T>
            [[nodiscard]] constexpr const T* data() const noexcept
            {
                assert_equal(type(), cttid<T>);
                return pointer_cast<const T>(allocation_.begin());
            }

            template<typename T>
            [[nodiscard]] constexpr T* data() noexcept
            {
                assert_equal(type(), cttid<T>);
                return pointer_cast<T>(allocation_.begin());
            }

            template<typename T, typename Self, typename U = forward_like_t<Self, T>>
            [[nodiscard]] constexpr U get(this Self&& self) noexcept
            {
                const auto ptr = fwd_cast<object>(cpp_forward(self)).template data<T>();
                assert_not_null(ptr);
                return static_cast<U>(*ptr);
            }

            template<
                typename T,
                typename... Args,
                std::invocable<allocator_type&, T*, Args...> Ctor = allocator_traits::constructor>
            constexpr void emplace(allocator_type& alloc, Args&&... args)
                noexcept(nothrow_invocable<Ctor, allocator_type&, T*, Args...>)
            {
                Expects(allocation_.size() * sizeof(value_type) >= sizeof(T));
                Ctor{}(alloc, data(), cpp_forward(args)...);
                type_ = cttid<T>;
            }

            constexpr void destroy(allocator_type& alloc) noexcept
            {
                if(!has_value()) return;

                allocator_traits::destroy(alloc, data());
                type_ = cttid_t{};
            }

            object() = default;

            object(const object&) = delete;

            constexpr object(object&& other) noexcept:
                allocation_(other.allocation_), type_(other.type_)
            {
                other = {};
            }

            object& operator=(const object&) = delete;

            constexpr object& operator=(object&& other) noexcept
            {
                if(this == &other) return *this;

                destroy();
                allocation_ = other.allocation_;
                type_ = other.type_;
                other = {};

                return *this;
            }

            constexpr ~object() { destroy(); }

            constexpr object(const struct allocation& allocation) noexcept: allocation_(allocation)
            {
            }

            constexpr object(
                const struct allocation& allocation,
                allocator_type& alloc,
                auto&&... args
            ) noexcept(noexcept(emplace(alloc, cpp_forward(args)...)))
                requires requires { emplace(alloc, cpp_forward(args)...); }
                : object(allocation)
            {
                emplace(alloc, cpp_forward(args)...);
            }
        };

        template<lifetime_req Req>
        class object<dynamic<Req>>
        {
        public:
            static constexpr auto req = Req;

        private:
            cttid_t type_;
            std::size_t size_ = 0;
            invocables<
                dispatcher<req.default_construct, void, allocator_type&, const allocation&>,
                dispatcher<
                    req.move_construct,
                    void,
                    allocator_type&,
                    const allocation&,
                    const allocation&>,
                dispatcher<
                    req.copy_construct,
                    void,
                    allocator_type&,
                    const allocation&,
                    const callocation&>,
                dispatcher<req.move_assign, void, const allocation&, const allocation&>,
                dispatcher<req.copy_assign, void, const allocation&, const callocation&>,
                dispatcher<req.destruct, void, allocator_type&, const allocation&>,
                dispatcher<req.swap, void, const allocation&, const allocation&>>
                dispatchers_;

            static constexpr auto self_cast = fwd_cast<object>;

        public:
            [[nodiscard]] constexpr cttid_t type() const noexcept { return type_; }

            [[nodiscard]] constexpr bool has_value() const noexcept { return type() != cttid_t{}; }

            template<
                typename T,
                typename... Args,
                std::invocable<allocator_type&, T*, Args...> Ctor = allocator_traits::constructor>
            constexpr void emplace(allocator_type& alloc, Args&&... args)
                noexcept(nothrow_invocable<Ctor, allocator_type&, T*, Args...>)
            {
                Expects(allocation_.size() * sizeof(value_type) >= sizeof(T));
                Ctor{}(alloc, data(), cpp_forward(args)...);
                type_ = cttid<T>;
                size_ = sizeof(T);
            }

            constexpr void operator()(
                const object_construct_t /*unused*/,
                allocator_type& alloc,
                const allocation& src
            ) const noexcept(is_noexcept(req.default_construct))
                requires(is_well_formed(req.default_construct))
            {
                Expects(src.size() * sizeof(value_type) >= size_);
                dispatchers_.template get<0>()(alloc, src);
            }

            constexpr void operator()(
                const object_construct_t /*unused*/,
                allocator_type& alloc,
                const allocation& src,
                const allocation& other //
            ) const noexcept(is_noexcept(req.move_construct))
                requires(is_well_formed(req.move_construct))
            {
                Expects(src.size() * sizeof(value_type) >= size_);
                Expects(other.size() * sizeof(value_type) >= size_);
                dispatchers_.template get<1>()(alloc, src, other);
            }

            constexpr void operator()(
                const object_construct_t /*unused*/,
                allocator_type& alloc,
                const allocation& src,
                const allocation& other //
            ) const noexcept(is_noexcept(req.copy_construct))
                requires(is_well_formed(req.copy_construct))
            {
                Expects(src.size() * sizeof(value_type) >= size_);
                Expects(other.size() * sizeof(value_type) >= size_);
                dispatchers_.template get<2>()(alloc, src, other);
            }

            constexpr void operator()(
                const object_assign_t /*unused*/,
                const allocation& src,
                const allocation& other
            ) const noexcept(is_noexcept(req.move_assign))
                requires(is_well_formed(req.move_assign))
            {
                Expects(src.size() * sizeof(value_type) >= size_);
                Expects(other.size() * sizeof(value_type) >= size_);
                dispatchers_.template get<3>()(src, other);
            }

            constexpr void operator()(
                const object_assign_t /*unused*/,
                const allocation& src,
                const callocation& other
            ) const noexcept(is_noexcept(req.copy_assign))
                requires(is_well_formed(req.copy_assign))
            {
                Expects(src.size() * sizeof(value_type) >= size_);
                Expects(other.size() * sizeof(value_type) >= size_);
                dispatchers_.template get<4>()(src, other);
            }

            constexpr void operator()(
                const object_destroy_t /*unused*/,
                allocator_type& alloc,
                const allocation& src
            ) const noexcept
            {
                Expects(src.size() * sizeof(value_type) >= size_);
                dispatchers_.template get<5>()(alloc, src); // NOLINT
            }

            constexpr void operator()(
                const object_swap_t /*unused*/,
                const allocation& src,
                const allocation& other
            ) const noexcept(is_noexcept(req.swap))
                requires(is_well_formed(req.swap))
            {
                Expects(src.size() * sizeof(value_type) >= size_);
                Expects(other.size() * sizeof(value_type) >= size_);
                dispatchers_.template get<6>()(src, other); // NOLINT
            }
        };
    };

    template<allocator_req Alloc>
    template<typename T>
    struct allocation_traits<Alloc>::object
    {
        static constexpr struct data_fn
        {
            [[nodiscard]] constexpr T* operator()(const allocation& allocation) const noexcept
            {
                return pointer_cast<T>(allocation.begin());
            }

            [[nodiscard]] constexpr T* operator()(const callocation& allocation) const noexcept
            {
                return pointer_cast<T>(allocation.begin());
            }
        } data{};

        static constexpr struct get_fn
        {
            [[nodiscard]] constexpr T& operator()(const allocation& allocation) const noexcept
            {
                return *data(allocation);
            }

            [[nodiscard]] constexpr T& operator()(const callocation& allocation) const noexcept
            {
                return *data(allocation);
            }
        } get{};

        constexpr void operator()(
            const object_construct_t /*unused*/,
            allocator_type& alloc,
            const allocation& src,
            const allocation& other //
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, const T&>)
            requires std::invocable<ctor, allocator_type&, T*, const T&>
        {
            ctor{}(alloc, data(src), get(other));
        }

        constexpr void operator()(
            const object_construct_t /*unused*/,
            allocator_type& alloc,
            const allocation& src,
            const callocation& other //
        ) const noexcept(nothrow_invocable<ctor, allocator_type&, T*, const T&>)
            requires std::invocable<ctor, allocator_type&, T*, const T&>
        {
            ctor{}(alloc, data(src), get(other));
        }

        constexpr void operator()(
            const object_destroy_t /*unused*/,
            allocator_type& alloc,
            const allocation& src
        ) const noexcept
        {
            dtor{}(alloc, data(src));
        }

        constexpr void operator()(
            const object_assign_t /*unused*/,
            const allocation& src,
            const allocation& other
        ) const noexcept(nothrow_move_assignable<T>)
            requires move_assignable<T>
        {
            get(src) = cpp_move(get(other));
        }

        constexpr void operator()(
            const object_assign_t /*unused*/,
            const allocation& src,
            const callocation& other
        ) const noexcept(nothrow_copy_assignable<T>)
            requires copy_assignable<T>
        {
            get(src) = get(other);
        }

        constexpr void operator()(
            const object_swap_t /*unused*/,
            const allocation& src,
            const allocation& other
        ) const noexcept(nothrow_swappable<T>)
            requires std::swappable<T>
        {
            std::ranges::swap(get(src), get(other));
        }
    };

    template<allocator_req Alloc>
    class allocation_manager
    {
        static constexpr struct get_alloc_fn
        {
            [[nodiscard]] constexpr const Alloc& operator()(const auto& self) noexcept
                requires requires {
                    { self.get_allocator() } -> std::convertible_to<const Alloc&>;
                }
            {
                return self.get_allocator();
            }

            [[nodiscard]] constexpr Alloc& operator()(auto& self) noexcept
                requires requires {
                    { self.get_allocator() } -> std::convertible_to<Alloc&>;
                }
            {
                return self.get_allocator();
            }
        } get_alloc{};

        static constexpr struct get_allocations_fn
        {
            [[nodiscard]] constexpr auto operator()(const auto& self) noexcept
                requires requires {
                    { self.allocations() } -> callocations<Alloc>;
                }
            {
                return self.allocations();
            }

            [[nodiscard]] constexpr auto operator()(auto& self) noexcept
                requires requires {
                    { self.allocations() } -> allocations<Alloc>;
                }
            {
                return self.allocations();
            }
        } get_allocations{};

        template<typename T>
        using allocations_t = std::invoke_result_t<get_allocations_fn, T>;

    public:
        using allocator_type = Alloc;

        using allocator_traits = allocator_traits<allocator_type>;
        using allocation_traits = allocation_traits<Alloc>;
        using allocation = allocation_traits::allocation;
        using callocation = allocation_traits::callocation;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using cvp = allocation_traits::cvp;

        constexpr auto allocate(
            this auto& self,
            const size_type size,
            const cvp hint = nullptr //
        )
        {
            return allocation_traits::allocate(get_alloc(self), size, hint);
        }

        constexpr auto try_allocate(
            this auto& self,
            const size_type size,
            const cvp hint = nullptr //
        ) noexcept
        {
            return allocation_traits::try_allocate(get_alloc(self), size, hint);
        }

        constexpr void deallocate(this auto& self, allocation& allocation) noexcept
        {
            allocation_traits::deallocate(get_alloc(self), allocation);
        }

        constexpr void deallocate(this auto& self) noexcept
            requires requires(allocation a) {
                { get_allocations(self) } -> callocations<Alloc>;
                self.deallocate(a);
            }
        {
            for(allocation& allocation : get_allocations(self)) self.deallocate(allocation);
        }

        constexpr void destroy(this auto& self) noexcept
            requires requires(allocation a) {
                { get_allocations(self) } -> callocations<Alloc>;
                self.destroy(a);
            }
        {
            for(allocation& allocation : get_allocations(self)) self.destroy(allocation);
        }

        template<typename T>
            requires allocator_traits::propagate_on_move_v
        constexpr void move_assign(this T& self, T&& other)
            noexcept(noexcept(self.destroy()) && nothrow_move_assignable<allocations_t<T&>>)
        {
            self.destroy();
            self.deallocate();
            get_alloc(self) = cpp_move(get_alloc(other));
            get_allocations(self) = cpp_move(get_allocations(other));
        }

        template<typename T>
            requires allocator_traits::propagate_on_copy_v && allocator_traits::always_equal_v
        constexpr void copy_assign(this T& self, T&& other)
            noexcept(noexcept(self.destroy()) && nothrow_move_assignable<allocations_t<T&>>)
        {
            get_alloc(self) = cpp_move(get_alloc(other));
            get_allocations(self).apply([&self](allocation& allocation) { self.assign(allocation); }
            ) = cpp_move(get_allocations(other));
        }
    };
}
