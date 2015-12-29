#include "rnn_layer.h"

void RnnLayer::setup() {
    inWeights = std::vector<std::vector<double>>(
        midDim, std::vector<double>(inDim + 1, 0));
    midWeights =
        std::vector<std::vector<double>>(midDim, std::vector<double>(midDim));
    outWeights =
        std::vector<std::vector<double>>(outDim, std::vector<double>(midDim));
}

void RnnLayer::forward_in(const std::vector<double> &in,
                          const std::vector<double> &before) {
    std::vector<double> biased(inDim + 1);
    biased[0] = 1;
    for (int i = 1; i < inDim + 1; i++) {
        biased[i] = in[i];
    }

    std::vector<double> mid(midDim);
    for (int i = 0; i < midDim; i++) {
        double s = 0;
        for (int j = 0; j < inDim + 1; j++) {
            s += inWeights[i][j] * biased[j];
        }
        for (int j = 0; j < midDim; j++) {
            s += midWeights[i][j] * before[j];
        }
        mid[i] = s;
    }

    // softmax function
    double s = 0;
    for (int i = 0; i < midDim; i++) {
        s += exp(mid[i]);
    }
    for (int i = 0; i < midDim; i++) {
        mid[i] = exp(mid[i]) / s;
    }

    this->mid = mid;
}

std::vector<double> RnnLayer::forward_out() {
    std::vector<double> out(outDim);
    for (int i = 0; i < outDim; i++) {
        double d = 0;
        for (int j = 0; j < midDim; j++) {
            d += outWeights[i][j] * mid[j];
        }
    }
    return out;
}
