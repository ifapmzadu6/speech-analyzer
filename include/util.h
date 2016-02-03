#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

#include "julius_importer.h"
#include "linear_interpolation.h"
#include "wave.h"

class Util {
    public:
        static std::vector<double> GetInput(std::string path, int padding) {
            std::cout << "- \"" + path + "\" -" << std::endl;
            Wave wav;
            if (wav.InputWave(path) != 0) {
                abort();
            }
            wav.Normalize();

            std::vector<double> tmp;
            wav.GetData(&tmp);

            std::vector<double> input;
            for (int i = padding; i < tmp.size() - padding * 2; i++) {
                input.push_back(tmp[i]);
            }
            // std::cout << std::endl << "input.size() -> " << input.size() << std::endl << std::endl;
            return input;
        }

        static std::vector<double> NormalizeVector(std::vector<double> input) {
            double max = 0;
            for (int i = 0; i < input.size(); i++) {
                double value = input[i];
                if (max < fabs(value)) {
                    max = fabs(value);
                }
            }
            for (int i = 0; i < input.size(); i++) {
                input[i] /= max;
            }
            return input;
        }

        static std::vector<std::vector<double>> NormalizeVectors(std::vector<std::vector<double>> input) {
            for (int i = 0; i < input.size(); i++) {
                double max = 0;
                for (int j = 0; j < input[i].size(); j++) {
                    double value = input[i][j];
                    if (max < fabs(value)) {
                        max = fabs(value);
                    }
                }
                for (int j = 0; j < input[i].size(); j++) {
                    input[i][j] /= max;
                }
            }
            return input;
        }

        static std::vector<std::vector<double>> NormalizeSummation(std::vector<std::vector<double>> input) {
            std::vector<double> sum;
            for (int i = 0; i < input.size(); i++) {
                double d = 0;
                for (int j = 0; j < input[i].size(); j++) {
                    d += std::abs(input[i][j]);
                }
                sum.push_back(d);
            }
            double max = 0;
            for (int i = 0; i < sum.size(); i++) {
                if (sum[i] > max) {
                    max = sum[i];
                }
            }
            for (int i = 0; i < input.size(); i++) {
                for (int j = 0; j < input[i].size(); j++) {
                    input[i][j] /= sum[i] / max;
                }
            }
            return input;
        }

        static std::vector<double> CopyVector(std::vector<double> &input, int begin, int length) {
            std::vector<double> vector;
            for (int j = 0; j < length; j++) {
                vector.push_back(input[begin + j]);
            }
            return vector;
        }

        // 相関関数が最大になるように入力のズレを修正
        static std::vector<double> MiximizeCrossCorrelation(std::vector<double> input, std::vector<double> vec) {
            if (input.size() != vec.size()) {
                abort();
            }

            int maxIndex = 0;
            double maxD = 0;
            int length = input.size();
            for (int i = 0; i < length; i++) {
                double d = 0;
                for (int j = 0; j < length; j++) {
                    d += input[(i + j) % length] * vec[j];
                }

                if (maxD < d) {
                    maxD = d;
                    maxIndex = i;
                }
            }

            std::vector<double> output;
            for (int i = 0; i < length; i++) {
                output.push_back(input[(maxIndex + i) % length]);
            }
            return output;
        }

        // 相関関数の値の配列を取得する
        static std::vector<double> GetCrossCorrelation(std::vector<double> input, std::vector<double> vec) {
            if (input.size() < vec.size()) {
                std::cout << "fjaskdfjklasdjkl" << std::endl;
                abort();
            }
            std::vector<double> crossCorrelation;
            for (int i = 0; i < input.size() - vec.size(); i++) {
                double d = 0;
                for (int j = 0; j < vec.size(); j++) {
                    d += input[i + j] * vec[j];
                }
                crossCorrelation.push_back(d);
            }
            return crossCorrelation;
        }

        // 入力の前後を0に近づける
        static std::vector<double> ZerofyFirstAndLast(std::vector<double> input) {
            int size = input.size();
            double ave = (input[0] + input[size - 1]) / 2;
            for (int i = 0; i < size; i++) {
                input[i] -= ave;
            }

            int fixSize = 5;
            int begin = 0;
            for (int i = 0; i < fixSize; i++) {
                /*
                   if (input[i] * input[i+1] < 0) {
                   double zero = i - input[i] / (input[i+1] - input[i]);
                   std::cout << "zero = " << zero << std::endl;
                   }
                   */
                if (input[i] == 0 || input[i] * input[i + 1] < 0) {
                    begin = i;
                    break;
                }
            }
            int end = size;
            for (int i = 0; i < fixSize; i++) {
                /*
                   if (input[size-1-i] * input[size-1-i-1] < 0) {
                   double zero = size-1-i - input[size-1-i-1] / (input[size-1-i] -
                   input[size-1-i-1]);
                   std::cout << "zeroend = " << zero << std::endl;
                   }
                   */
                if (input[size - 1 - i] == 0 || input[size - 1 - i] * input[size - 1 - i - 1] < 0) {
                    end = size - i;
                    break;
                }
            }

            std::vector<double> vec = CopyVector(input, begin, end - begin);
            return LinearInterpolation::convert(vec, size);
        }

        // Julius
        static std::vector<std::vector<double>> GetSplittedDataByJulius(std::vector<double> &input, int samplingSize, std::vector<JuliusResult> &juliusResults) {
            std::vector<std::vector<double>> splitted;
            for (int i = 0; i < juliusResults.size(); i++) {
                int from = juliusResults[i].from * samplingSize;
                int to = juliusResults[i].to * samplingSize;
                // std::cout << from << " " << to << " " << to - from << " " << result.unit << std::endl;
                std::vector<double> vec;
                for (int j = from; j < to; j++) {
                    vec.push_back(input[j]);
                }
                splitted.push_back(vec);
            }
            return splitted;
        }

        static std::string toString(int i) {
            std::stringstream ss;
            ss << i;
            return ss.str();
        }
};
