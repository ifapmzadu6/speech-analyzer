#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/LU>

#include "lorenz_system.h"
#include "gnuplot.h"


class SanoSawadaMethod {
    public:

        std::vector<double> LyapunovExponent(const std::vector<std::vector<double>> &x) {
            Eigen::MatrixXd ex = Eigen::MatrixXd::Zero(x.size(), x[0].size());
            for (int i=0; i<x.size(); i++) for (int j=0; j<x[0].size(); j++) ex(i, j) = x[i][j];

            int n = ex.rows();
            std::vector<double> l(n, 0);
            int ei = 300;
            for (int k=0; k<ei; k++) {
                Eigen::MatrixXd j = jacobi(ex, k);
                Eigen::MatrixXd g = gramShumidt(j);
                for (int i=0; i<n; i++) {
                    l[i] += log(g.row(i).squaredNorm()) / ei;
                }
            }
            return l;
        }

        void Test() {

            // check LyapunovExponent
            std::vector<double> x, y, z;
            LorenzSystem ls;
            ls.r = 25.6;
            for (double i = 0; i < 100000; i++) {
                ls.nextTime();
                x.push_back(ls.x);
                y.push_back(ls.y);
                z.push_back(ls.z);
            }
            std::vector<std::vector<double>> lo{x, y, z};

            std::vector<double> l = LyapunovExponent(lo);
            for (int i=0; i<l.size(); i++) {
                std::cout << l[i] << std::endl;
            }
        }




    private:
        Eigen::MatrixXd jacobi(Eigen::MatrixXd &x, int index, double eps = 10, int m = 20, int s = 1) {
            std::vector<int> ri = randomIndexes(x.cols());

            Eigen::VectorXd vn = x.col(index);
            Eigen::VectorXd vns = x.col(index + s);

            // find cols inside eps
            std::vector<Eigen::VectorXd> y, z;
            for (int t=0; t<ri.size() && y.size() < m; t++) {
                int i = ri[t];
                if (i == index) continue;
                Eigen::VectorXd d = vn - x.col(i);
                if (d.squaredNorm() < eps) {
                    y.push_back(d);
                    z.push_back(vns - x.col(i + s));
                }
            }
            if (y.size() != m) {
                std::cout << "can't find one inside eps: " << y.size() << std::endl;
                abort();
            }

            int d = x.rows();
            Eigen::MatrixXd v = Eigen::MatrixXd::Zero(d, d);
            Eigen::MatrixXd c = Eigen::MatrixXd::Zero(d, d);
            for (int i = 0; i < d; i++) {
                for (int j = 0; j < d; j++) {
                    for (int k = 0; k < m; k++) {
                        v(i, j) += y[k][i] * y[k][j] / m;
                        c(i, j) += z[k][i] * y[k][j] / m;
                    }
                }
            }
            return c * v.inverse();
        }

        Eigen::MatrixXd gramShumidt(Eigen::MatrixXd &y) {
            Eigen::MatrixXd u = Eigen::MatrixXd::Zero(y.cols(), y.rows());
            u.col(0) = y.col(0);
            for (int i=1; i<y.cols(); i++) {
                Eigen::VectorXd v = y.col(i);
                Eigen::VectorXd ub = u.col(i-1);
                u.col(i) = v - (v.dot(ub) / ub.squaredNorm()) * ub;
            }
            return u;
        }

        std::vector<int> randomIndexes(int n) {
            std::vector<int> t;
            for (int i=0; i<n; i++) t.push_back(i);
            std::random_device rnd;
            std::mt19937 mt(rnd());

            std::vector<int> r;
            while (t.size() > 0) {
                int i = mt() % t.size();
                r.push_back(t[i]);
                t.erase(t.begin() + i);
            }
            return r;
        }

};

