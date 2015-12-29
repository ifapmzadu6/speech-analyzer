
#include "util.h"

std::vector<double> Util::GetInput(std::string path, int padding) {
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
    std::cout << std::endl
              << "input.size() ->" << input.size() << std::endl
              << std::endl;
    return input;
}

std::vector<double> Util::NormalizeVector(std::vector<double> input) {
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

std::vector<std::vector<double>> Util::NormalizeVectors(
    std::vector<std::vector<double>> input) {
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

std::vector<std::vector<double>> Util::NormalizeSummation(
    std::vector<std::vector<double>> input) {
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

std::vector<double> Util::CopyVector(std::vector<double> &input, int begin,
                                     int length) {
    std::vector<double> vector;
    for (int j = 0; j < length; j++) {
        vector.push_back(input[begin + j]);
    }
    return vector;
}

std::vector<double> Util::MiximizeCrossCorrelation(std::vector<double> input,
                                                   std::vector<double> vec) {
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
        output.push_back(input[(maxIndex + i) % (length - 4)]);
    }
    return output;
}

std::vector<double> Util::ZerofyFirstAndLast(std::vector<double> input) {
    double first = input[0];
    double last = input[input.size() - 1];
    double ave = (first + last) / 2;
    std::vector<double> output;
    for (int i = 0; i < input.size(); i++) {
        double val = input[i] - ave;
        output.push_back(val);
    }
    return output;
}

// Julius
std::vector<std::vector<double>> Util::GetSplittedDataByJulius(
    std::vector<double> &input, int samplingSize,
    std::vector<JuliusResult> &juliusResults) {
    std::vector<std::vector<double>> splitted;
    for (int i = 0; i < juliusResults.size(); i++) {
        int from = juliusResults[i].from * samplingSize;
        int to = juliusResults[i].to * samplingSize;
        // std::cout << from << " " << to << " " << to - from << " " <<
        // result.unit
        // << std::endl;
        std::vector<double> vector;
        for (int j = from; j < to; j++) {
            vector.push_back(input[j]);
        }
        splitted.push_back(vector);
    }
    return splitted;
}

std::string Util::toString(int i) {
    std::stringstream ss;
    ss << i;
    return ss.str();
}
