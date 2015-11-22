#include <iostream>
#include <vector>
#include <cmath>

#ifndef kernel_density_estimation_h
#define kernel_density_estimation_h

class KernelDensityEstimation {
public:
    static int IndexOfMaxDensity(std::vector<std::vector<double> >& inputs);

private:
    inline static double gaussian(double x);
};

#endif
