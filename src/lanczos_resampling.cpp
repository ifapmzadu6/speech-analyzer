
#include "lanczos_resampling.h"

std::vector<double> LanczosResampling::convert(std::vector<double> &input, int toCycle) {
    int fromCycle = input.size();
    int filterSize = 64;
    std::vector<double> output;
    for (int i = 0; i < toCycle; i++) {
        double v = double(fromCycle) * i / toCycle;
        int x = int(v);
        // double a = v - x;

        double sum = 0;
        for (int j=x-filterSize+1; j<=x+filterSize; j++) {
            sum += input[(fromCycle + j) % fromCycle] * lanczos(v-j, filterSize);
        }
        output.push_back(sum);
    }
    return output;
}
