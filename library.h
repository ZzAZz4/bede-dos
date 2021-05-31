//
// Created by Jean Paul on 30/05/2021.
//

#ifndef MEMORYMAP_LIBRARY_H
#define MEMORYMAP_LIBRARY_H

#include <iostream>
#include <direct.h>
#include <cassert>
#include <sstream>
#include <string_view>
#include <string>
#include "HashIndex/hash_index.hpp"


struct wineQuality{
    int id;
    float fixed_acidity;
    float citric_acidity;
    float residual_sugar;
    float chlorides;
    float free_sulfur_dioxide;
    float total_sulfur_dioxide;
    float density;
    float ph;
    float sulphates;
    float alcohol;
    float quality;

    [[nodiscard]]
    const auto& key () const
    {
        return id;
    }
};

std::ostream& operator << (std::ostream& os, const wineQuality& r)
{
    os << r.id << ' '
       << r.fixed_acidity<<' '
       << r.citric_acidity<<' '
       << r.residual_sugar<<' '
       << r.chlorides<<' '
       << r.free_sulfur_dioxide<<' '
            ;
    return os;
}

#endif //MEMORYMAP_LIBRARY_H


