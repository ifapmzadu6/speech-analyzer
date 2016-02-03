#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include "gnuplot.h"

class LanczosResampling {
   public:
    static std::vector<double> convert(std::vector<double> &input, int toCycle) {
        int fromCycle = input.size();
        int filterSize = 64;
        std::vector<double> output;
        for (int i = 0; i < toCycle; i++) {
            double v = double(fromCycle) * i / toCycle;
            int x = int(v);
            // double a = v - x;

            double sum = 0;
            for (int j = x - filterSize + 1; j <= x + filterSize; j++) {
                sum += input[(fromCycle + j) % fromCycle] * lanczos(v - j, filterSize);
            }
            output.push_back(sum);
        }
        return output;
    }

    static double sinc(double x) {
        if (x == 0.0) {
            return (1.0);
        }
        double v = M_PI * x;
        return std::sin(v) / v;
    }

    static double lanczos(double x, int a) {
        if (-a < x && x < a) {
            return sinc(x) * sinc(x / a);
        }
        return 0.0;
    }

    static void tests() {
        std::vector<double> y;
        for (int i = 0; i < 290; i++) {
            double value = sin(10 * M_PI * i / 290);
            y.push_back(value);
        }
        y = convert(y, 3500);

        Gnuplot<double>::Output(y, "test", "w l");
    }
};
