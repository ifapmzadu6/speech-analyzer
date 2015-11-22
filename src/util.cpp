
#include "util.h"

std::vector<double> Util::GetInput(std::string path, int padding)
{
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

std::vector<std::vector<double> >
Util::NormalizeVector(std::vector<std::vector<double> >& input)
{
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

std::vector<double> Util::CopyVector(std::vector<double>& input, int begin,
    int length)
{
    std::vector<double> vector;
    for (int j = 0; j < length; j++) {
        vector.push_back(input[begin + j]);
    }
    return vector;
}

// Julius
std::vector<std::vector<double> >
Util::GetSplittedDataByJulius(std::vector<double>& input, int samplingSize,
    std::vector<JuliusResult>& juliusResults)
{
    std::vector<std::vector<double> > splitted;
    for (int i = 0; i < juliusResults.size(); i++) {
        int from = juliusResults[i].from * samplingSize;
        int to = juliusResults[i].to * samplingSize;
        // std::cout << from << " " << to << " " << to - from << " " << result.unit
        // << std::endl;
        std::vector<double> vector;
        for (int j = from; j < to; j++) {
            vector.push_back(input[j]);
        }
        splitted.push_back(vector);
    }
    return splitted;
}
