#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

#include "julius_importer.h"
#include "wave.h"

class Util {
   public:
    static std::vector<double> GetInput(std::string path, int padding);

    static std::vector<double> NormalizeVector(std::vector<double> input);

    static std::vector<std::vector<double>> NormalizeVectors(
        std::vector<std::vector<double>> input);

    static std::vector<std::vector<double>> NormalizeSummation(
        std::vector<std::vector<double>> input);

    static std::vector<double> CopyVector(std::vector<double> &input, int begin,
                                          int length);

    // 相関関数が最大になるように入力のズレを修正
    static std::vector<double> MiximizeCrossCorrelation(
        std::vector<double> input, std::vector<double> vec);

    // 入力の前後を0に近づける
    static std::vector<double> ZerofyFirstAndLast(std::vector<double> input);

    // Julius
    static std::vector<std::vector<double>> GetSplittedDataByJulius(
        std::vector<double> &input, int samplingSize,
        std::vector<JuliusResult> &juliusResults);

    static std::string toString(int i);
};
