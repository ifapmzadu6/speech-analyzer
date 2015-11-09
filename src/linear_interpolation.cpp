
#include "linear_interpolation.h"


std::vector<double> LinearInterpolation::convert(std::vector<double> &input, int toCycle) {
    int fromCycle = input.size();
    double rate = double(fromCycle) / toCycle;
    std::vector<double> output;
    for (int i=0; i<toCycle; i++) {
        int x = (int)( i * rate );
        double a = ( i * rate ) - x;
        double value = ( 1 - a ) * input[x] + a * input[x+1];
        output.push_back(value);
    }
    return output;
}

