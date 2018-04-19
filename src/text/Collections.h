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

    struct StringCaseInsensitiveComparer
    {
        bool operator()(const MotString& a, const MotString& b) const
        {
            return MotString::AreCaseInsensitiveEqual(a, b);
        }
    };

//    template <typename T>
//    using ByString = std::unordered_map<MotString, T, StringHasher, StringCaseInsensitiveComparer>;

    using StringSet = std::unordered_set<MotString, StringHasher, StringCaseInsensitiveComparer>;
}

#endif //MOT_COLLECTIONS_H
