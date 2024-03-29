#include "test.h"
#include "stdsharp/synchronizer.h"

using namespace std;
using namespace stdsharp;

SCENARIO("synchronizer", "[synchronizer]") // NOLINT
{
    struct my_mutex
    {
        static constexpr void lock() {}

        static constexpr void unlock() {}

        static constexpr void lock_shared() {}

        static constexpr bool try_lock_shared() { return true; }

        static constexpr bool try_lock() { return true; }

        static constexpr void unlock_shared() {}

        my_mutex() = default;
        my_mutex(const my_mutex&) = delete;
        my_mutex(my_mutex&&) = delete;
        my_mutex& operator=(const my_mutex&) = delete;
        my_mutex& operator=(my_mutex&&) = delete;
        ~my_mutex() = default;
    };

    using synchronizer = synchronizer<my_mutex>;

    STATIC_REQUIRE(!movable<synchronizer>);
    STATIC_REQUIRE(default_initializable<synchronizer>);
    STATIC_REQUIRE(constructible_from<synchronizer>);

    synchronizer syn;
    int i{};
    auto&& [value, lock] = syn.read_with(i);
}