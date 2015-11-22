#include <iostream>
#include <vector>
#include <cmath>

#include "julius_importer.h"
#include "wave.h"

#ifndef util_h
#define util_h

class Util {
public:
    static std::vector<double> GetInput(std::string path, int padding);

    static std::vector<std::vector<double> >
    NormalizeVector(std::vector<std::vector<double> >& input);

    static std::vector<double> CopyVector(std::vector<double>& input, int begin,
        int length);

    // Julius
    static std::vector<std::vector<double> >
    GetSplittedDataByJulius(std::vector<double>& input, int samplingSize,
        std::vector<JuliusResult>& juliusResults);
};

#endif
