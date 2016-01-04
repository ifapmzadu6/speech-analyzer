//
//  RBF.cpp
//  FireflyProject
//
//  Created by Keisuke Karijuku on 2014/09/22.
//  Copyright (c) 2014年 Keisuke Karijuku. All rights reserved.
//

#include "RBF.h"

double RadiaBasisFunction::output(const double &spread, const std::vector<double> &centerVector, const std::vector<double> &x) { return exp(-spread * squeredNorm(centerVector, x)); }

double RadiaBasisFunction::meanSquaredError(const std::vector<std::vector<double>> &d, const std::vector<std::vector<double>> &o) {
    double mse = 0.0;
    auto ms_dIIter = d.begin();
    auto ms_dIIterEnd = d.end();
    auto ms_oIIter = o.begin();
    while (ms_dIIter != ms_dIIterEnd) {
        auto ms_dJIter = (*ms_dIIter).begin();
        auto ms_dJIterEnd = (*ms_dIIter).end();
        auto ms_oJIter = (*ms_oIIter).begin();
        while (ms_dJIter != ms_dJIterEnd) {
            double tmp = (*ms_dJIter - *ms_oJIter);
            mse += tmp * tmp;
            ++ms_dJIter;
            ++ms_oJIter;
        }
        ++ms_dIIter;
        ++ms_oIIter;
    }
    mse /= d.size();
    return mse;
}

double RadiaBasisFunction::squeredNorm(const std::vector<double> &a, const std::vector<double> &b) {
    double d = 0.0;
    auto no_xIter = a.begin();
    auto no_xIterEnd = a.end();
    auto no_yIter = b.begin();
    while (no_xIter != no_xIterEnd) {
        double tmp = (*no_xIter) - (*no_yIter);
        d += tmp * tmp;
        ++no_xIter;
        ++no_yIter;
    }
    return d;
}

void RadiaBasisFunction::mult(std::vector<std::vector<double>> &Y, const std::vector<std::vector<double>> &A, const std::vector<std::vector<double>> &B) {
    for (auto &vec : Y) {
        for (auto &value : vec) {
            value = 0.0;
        }
    }

    auto mu_oIIter = Y.begin();
    auto mu_oIterEnd = Y.end();
    auto mu_rIIter = A.begin();
    while (mu_oIIter != mu_oIterEnd) {
        auto mu_rKIter = (*mu_rIIter).begin();
        auto mu_rKIterEnd = (*mu_rIIter).end();
        auto mu_wKIter = B.begin();
        while (mu_rKIter != mu_rKIterEnd) {
            auto mu_oJIter = (*mu_oIIter).begin();
            auto mu_oJIterEnd = (*mu_oIIter).end();
            auto mu_wJIter = (*mu_wKIter).begin();
            while (mu_oJIter != mu_oJIterEnd) {
                (*mu_oJIter) += (*mu_rKIter) * (*mu_wJIter);
                ++mu_oJIter;
                ++mu_wJIter;
            }
            ++mu_rKIter;
            ++mu_wKIter;
        }
        ++mu_oIIter;
        ++mu_rIIter;
    }
}
