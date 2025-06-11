#pragma once

#include <memory>

namespace slk
{

template <auto fn>
struct deleter_from_fn {
    template <typename T>
    constexpr void operator()(T* arg) const {
        fn(arg);
    }
};

template <typename T, auto fn>
using unique_ptr = std::unique_ptr<T, deleter_from_fn<fn>>;

}
