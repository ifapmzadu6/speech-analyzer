#pragma once

#include <iostream>
#include <vector>
#include <cmath>

class RnnLayer {
   public:
    RnnLayer(int inDim, int midDim, int outDim)
        : inDim(inDim), midDim(midDim), outDim(outDim){};

    void setup();

    void forward_in(const std::vector<double> &in,
                    const std::vector<double> &before);
    std::vector<double> forward_out();

    std::vector<double> getMid();

   private:
    int inDim;
    int midDim;
    int outDim;

    std::vector<std::vector<double>> inWeights;
    std::vector<std::vector<double>> midWeights;
    std::vector<std::vector<double>> outWeights;

    std::vector<double> mid;
};
