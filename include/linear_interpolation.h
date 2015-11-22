#include <iostream>
#include <vector>

#ifndef linear_interpolation_h
#define linear_interpolation_h

class LinearInterpolation {
public:
    static std::vector<double> convert(std::vector<double>& input, int toCycle);
};

#endif
