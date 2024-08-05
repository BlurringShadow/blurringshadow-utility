#pragma once

#include "../memory/allocator_traits.h"
#include "../type_traits/indexed_traits.h"

#include <array> // IWYU pragma: export
#include <deque> // IWYU pragma: export
#include <forward_list>
#include <list> // IWYU pragma: export
#include <map> // IWYU pragma: export
#include <set> // IWYU pragma: export
#include <unordered_map> // IWYU pragma: export
#include <unordered_set> // IWYU pragma: export
#include <vector> // IWYU pragma: export

namespace stdsharp
{
    template<typename T, auto Size>
    struct allocator_of<std::array<T, Size>>
    {
        using type = std::allocator<T>;
    };

    template<typename Rng, typename ValueType>
    concept compatible_range = std::ranges::input_range<Rng> &&
        std::convertible_to<std::ranges::range_reference_t<Rng>, ValueType>;

    template<typename ValueType, typename Allocator>
    concept erasable = requires(Allocator allocator_instance, ValueType* ptr) {
        requires allocator_req<Allocator>;
        requires std::same_as<
            Allocator,
            typename allocator_traits<Allocator>:: //
            template rebind_alloc<ValueType>>;
        requires std::destructible<ValueType>;
        allocator_traits<Allocator>::destroy(allocator_instance, ptr);
    };

    template<typename ValueType, typename Allocator>
    concept nothrow_erasable = requires(Allocator allocator_instance, ValueType* ptr) {
        requires erasable<ValueType, Allocator>;
        requires noexcept(allocator_traits<Allocator>::destroy(allocator_instance, ptr));
    };

    template<typename Container>
    concept container_erasable = requires {
        requires erasable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename Container>
    concept container_nothrow_erasable = requires {
        requires nothrow_erasable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename ValueType, typename Allocator>
    concept move_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv) {
            requires std::same_as<
                Allocator,
                typename allocator_traits<Allocator>::template rebind_alloc<ValueType>>;
            requires std::move_constructible<ValueType>;
            allocator_traits<Allocator>::construct(allocator_instance, ptr, cpp_move(rv));
        };

    template<typename ValueType, typename Allocator>
    concept nothrow_move_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv) {
            requires move_insertable<ValueType, Allocator>;
            requires nothrow_move_constructible<ValueType>;
            requires noexcept(allocator_traits<Allocator>::
                                  construct(allocator_instance, ptr, cpp_move(rv)));
        };

    template<typename Container>
    concept container_move_insertable = requires {
        requires move_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename Container>
    concept container_nothrow_move_insertable = requires {
        requires nothrow_move_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename ValueType, typename Allocator>
    concept copy_insertable = requires(Allocator allocator_instance, ValueType* ptr, ValueType v) {
        requires move_insertable<ValueType, Allocator> && std::copy_constructible<ValueType>;
        allocator_traits<Allocator>::construct(allocator_instance, ptr, v);
    };

    template<typename ValueType, typename Allocator>
    concept nothrow_copy_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType v) {
            requires nothrow_move_insertable<ValueType, Allocator>;
            requires nothrow_copy_constructible<ValueType>;
            requires noexcept(allocator_traits<Allocator>::construct(allocator_instance, ptr, v));
        };

    template<typename Container>
    concept container_copy_insertable = requires {
        requires copy_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename Container>
    concept container_nothrow_copy_insertable = requires {
        requires nothrow_copy_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename ValueType, typename Allocator, typename... Args>
    concept emplace_constructible =
        requires(Allocator allocator_instance, ValueType* ptr, Args&&... args) {
            requires std::same_as<
                Allocator,
                typename allocator_traits<Allocator>::template rebind_alloc<ValueType>>;
            requires std::constructible_from<ValueType, Args...>;
            allocator_traits<Allocator>::construct(allocator_instance, ptr, cpp_forward(args)...);
        };

    template<typename ValueType, typename Allocator, typename... Args>
    concept nothrow_emplace_constructible =
        requires(Allocator allocator_instance, ValueType* ptr, Args&&... args) {
            requires nothrow_constructible_from<ValueType, Args...>;
            requires noexcept( //
                allocator_traits<Allocator>::
                    construct(allocator_instance, ptr, cpp_forward(args)...)
            );
        };

    template<typename Container, typename... Args>
    concept container_emplace_constructible = requires {
        requires emplace_constructible<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>,
            Args...>;
    };

    template<typename Container, typename... Args>
    concept container_nothrow_emplace_constructible = requires {
        requires nothrow_emplace_constructible<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>,
            Args...>;
    };
}

namespace stdsharp::details
{
    template<typename... T, std::size_t... I>
    static consteval inherited<indexed_value<I, T>...>
        make_container_members(std::index_sequence<I...>);

    template<typename... T>
    using container_members =
        decltype(make_container_members<T...>(std::index_sequence_for<T...>{}));

    template<
        typename T,
        typename U,
        typename TTraits = std::iterator_traits<T>,
        typename UTraits = std::iterator_traits<U>>
    concept iterator_identical =
        std::same_as<typename TTraits::value_type, typename UTraits::value_type> &&
        std::same_as<typename TTraits::difference_type, typename UTraits::difference_type> &&
        std::same_as<typename TTraits::pointer, typename UTraits::pointer> &&
        std::same_as<typename TTraits::reference, typename UTraits::reference>;

    template<
        typename ContainerType,
        typename Iter = ContainerType::iterator,
        typename ValueType = ContainerType::value_type>
    concept container_insertable = requires(
        ContainerType instance,
        ValueType value,
        const ValueType& const_value,
        ContainerType::const_iterator const_iter
    ) //
    {
        requires !container_move_insertable<ContainerType> || requires {
            { instance.insert(const_iter, cpp_move(value)) } -> std::same_as<Iter>;

            requires !container_copy_insertable<ContainerType> || requires {
                { instance.insert(const_iter, const_value) } -> std::same_as<Iter>;
            };
        };
    };

    template<
        typename Container,
        typename Members,
        typename ValueType = Container::value_type,
        typename Allocator = allocator_of_t<Container>,
        typename Traits = allocator_traits<Allocator>,
        typename Diff = Container::difference_type,
        typename Size = Container::size_type,
        typename Iter = Container::iterator,
        typename CIter = Container::const_iterator>
    concept container = requires(Container instance, const Container const_instance) {
        requires std::forward_iterator<Iter>;
        requires std::convertible_to<Iter, CIter>;
        requires std::forward_iterator<CIter>;

        requires std::signed_integral<Diff>;
        requires all_same<
            Diff,
            typename std::iterator_traits<Iter>::difference_type,
            typename std::iterator_traits<CIter>::difference_type>;

        requires std::unsigned_integral<Size>;
        requires(std::numeric_limits<Size>::max() >= std::numeric_limits<Diff>::max());

        requires !nothrow_default_initializable<Members> || noexcept(Container()) ||
            !std::default_initializable<Members> || requires { Container(); };

        requires !(std::move_constructible<Members> && container_move_insertable<Container>) ||
            requires { Container(cpp_move(instance)); };

        requires !(std::copy_constructible<Members> && container_copy_insertable<Container>) ||
            requires { Container(instance); };

        requires !(Traits::propagate_on_move_v ||
                   move_assignable<Members> && move_assignable<ValueType> &&
                       container_move_insertable<Container>) ||
            requires { instance = cpp_move(instance); };

        requires !(copy_assignable<Members> && copy_assignable<ValueType> &&
                   container_copy_insertable<Container>) ||
            requires { instance = instance; };

        requires container_erasable<Container> && std::destructible<Container>;

        { instance.begin() } -> std::same_as<Iter>;
        { instance.end() } -> std::same_as<Iter>;

        { const_instance.begin() } -> std::same_as<CIter>;
        { const_instance.end() } -> std::same_as<CIter>;
        { const_instance.cbegin() } -> std::same_as<CIter>;
        { const_instance.cend() } -> std::same_as<CIter>;

        requires !std::equality_comparable<ValueType> || std::equality_comparable<Container>;

        { instance.swap(instance) } -> std::same_as<void>;
        { swap(instance, instance) } -> std::same_as<void>;

        requires requires { []<typename T, typename U>(std::forward_list<T, U>*) {}(&instance); } ||
            requires {
                { const_instance.size() } -> std::same_as<Size>;
            };

        { const_instance.max_size() } -> std::same_as<Size>;

        { const_instance.empty() } -> std::convertible_to<bool>;
    };
}

namespace stdsharp
{
    template<typename Container, typename... Members>
    concept container = details::container<Container, details::container_members<Members...>>;
}

namespace stdsharp::details
{
    template<
        typename Container,
        typename RIter = Container::reverse_iterator,
        typename CRIter = Container::const_reverse_iterator>
    concept reversible_container = requires(Container instance, const Container const_instance) {
        { instance.rbegin() } -> std::same_as<RIter>;
        { instance.rend() } -> std::same_as<RIter>;

        { const_instance.rbegin() } -> std::same_as<CRIter>;
        { const_instance.rend() } -> std::same_as<CRIter>;

        { instance.crbegin() } -> std::same_as<CRIter>;
        { instance.crend() } -> std::same_as<CRIter>;
    };

    template<typename Container, typename Members, typename Alloc = Container::allocator_type>
    concept allocator_aware_container = requires(Container instance, Alloc alloc) {
        requires allocator_req<Alloc>;

        requires details::container<Container, container_members<Members, Alloc>>;

        Container(alloc);

        requires !container_copy_insertable<Container> || requires { Container(instance, alloc); };

        requires !container_move_insertable<Container> ||
            requires { Container(cpp_move(instance), alloc); };

        requires std::same_as<typename Container::value_type, typename Alloc::value_type>;

        { instance.get_allocator() } -> std::same_as<Alloc>;
    };

    template<
        typename Container,
        typename ValueType = Container::value_type,
        typename Iter = Container::iterator,
        typename CIter = Container::const_iterator,
        typename IL = std::initializer_list<ValueType>,
        typename Size = Container::size_type>
    concept sequence_container = requires(
        Container instance,
        const Container const_instance,
        const ValueType const_value,
        CIter const_iter,
        Size size,
        IL il
    ) {
        requires !container_copy_insertable<Container> ||
            std::constructible_from<Container, Size, ValueType>;

        requires !container_emplace_constructible<Container, ValueType> || requires {
            requires std::constructible_from<Container, CIter, CIter>;
            requires std::constructible_from<Container, IL>;
            { instance.insert(const_iter, il) } -> std::same_as<Iter>;
            { instance.assign(il) } -> std::same_as<void>;

            { instance.emplace(const_iter, const_value) } -> std::same_as<Iter>;

            { instance.insert(const_iter, const_iter, const_iter) } -> std::same_as<Iter>;
            { instance.assign(const_iter, const_iter) } -> std::same_as<void>;

            { instance.erase(const_iter) } -> std::same_as<Iter>;
            { instance.erase(const_iter, const_iter) } -> std::same_as<Iter>;

            { instance.clear() } -> std::same_as<void>;
        };

        requires container_insertable<Container>;

        requires !(container_copy_insertable<Container> && copy_assignable<ValueType>) ||
            std::assignable_from<Container&, IL> && requires {
                { instance.insert(const_iter, size, const_value) } -> std::same_as<Iter>;

                { instance.assign(size, const_value) } -> std::same_as<void>;
            };

        { instance.front() } -> std::same_as<typename Container::reference>;
        { const_instance.front() } -> std::same_as<typename Container::const_reference>;
    };
}

namespace stdsharp
{
    template<typename Container, typename... Members>
    concept reversible_container = container<Container, Members...> &&
        details::reversible_container<Container>;

    template<typename Container, typename... Members>
    concept allocator_aware_container =
        details::allocator_aware_container<Container, details::container_members<Members...>>;

    template<typename Container, typename... Members>
    concept sequence_container = container<Container, Members...> &&
        details::sequence_container<Container>;

    template<typename Container>
    concept contiguous_container = container<Container> && std::ranges::contiguous_range<Container>;
}

namespace stdsharp::details
{
    template<typename Container, typename... Args>
    struct container_optional_constructible
    {
        using size_t = std::size_t;

        template<typename... Optional>
        static constexpr bool value =
            []<size_t... I>(this const auto self, const std::index_sequence<I...>) consteval
        {
            constexpr auto count = sizeof...(I);
            auto res = std::constructible_from<Container, Args..., type_at<I, Optional...>...>;

            if constexpr(count > 0) res = res && self(std::make_index_sequence<count - 1>{});

            return res;
        }(std::make_index_sequence<sizeof...(Optional)>{});
    };

    template<
        typename Container,
        typename Members,
        typename ValueType = Container::value_type,
        typename Iter = Container::iterator,
        typename CIter = Container::const_iterator,
        typename Size = Container::size_type,
        typename Node = Container::node_type>
    concept associative_like_container = requires(
        Container instance,
        const Container& const_instance,
        Container::key_type key,
        Container::const_reference const_ref,
        Iter iter,
        CIter const_iter,
        Node node,
        std::initializer_list<ValueType> il
    ) {
        requires stdsharp::allocator_aware_container<Container, Members>;

        requires container_insertable<Container>;

        requires !container_emplace_constructible<Container, ValueType> || requires {
            { instance.emplace_hint(const_iter, const_ref) } -> std::same_as<Iter>;
            { instance.insert(const_iter, const_iter) } -> std::same_as<void>;
            { instance.insert(il) } -> std::same_as<void>;
        };

        { instance.extract(const_iter) } -> std::same_as<Node>;
        { instance.extract(key) } -> std::same_as<Node>;

        { instance.erase(key) } -> std::same_as<Size>;

        { instance.find(key) } -> std::same_as<Iter>;
        { const_instance.find(key) } -> std::same_as<CIter>;

        { const_instance.count(key) } -> std::same_as<Size>;

        { const_instance.contains(key) } -> std::same_as<bool>;

        { instance.equal_range(key) } -> std::same_as<std::pair<Iter, Iter>>;
        { const_instance.equal_range(key) } -> std::same_as<std::pair<CIter, CIter>>;

        { instance.insert(const_iter, cpp_move(node)) } -> std::same_as<Iter>;

        { instance.erase(const_iter) } -> std::same_as<Iter>;
        { instance.erase(const_iter, const_iter) } -> std::same_as<Iter>;
        { instance.erase(iter) } -> std::same_as<Iter>;

        { instance.merge(instance) } -> std::same_as<void>;

        { instance.clear() } -> std::same_as<void>;
    };

    template<
        typename Container,
        typename Members,
        typename Iter = Container::iterator,
        typename CIter = Container::const_iterator,
        typename Alloc = Container::allocator_type,
        typename KeyType = Container::key_type,
        typename ValueType = Container::value_type,
        typename KeyCmp = Container::key_compare,
        typename ValueCmp = Container::value_compare,
        typename IL = std::initializer_list<ValueType>>
    concept associative_container = requires(
        Container instance,
        const Container const_instance,
        Container::key_type key //
    ) {
        requires associative_like_container<
            Container,
            container_members<KeyCmp, ValueCmp, Members>>;

        requires std::copyable<KeyCmp>;
        requires std::predicate<KeyCmp, KeyType, KeyType>;
        requires std::constructible_from<Container, const KeyCmp>;

        requires std::copyable<ValueCmp>;
        requires std::predicate<ValueCmp, ValueType, ValueType>;

        requires !container_emplace_constructible<Container, ValueType> ||
            details::container_optional_constructible<Container, CIter, CIter>::
                    template value<KeyCmp, Alloc> &&
                details::container_optional_constructible<Container, IL>::
                    template value<KeyCmp, Alloc>;

        requires !compatible_range<IL, ValueType> ||
            details::container_optional_constructible<Container, std::from_range_t, IL>::
                template value<KeyCmp, Alloc>;

        { instance.key_comp() } -> std::same_as<KeyCmp>;
        { instance.value_comp() } -> std::same_as<ValueCmp>;

        { instance.lower_bound(key) } -> std::same_as<Iter>;
        { const_instance.lower_bound(key) } -> std::same_as<CIter>;
        { instance.upper_bound(key) } -> std::same_as<Iter>;
        { const_instance.upper_bound(key) } -> std::same_as<CIter>;
    };

    // template<class Iter, class NodeType>
    // struct /*unspecified*/
    // {
    //     Iter     position;
    //     bool     inserted;
    //     NodeType node;
    // };
    template<
        typename Container,
        typename InsertRet = Container::insert_return_type,
        typename Iter = typename Container::iterator,
        typename Node = Container::node_type>
    concept unique_associative = requires(
        Container instance,
        Node node,
        InsertRet insert_ret,
        Iter iter //
    ) {
        { instance.insert(cpp_move(node)) } -> std::same_as<InsertRet>;

        requires std::same_as<Iter, decltype(insert_ret.position)>;
        requires std::same_as<bool, decltype(insert_ret.inserted)>;
        requires std::same_as<Node, decltype(insert_ret.node)>;

        requires aggregate<InsertRet>;
        InsertRet{iter, true, cpp_move(node)};
    };

    template<typename Container>
    concept multikey_associative = requires(Container instance, Container::node_type node) {
        { instance.insert(cpp_move(node)) } -> std::same_as<typename Container::iterator>;
    };

    template<
        typename Container,
        typename Members,
        typename Iter = Container::iterator,
        typename CIter = Container::const_iterator,
        typename LocalIter = Container::local_iterator,
        typename LocalCIter = Container::const_local_iterator,
        typename Size = Container::size_type,
        typename IL = std::initializer_list<typename Container::value_type>,
        typename KeyType = Container::key_type,
        typename ValueType = Container::value_type,
        typename Alloc = Container::allocator_type,
        typename KeyEqual = Container::key_equal,
        typename Hasher = Container::hasher>
    concept unordered_associative_container = requires(
        Container instance,
        const Container const_instance,
        KeyType key,
        Size size //
    ) {
        requires associative_like_container<Container, container_members<Members, Hasher>>;

        requires std::copyable<KeyEqual>;
        requires std::predicate<KeyEqual, KeyType, KeyType>;

        requires std::copyable<Hasher>;
        requires invocable_r<Hasher, std::size_t, KeyType>;

        requires details::iterator_identical<Iter, LocalIter>;
        requires details::iterator_identical<CIter, LocalCIter>;

        requires details::container_optional_constructible<Container, Size>::
            template value<Hasher, KeyEqual, Alloc>;

        requires !container_emplace_constructible<Container, ValueType> || requires {
            requires details::container_optional_constructible<Container, CIter, CIter>::
                template value<Size, Hasher, KeyEqual, Alloc>;

            requires details::container_optional_constructible<
                Container,
                IL>:: //
                template value<Size, Hasher, KeyEqual, Alloc>;

            requires !compatible_range<IL, ValueType> ||
                details::container_optional_constructible<
                    Container,
                    std::from_range_t,
                    IL>:: //
                template value<Size, Hasher, KeyEqual, Alloc>;
        };

        requires !container_copy_insertable<Container> && copy_assignable<ValueType> &&
                std::default_initializable<Hasher> && std::default_initializable<KeyEqual> &&
                std::default_initializable<Alloc> || std::constructible_from<Container, IL>;

        { instance.key_eq() } -> std::same_as<KeyEqual>;
        { instance.hash_function() } -> std::same_as<Hasher>;

        { const_instance.bucket(key) } -> std::same_as<Size>;
        { const_instance.bucket_count() } -> std::same_as<Size>;
        { const_instance.max_bucket_count() } -> std::same_as<Size>;

        { instance.max_load_factor(0.0f) } -> std::same_as<void>;

        { const_instance.load_factor() } -> std::same_as<float>;
        { const_instance.max_load_factor() } -> std::same_as<float>;

        { const_instance.bucket_size(size) } -> std::same_as<Size>;

        { instance.rehash(size) } -> std::same_as<void>;

        { instance.reserve(size) } -> std::same_as<void>;

        { instance.begin(size) } -> std::same_as<LocalIter>;
        { instance.end(size) } -> std::same_as<LocalIter>;

        { const_instance.begin(size) } -> std::same_as<LocalCIter>;
        { const_instance.end(size) } -> std::same_as<LocalCIter>;
        { const_instance.cbegin(size) } -> std::same_as<LocalCIter>;
        { const_instance.cend(size) } -> std::same_as<LocalCIter>;
    };
}

namespace stdsharp
{
    template<typename Container, typename... Members>
    concept associative_like_container =
        details::associative_like_container<Container, details::container_members<Members...>>;

    template<typename Container, typename... Members>
    concept associative_container =
        details::associative_container<Container, details::container_members<Members...>>;

    template<typename Container, typename... Members>
    concept unique_associative_container = associative_container<Container, Members...> &&
        details::unique_associative<Container>;

    template<typename Container, typename... Members>
    concept multikey_associative_container = associative_container<Container, Members...> &&
        details::multikey_associative<Container>;

    template<typename Container, typename... Members>
    concept unordered_associative_container =
        details::unordered_associative_container<Container, details::container_members<Members...>>;

    template<typename Container, typename... Members>
    concept unique_unordered_associative_container =
        unordered_associative_container<Container, Members...> &&
        details::unique_associative<Container>;

    template<typename Container, typename... Members>
    concept multikey_unordered_associative_container =
        unordered_associative_container<Container, Members...> &&
        details::multikey_associative<Container>;

    template<typename Predicate, typename Container>
    concept container_predicate = std::predicate<Predicate, typename Container::const_reference>;
}
