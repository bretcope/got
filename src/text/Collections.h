#ifndef MOT_COLLECTIONS_H
#define MOT_COLLECTIONS_H

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include "MotString.h"

namespace mot
{
    struct StringHasher
    {
        size_t operator()(const MotString& s) const
        {
            return s.HashCode();
        }
    };

    struct StringPtrHasher
    {
        size_t operator()(const MotString* s) const
        {
            return s == nullptr ? 0 : s->HashCode();
        }
    };

    struct StringCaseInsensitiveComparer
    {
        bool operator()(const MotString& a, const MotString& b) const
        {
            return MotString::AreCaseInsensitiveEqual(&a, &b);
        }
    };

    struct StringPtrCaseInsensitiveComparer
    {
        bool operator()(const MotString* a, const MotString* b) const
        {
            return MotString::AreCaseInsensitiveEqual(a, b);
        }
    };

    template <typename T>
    using ByString = std::unordered_map<MotString, T, StringPtrHasher, StringPtrCaseInsensitiveComparer>;

    template <typename T>
    using ByStringPtr = std::unordered_map<const MotString*, T, StringPtrHasher, StringPtrCaseInsensitiveComparer>;

    using StringSet = std::unordered_set<MotString, StringPtrHasher, StringPtrCaseInsensitiveComparer>;
    using StringPtrSet = std::unordered_set<const MotString*, StringPtrHasher, StringPtrCaseInsensitiveComparer>;
}

#endif //MOT_COLLECTIONS_H
