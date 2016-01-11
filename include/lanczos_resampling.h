#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include "gnuplot.h"

class LanczosResampling {
   public:

    static std::vector<double> convert(std::vector<double> &input, int toCycle);

    static double sinc(double x);

    static double lanczos(double x, int a);

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

