#pragma once

#include "../cmath/cmath.h"
#include "../concepts/object.h"
#include "../cstdint/cstdint.h"

#include <array>
#include <bitset>
#include <random>

namespace stdsharp::details
{
    template<
        typename S,
        typename ResultType = S::result_type,
        typename It = ResultType*,
        typename IL = std::initializer_list<ResultType>>
    concept seed_sequence_concept = requires(
        S q,
        const S r,
        IL il,
        It it //
    ) {
        requires std::unsigned_integral<ResultType>;

        S();
        S(it, it);
        S(il);

        { q.generate(it, it) } -> std::same_as<void>;

        { r.size() } -> std::same_as<std::size_t>;

        { r.param(it) } -> std::same_as<void>;
    };

    template<
        typename E,
        typename T = E::result_type,
        typename OS = std::basic_ostream<char>,
        typename IS = std::basic_istream<char>>
    concept random_number_engine_concept = requires(
        E e,
        E& v,
        const E x,
        T s,
        std::seed_seq& q,
        unsigned long long z,
        OS& os,
        IS& is
    ) {
        requires std::uniform_random_bit_generator<E>;

        E();
        E(x);
        E(s);
        E(q);

        { e.seed() } -> std::same_as<void>;
        { e.seed(s) } -> std::same_as<void>;
        { v.seed(q) } -> std::same_as<void>;

        { e() } -> std::same_as<T>;

        { e.discard(z) } -> std::same_as<void>;

        { x == v } -> std::same_as<bool>;
        { x != v } -> std::same_as<bool>;

        { os << x } -> std::same_as<OS&>;
        { is >> v } -> std::same_as<IS&>;
    };

    template<
        typename D,
        typename T = D::result_type,
        typename P = D::param_type,
        typename OS = std::basic_ostream<char>,
        typename IS = std::basic_istream<char>>
    concept random_number_distribution = requires(
        D d,
        const D x,
        P p,
        std::random_device& g,
        OS& os,
        IS& is //
    ) {
        requires arithmetic<T>;

        D();
        D(p);
        requires std::copy_constructible<D>;
        requires copy_assignable<D>;

        requires std::copy_constructible<P>;
        requires copy_assignable<P>;
        requires std::equality_comparable<P>;
        requires std::same_as<typename P::distribution_type, D>;

        { d.reset() } -> std::same_as<void>;

        { x.param() } -> std::same_as<P>;
        { d.param(p) } -> std::same_as<void>;

        { d(g) } -> std::same_as<T>;
        { d(g, p) } -> std::same_as<T>;

        { x.min() } -> std::same_as<T>;
        { x.max() } -> std::same_as<T>;

        { x == x } -> std::same_as<bool>;
        { x != x } -> std::same_as<bool>;

        { os << x } -> std::same_as<OS&>;
        { is >> d } -> std::same_as<IS&>;
    };
}

namespace stdsharp
{
    template<typename S>
    concept seed_sequence = details::seed_sequence_concept<S>;

    template<typename Engine>
    concept random_number_engine = details::random_number_engine_concept<Engine>;

    template<typename Distribution>
    concept random_number_distribution = details::random_number_distribution<Distribution>;

    inline constexpr struct
    {
        [[nodiscard]] std::random_device& operator()() const
        {
            thread_local std::random_device random_device;
            return random_device;
        }
    } get_random_device{};

    inline constexpr struct get_seed_seq_fn
    {
    private:
        static auto make_seq()
        {
            const auto num = get_random_device()();

            constexpr auto num_type_size = sizeof(num) * char_bit;
            constexpr auto seed_num_type_size = 32;

            if constexpr(seed_num_type_size >= num_type_size) return std::seed_seq{num};
            else
            {
                std::bitset<num_type_size> bits{num};
                std::array<std::uint_least32_t, ceil(num_type_size, seed_num_type_size)> seeds{};

                for(auto i = 0; i < num_type_size;)
                {
                    std::bitset<seed_num_type_size> seed_bits;

                    for(auto j = 0; j < seed_num_type_size && i < num_type_size; ++j, ++i)
                        seed_bits[j] = bits[i];

                    seeds[i] = static_cast<std::uint_least32_t>(seed_bits.to_ullong());

                    bits >>= seed_num_type_size;
                }

                return std::seed_seq{seeds.begin(), seeds.end()};
            }
        }

    public:
        [[nodiscard]] std::seed_seq& operator()() const
        {
            thread_local std::seed_seq seq{get_random_device()()};

            return seq;
        }
    } get_seed_seq{};

    template<random_number_engine Engine = std::default_random_engine>
        requires std::constructible_from<std::random_device::result_type>
    struct make_engine_fn
    {
        [[nodiscard]] auto operator()() const { return Engine{get_random_device()()}; }
    };

    template<typename Engine>
    inline constexpr make_engine_fn<Engine> make_engine{};

    template<typename Engine = std::default_random_engine>
        requires std::invocable<make_engine_fn<Engine>>
    struct get_engine_fn
    {
        [[nodiscard]] auto& operator()() const
        {
            thread_local auto engine = make_engine<Engine>();
            return engine;
        }
    };

    template<typename Engine = std::default_random_engine>
    inline constexpr get_engine_fn<Engine> get_engine{};

    inline constexpr struct make_uniform_distribution_fn
    {
    private:
        template<typename T>
        static constexpr auto min_value = std::numeric_limits<T>::min();

        template<typename T>
        static constexpr auto max_value = std::numeric_limits<T>::max();

    public:
        template<same_as_any<short, int, long, long long, ushort, unsigned, ulong, ull> T>
        [[nodiscard]] auto
            operator()(const T min = min_value<T>, decltype(min) max = max_value<T>) const
        {
            return std::uniform_int_distribution<T>{min, max};
        }

        template<same_as_any<float, double, long double> T>
        [[nodiscard]] auto
            operator()(const T min = min_value<T>, decltype(min) max = max_value<T>) const
        {
            return std::uniform_real_distribution<T>{min, max};
        }
    } make_uniform_distribution{};
}
