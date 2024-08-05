#include "box.h"
#include "composed_allocator.h"
#include "fixed_single_allocator.h"

namespace stdsharp
{
    template<std::size_t Size = default_soo_size, allocator_req Allocator = std::allocator<byte>>
    using soo_allocator = composed_allocator<fixed_single_allocator<byte, Size>, Allocator>;

    inline constexpr struct make_soo_allocator_fn
    {
        template<std::size_t Size, typename Allocator = std::allocator<byte>>
        constexpr soo_allocator<Size, std::decay_t<Allocator>> operator()(
            fixed_single_resource<Size>& buffer,
            Allocator&& alloc = Allocator{}
        ) const noexcept
        {
            return {buffer, cpp_forward(alloc)};
        }
    } make_soo_allocator{};

    template<std::size_t Size = default_soo_size, allocator_req Allocator = std::allocator<byte>>
    using trivial_soo_box = trivial_box<soo_allocator<Size, Allocator>>;

    template<std::size_t Size = default_soo_size, allocator_req Allocator = std::allocator<byte>>
    using normal_soo_box = normal_box<soo_allocator<Size, Allocator>>;

    template<std::size_t Size = default_soo_size, allocator_req Allocator = std::allocator<byte>>
    using unique_soo_box = unique_box<soo_allocator<Size, Allocator>>;

    template<
        typename T,
        std::size_t Size = default_soo_size,
        allocator_req Allocator = std::allocator<byte>>
    using soo_box_for = box_for<T, soo_allocator<Size, Allocator>>;
}
