#pragma once

#include "concepts/object.h"
#include "mutex/mutex.h"
#include "utility/forward_cast.h"

#include <shared_mutex>

namespace stdsharp
{
    template<shared_lockable Lockable = std::shared_mutex>
        requires basic_lockable<Lockable>
    class synchronizer
    {
    public:
        using lock_type = Lockable;
        using shared_lock = std::shared_lock<lock_type>;
        using unique_lock = std::unique_lock<lock_type>;

        synchronizer() = default;

        template<typename... Args>
            requires std::constructible_from<Lockable, Args...>
        explicit(sizeof...(Args) == 1) constexpr synchronizer(Args&&... args)
            noexcept(nothrow_constructible_from<Lockable, Args...>):
            lockable_(cpp_forward(args)...)
        {
        }

        synchronizer(const synchronizer&) = delete;

        synchronizer(synchronizer&&) = delete;

        synchronizer& operator=(const synchronizer&) = delete;

        synchronizer& operator=(synchronizer&&) = delete;

        ~synchronizer() = default;

    private:
        template<typename Lock, typename... Args, typename T, typename Self>
            requires std::constructible_from<Lock, lock_type&, Args...>
        constexpr auto write_impl(this Self&& self, T&& value, Args&&... args)
        {
            using cast_t = forward_cast_t<Self, T>;

            struct local
            {
                cast_t value;
                Lock lock;
            };

            return local{
                .value = static_cast<cast_t>(value),
                .lock =
                    Lock{forward_cast<Self&, synchronizer>(self).lockable_, cpp_forward(args)...}
            };
        }

    public:
#define STDSHARP_SYNCHRONIZER_READ_WITH(ref)                                             \
    template<typename Self, typename SelfT = const Self ref>                             \
    [[nodiscard]] constexpr auto read_with(this const Self ref self, auto&&... args)     \
        requires requires {                                                              \
            forward_cast<SelfT, synchronizer>(self).template write_impl<shared_lock>(    \
                cpp_forward(args)...                                                     \
            );                                                                           \
        }                                                                                \
    {                                                                                    \
        return forward_cast<SelfT, synchronizer>(self).template write_impl<shared_lock>( \
            cpp_forward(args)...                                                         \
        );                                                                               \
    }

        STDSHARP_SYNCHRONIZER_READ_WITH(&)
        STDSHARP_SYNCHRONIZER_READ_WITH(&&)

#undef STDSHARP_SYNCHRONIZER_READ_WITH

        template<typename Self, typename SelfT = const Self&>
        [[nodiscard]] constexpr auto write_with(this Self&& self, auto&&... args)
            requires requires {
                forward_cast<Self, synchronizer>(self).template write_impl<unique_lock>(
                    cpp_forward(args)...
                );
            }
        {
            return forward_cast<SelfT, synchronizer>(self).template write_impl<unique_lock>(
                cpp_forward(args)...
            );
        }

        constexpr const lock_type& lockable() const noexcept { return lockable_; }

    protected:
        mutable lock_type lockable_{};
    };

    template<typename T>
    synchronizer(T&&) -> synchronizer<std::decay_t<T>>;
}
