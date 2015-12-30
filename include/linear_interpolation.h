#pragma once

#include <iostream>
#include <vector>
#include <cmath>

#include "gnuplot.h"


class LinearInterpolation {
   public:
    static std::vector<double> convert(std::vector<double> &input, int toCycle);

    static void tests() {
        std::vector<double> y;
        for (int i=0; i<290; i++) {
            double value = sin(2 * M_PI * i / 290);
            y.push_back(value);
        }
        y = convert(y, 340);

        Gnuplot<double>::Output(y, "test", "w l");
    }

};

