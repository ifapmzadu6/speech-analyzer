#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

template <typename T>
class FindPeaks {
   public:
    static std::vector<int> finds(std::vector<T> &input);

    static void tests();
};
