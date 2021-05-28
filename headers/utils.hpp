//
// Created by esteb on 28-May-21.
//

#ifndef MEMORYMAP_UTILS_HPP
#define MEMORYMAP_UTILS_HPP

#include "fixed_string.hpp"

template<class Record, bool V = std::is_fundamental_v < Record>>
auto get_key (const Record& r)
{
    if constexpr (V) return r;
    else return r.key();
}

[[nodiscard]]
auto random_name ()
{
    static constexpr char charset[] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    auto name = fixed_string<8>();
    for (int i = 0; i < 7; ++i)
        name[i] = charset[(unsigned) rand() % (sizeof(charset) - 1)];

    return name;
}



#endif //MEMORYMAP_UTILS_HPP
