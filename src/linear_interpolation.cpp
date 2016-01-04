
#include "linear_interpolation.h"

std::vector<double> LinearInterpolation::convert(std::vector<double> &input, int toCycle) {
    int fromCycle = input.size();
    std::vector<double> output;
    for (int i = 0; i < toCycle; i++) {
        double v = double(fromCycle) * i / toCycle;
        int x = int(v);
        if (x < input.size() - 1) {
            double a = v - x;
            double value = (1.0 - a) * input[x] + a * input[x + 1];
            output.push_back(value);
        } else if (x == input.size() - 1) {
            double a = v - x;
            double value = (1.0 - a) * input[x];
            output.push_back(value);
        } else {
            output.push_back(0);
        }
    }
    return output;
}
