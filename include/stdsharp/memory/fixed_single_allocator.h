#pragma once

#include "aligned.h"
#include "allocator_traits.h"

namespace stdsharp
{
    inline constexpr auto default_soo_size = sizeof(std::max_align_t);

    template<std::size_t Size = default_soo_size>
    class fixed_single_resource
    {
    public:
        static constexpr auto size = Size;

        fixed_single_resource() = default;
        fixed_single_resource(const fixed_single_resource&) = delete;
        fixed_single_resource(fixed_single_resource&&) = delete;
        fixed_single_resource& operator=(const fixed_single_resource&) = delete;
        fixed_single_resource& operator=(fixed_single_resource&&) = delete;
        ~fixed_single_resource() = default;

        [[nodiscard]] constexpr void*
            allocate(const std::size_t s, const std::size_t alignment = max_alignment_v) noexcept
        {
            if(s > size || p_ != nullptr) return nullptr;

            if(std::is_constant_evaluated())
            {
                Expects(alignment <= max_alignment_v);
                p_ = buffer_.data();
            }
            else if(const auto span = align(alignment, s, std::span{buffer_}); !span.empty())
                p_ = span.data();

            return p_;
        }

        constexpr void deallocate(void* const p) noexcept
        {
            Expects(p_ == p);
            p_ = nullptr;
        }

        [[nodiscard]] constexpr auto contains(const void* const in_ptr) noexcept
        {
            return in_ptr == p_;
        }

        [[nodiscard]] constexpr const auto& buffer() const noexcept { return buffer_; }

        [[nodiscard]] constexpr bool operator==(const fixed_single_resource& other) const noexcept
        {
            return this == &other;
        }

    private:
        void* p_ = nullptr;
        alignas(std::max_align_t) std::array<byte, size> buffer_{};
    };

    template<typename T, std::size_t Size>
    class fixed_single_allocator
    {
        [[nodiscard]] static constexpr auto byte_size(const std::size_t s) { return s * sizeof(T); }

    public:
        using value_type = T;
        using propagate_on_container_move_assignment = std::true_type;

        static constexpr auto size = Size;

        using resource_type = fixed_single_resource<size>;

        constexpr explicit fixed_single_allocator(resource_type& src) noexcept: src_(src) {}

        template<typename U>
        struct rebind
        {
            using other = fixed_single_allocator<U, Size>;
        };

        template<typename U>
        constexpr fixed_single_allocator(const typename rebind<U>::other other) noexcept:
            fixed_single_allocator(other.resource())
        {
        }

        [[nodiscard]] constexpr T* allocate(const std::size_t s)
        {
            const auto p = resource().allocate(byte_size(s), alignof(T));
            return p == nullptr ? throw std::bad_alloc{} : pointer_cast<T>(p);
        }

        [[nodiscard]] constexpr T* try_allocate(const std::size_t s)
        {
            return pointer_cast<T>(resource().allocate(byte_size(s), alignof(T)));
        }

        constexpr void deallocate(T* const ptr, const std::size_t /*unused*/) noexcept
        {
            resource().deallocate(to_void_pointer(ptr));
        }

        [[nodiscard]] constexpr resource_type& resource() const noexcept { return src_.get(); }

        [[nodiscard]] constexpr bool operator==(const fixed_single_allocator other) const noexcept
        {
            return resource() == other.resource();
        }

        [[nodiscard]] constexpr bool contains(const T* const ptr) const noexcept
        {
            return resource().contains(ptr);
        }

    private:
        std::reference_wrapper<resource_type> src_;
    };

    template<std::size_t Size>
    fixed_single_allocator(fixed_single_resource<Size>&) -> fixed_single_allocator<byte, Size>;

    template<typename T = byte>
    struct make_fixed_single_allocator_fn
    {
        template<std::size_t Size>
        [[nodiscard]] constexpr auto operator()(fixed_single_resource<Size>& buffer) const noexcept
        {
            return fixed_single_allocator<T, Size>{buffer};
        }
    };

    template<typename T = byte>
    inline constexpr make_fixed_single_allocator_fn<T> make_fixed_single_allocator{};
}
