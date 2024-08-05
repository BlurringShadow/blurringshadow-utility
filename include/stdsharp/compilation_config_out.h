#if defined(__GNUG__) || defined(_MSC_VER)
    #pragma pop_macro("STDSHARP_ALWAYS_INLINE")
    #pragma pop_macro("STDSHARP_NO_UNIQUE_ADDRESS")
    #pragma pop_macro("STDSHARP_INTRINSIC")
    #pragma pop_macro("STDSHARP_EBO")
#else
    #undef STDSHARP_NO_UNIQUE_ADDRESS
    #undef STDSHARP_INTRINSIC
    #undef STDSHARP_ALWAYS_INLINE
    #undef STDSHARP_EBO
#endif
