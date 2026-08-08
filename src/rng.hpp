#pragma once
#include <random>

inline int randInt(int lo, int hi)
{
    static std::mt19937 rng{std::random_device{}()};
    return std::uniform_int_distribution<int>{lo, hi}(rng);
}
