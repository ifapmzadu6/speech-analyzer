
#include "voice_wave_analyzer.h"

#include "gnuplot.h"

std::vector<Cycle> VoiceWaveAnalyzer::GetCycles(const std::vector<double> &input, int initIndex, int minLength, int maxLength) {
    int inputSize = input.size();               // 入力のサンプル数
    int dsize = (minLength + maxLength) / 2;    // 相関関数の幅
    int fixSize = (maxLength - minLength) / 2;  // 零点修正誤差範囲

    double maxValue = 0;
    // ウィンドウサイズを修正
    for (int j = 0; j < inputSize - dsize - 1; j++) {
        // 相関関数を求める
        double value = 0;
        for (int l = 0; l < dsize; l++) {
            value += input[j + l] * input[j + l + dsize];
        }
        if (maxValue < value) {
            maxValue = value;
        }
    }
    // ボーダーの値を決定する
    double border = maxValue / 10;
    border = 0;

    // 入力を探索し、それぞれの相関関数を求め、
    // 最大となる（波形が似ている）箇所を探し、保存していく
    int index = initIndex;
    std::vector<int> indexes;
    while (index + maxLength + dsize < inputSize) {
        double maxValue = 0;
        double maxIndex = 1;
        // 相関関数が最大になる部分を探索
        for (int i = minLength; i < maxLength; i++) {
            double value = 0;
            for (int j = 0; j < dsize; j++) {
                value += input[index + i + j] * input[index + j];
            }
            if (value > maxValue) {
                maxValue = value;
                maxIndex = i;
            }
        }
        if (maxValue > border) {
            // indexesに現在のindexを保存
            indexes.push_back(index);
        }
        // 最大の分だけindexを移動
        index += maxIndex;
    }

    // indexesの零点を修正
    std::vector<int> fixedIndexes;
    for (int i = 0; i < indexes.size(); i++) {
        int index = indexes[i];
        // indexの前後を調べ、符号が逆になる箇所を探す
        for (int j = 0; j < fixSize; j++) {
            if ((index + j + 1 < inputSize) && (input[index + j] * input[index + j + 1] <= 0)) {
                fixedIndexes.push_back(index + j);
                break;
            } else if ((index - j - 1 > 0) && (input[index - j - 1] * input[index - j] <= 0)) {
                fixedIndexes.push_back(index - j - 1);
                break;
            }
        }
    }

    if (fixedIndexes.size() == 0 || indexes.size() == 0) {
        // std::cout << "no results" << std::endl;
        return std::vector<Cycle>();
    }

    std::vector<Cycle> cycles;
    for (int i = 0; i < fixedIndexes.size() - 1; i++) {
        int length = fixedIndexes[i + 1] - fixedIndexes[i];
        if ((minLength < length) && (length < maxLength)) {
            Cycle cycle;
            cycle.index = fixedIndexes[i];
            cycle.length = length;
            cycles.push_back(cycle);
        }
    }

    /*
       std::cout << "zero detection rate: "
       << 100.0 * fixedIndexes.size() / indexes.size() << "%"
       << std::endl;
       std::cout << "correct data rate : "
       << 100.0 * cycles.size() / fixedIndexes.size() << "%"
       << std::endl;
       */

    return cycles;
}
