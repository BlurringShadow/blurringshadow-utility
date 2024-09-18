#pragma once

#include "stdsharp/tuple/get.h"
#include "type_traits/indexed_traits.h"
#include "type_traits/object.h"

#include <memory>

namespace stdsharp::details
{
    struct unique_resource_traits
    {
        template<typename T, nothrow_invocable<T> D>
        class impl : public unique_object // NOLINT(*-special-member-functions)
        {
        private:
            using indexed_values = indexed_values<T, D>;

            indexed_values compressed_;

            bool released_ = false;

            constexpr auto& get() noexcept { return get_element<0>(compressed_); }

            constexpr auto& get_deleter() noexcept { return get_element<1>(compressed_); }

        public:
            impl() = default;

            template<typename U, typename V>
                requires std::constructible_from<T, U> && std::constructible_from<D, V>
            constexpr impl(U&& resource, V&& deleter)
                noexcept(nothrow_constructible_from<T, U> && nothrow_constructible_from<D, V>):
                compressed_(cpp_forward(resource), cpp_forward(deleter))
            {
            }

            constexpr impl(impl&& other)
                noexcept(nothrow_move_constructible<T> && nothrow_move_constructible<D>)
                requires std::move_constructible<T> && std::move_constructible<D>
                : compressed_(cpp_move(other.resource_), cpp_move(other.deleter_))
            {
                other.released_ = true;
            }

            constexpr impl& operator=(impl&& other)
                noexcept(nothrow_move_assignable<T> && nothrow_move_assignable<D>)
            {
                compressed_ = cpp_move(other.compressed_);
                other.released_ = true;
            }

            constexpr ~impl() { release(); }

            constexpr auto& get() const noexcept { return get_element<0>(compressed_); }

            constexpr auto& get_deleter() const noexcept
            {
                return get_element<1>(compressed_);
            }

            constexpr void release() noexcept
            {
                if(released_) return;
                invoke(get_deleter(), get());
                released_ = true;
            }

            template<typename... Args>
            constexpr void emplace(Args&&... args)
                noexcept(nothrow_constructible_from<T, Args...> && nothrow_move_assignable<T>)
            {
                release();
                get() = T{cpp_forward(args)...};
                released_ = false;
            }

            constexpr void reset() noexcept { release(); }

            template<typename U = T>
            constexpr void reset(U&& r) noexcept(noexcept(emplace(cpp_forward(r))))
                requires requires { emplace(cpp_forward(r)); }
            {
                emplace(cpp_forward(r));
            }
        };

        template<nothrow_invocable D>
        class impl<void, D> : public unique_object // NOLINT(*-special-member-functions)
        {
        private:
            D deleter_;

            constexpr auto& get_deleter() noexcept { return deleter_; }

        public:
            impl() = default;

            template<typename U>
                requires std::constructible_from<D, U>
            constexpr impl(U&& deleter) noexcept(nothrow_constructible_from<D, U>):
                deleter_(cpp_forward(deleter))
            {
            }

            constexpr impl(impl&& other) noexcept(nothrow_move_constructible<D>)
                requires std::move_constructible<D>
                : deleter_(cpp_move(other.deleter_))
            {
                other.release();
            }

            constexpr impl& operator=(impl&& other) noexcept(nothrow_move_assignable<D>)
            {
                other.release();
                get_deleter() = cpp_move(other.get_deleter());
            }

            constexpr ~impl() { release(); }

            constexpr auto& get_deleter() const noexcept { return deleter_; }

            constexpr void release() noexcept { invoke(get_deleter(), get()); }
        };

        template<typename T, typename D>
        struct impl_traits
        {
            using type = impl<T, D>;
        };

        template<typename T, typename D>
            requires requires { std::unique_ptr<T, D>{}; }
        struct impl_traits<T, D>
        {
            using type = std::unique_ptr<T, D>;
        };

        template<typename T, typename D>
        using type = typename impl_traits<T, D>::type;
    };
}

namespace stdsharp
{
    template<typename T, typename D = std::default_delete<T>>
    using unique_resource = details::unique_resource_traits::type<T, D>;

    template<typename D>
    using resource_deleter = unique_resource<void, D>;
}
