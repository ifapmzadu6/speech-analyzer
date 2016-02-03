#pragma once

#include <iostream>
#include <vector>
#include <cmath>

class KernelDensityEstimation {
    public:
        static int IndexOfMaxDensity(std::vector<std::vector<double>> &inputs) {
            if (inputs.size() == 0) {
                return -1;
            }

            int maxValue = -1;
            double maxIndex = 0;
            for (int i = 0; i < inputs.size(); i++) {
                std::vector<double> input = inputs[i];

                double value = 0;
                for (int j = 0; j < input.size(); j++) {
                    for (int k = 0; k < inputs.size(); k++) {
                        if (k != i) {
                            value += gaussian(input[j] - inputs[k][j]);
                        }
                    }
                }

                if (maxValue < value) {
                    maxValue = value;
                    maxIndex = i;
                }
            }
            return maxIndex;
        }

    private:
        inline static double gaussian(double x) {
            double sigma = 0.2;
            return exp(-pow(x, 2) / 2 / pow(sigma, 2)) / sqrt(2 * M_PI) / sigma;
        }
};
