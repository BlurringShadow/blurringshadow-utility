#if defined(__GNUG__) || defined(_MSC_VER)
    #pragma push_macro("STDSHARP_ALWAYS_INLINE")
    #pragma push_macro("STDSHARP_NO_UNIQUE_ADDRESS")
    #pragma push_macro("STDSHARP_INTRINSIC")
    #pragma push_macro("STDSHARP_EBO")
#endif

#if defined(__GNUG__) && __has_cpp_attribute(gnu::always_inline)
    #define STDSHARP_ALWAYS_INLINE [[gnu::always_inline]]
#elif defined(_MSC_VER) && __has_cpp_attribute(msvc::forceinline)
    #define STDSHARP_ALWAYS_INLINE [[msvc::forceinline]]
#else
    #define STDSHARP_ALWAYS_INLINE inline
#endif

#if defined(_MSC_VER) && __has_cpp_attribute(msvc::no_unique_address) && \
    __has_cpp_attribute(msvc::intrinsic)
    #define STDSHARP_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
    #define STDSHARP_INTRINSIC [[msvc::intrinsic]] [[nodiscard]]
#else
    #define STDSHARP_NO_UNIQUE_ADDRESS [[no_unique_address]]
    #define STDSHARP_INTRINSIC [[nodiscard]] STDSHARP_ALWAYS_INLINE
#endif

#if defined(_MSC_VER) || (defined(_WIN32) && defined(__clang__))
    #define STDSHARP_EBO __declspec(empty_bases)
#else
    #define STDSHARP_EBO
#endif
