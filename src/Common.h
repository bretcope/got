#ifndef MOT_COMMON_H
#define MOT_COMMON_H

#include <memory>

namespace mot
{
    template <typename T>
    using UP = std::unique_ptr<T>;

    template <typename T>
    using SP = std::shared_ptr<T>;

    template <typename T>
    using WP = std::weak_ptr<T>;
}

#endif //MOT_COMMON_H
