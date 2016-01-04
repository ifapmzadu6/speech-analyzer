#pragma once

#include <cmath>
#include <vector>

class RadiaBasisFunction {
   public:
    static double output(const double &spread, const std::vector<double> &centerVector, const std::vector<double> &x);
    static double meanSquaredError(const std::vector<std::vector<double>> &d, const std::vector<std::vector<double>> &o);

   private:
    static void mult(std::vector<std::vector<double>> &Y, const std::vector<std::vector<double>> &A, const std::vector<std::vector<double>> &B);
    static double squeredNorm(const std::vector<double> &a, const std::vector<double> &b);
};
