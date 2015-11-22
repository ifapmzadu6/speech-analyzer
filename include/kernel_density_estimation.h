#include <iostream>
#include <vector>
#include <cmath>


class KernelDensityEstimation {
public:

    static int IndexOfMaxDensity(std::vector<std::vector<double>> &inputs);

private:

    inline static double gaussian(double x);

};

