#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "wave.h"
#include "voice_wave_analyzer.h"
#include "kmeans_method.h"
#include "gnuplot.h"


std::vector<double> getInput() {
    std::vector<double> tmp;
    Wave wav;
    if (wav.InputWave("sample.wav") != 0) {
        abort();
    }
    wav.Normalize();
    wav.GetData(tmp);

    int padding = 40200;
    std::vector<double> input;
    for (int i = padding; i < tmp.size() - padding * 2; i++) {
        input.push_back(tmp[i]);
    }
    return input;
}




int main() {

    std::vector<double> input = getInput();
    std::cout << std::endl;
    std::cout << "入力: " << input.size() << " 個のサンプル" << std::endl << std::endl;
    
    // 周期を解析
    int samplingFrequency = 44100;
    int targetFrequency = 360;
    int errorFrequency = targetFrequency / 10;
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(input, samplingFrequency, targetFrequency-errorFrequency, targetFrequency+errorFrequency);

    // 周期切り出し
    int padding = 10;
    std::vector<std::vector<double> > inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        Cycle cycle = cycles[i];

        std::vector<double> inputCycle;
        for (int j = 0; j < cycle.length; j++) {
            inputCycle.push_back(input[cycle.index + j]);
        }

        // 足りない分を0で埋める
        while (inputCycle.size() < targetFrequency + padding) {
            inputCycle.push_back(0);
        }

        inputCycles.push_back(inputCycle);
    }

    Gnuplot<double>::OutputToGnuplot(inputCycles, "p 'output.txt' w l lc rgb '#F0FF0000'");


    // クラスタリング
    int countOfCluster = 4;
    int dim = targetFrequency + padding;
    KMeansMethodResult result = KMeansMethod::Clustering(inputCycles, dim, countOfCluster);

    Gnuplot<double>::OutputToGnuplot(result.clusters, "p 'output.txt' w l");


    // エラー率を算出
    std::vector<double> errors;
    for (int i = 0; i < inputCycles.size(); i++) {
        // 一番近いクラスター
        int index = result.indexOfCluster[i];
        std::vector<double> minCluster = result.clusters[index];

        // 二乗誤差を算出
        double error = 0;
        for (int j = 0; j < dim; j++) {
            error += std::abs(inputCycles[i][j] - minCluster[j]) / 2.0;
        }

        errors.push_back(error);
    }

    Gnuplot<double>::OutputToGnuplot(errors, "p 'output.txt' w l");

    Gnuplot<int>::OutputToGnuplot(result.indexOfCluster, "p 'output.txt'");


    return 0;
}


