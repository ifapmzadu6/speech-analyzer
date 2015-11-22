
#include "kernel_density_estimation.h"



int KernelDensityEstimation::IndexOfMaxDensity(std::vector<std::vector<double>> &inputs) {
    if (inputs.size() == 0) {
        return -1;
    }

    int dim = inputs[0].size();

    int max = -1;
    double maxY = 0;
    for (int i=0; i<dim; i++) {

        for (int j=0; j<inputs.size(); j++) {

            double y = 0;
            for (int k=0; k<inputs.size(); k++) {
                y += gaussian(inputs[k][i] - inputs[j][i]);
            }
            if (maxY < y) {
                maxY = y;
                max = j;
            }

        }

    }

    return max;
}


double KernelDensityEstimation::gaussian(double x) {
    return exp(- pow(x, 2) / 2) / sqrt(2 * M_PI);
}


