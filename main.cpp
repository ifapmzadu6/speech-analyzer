#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <climits>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>

#include "wave.h"
#include "voice_wave_analyzer.h"
#include "kmeans_method.h"
#include "gnuplot.h"
#include "julius_importer.h"
#include "audio.h"


std::vector<std::vector<double>> normalize(std::vector<std::vector<double>> input) {
    for (int i=0; i<input.size(); i++) {
        double max = 0;
        for (int j=0; j<input[i].size(); j++) {
            double value = input[i][j];
            if (max < fabs(value)) {
                max = fabs(value);
            }
        }
        for (int j=0; j<input[i].size(); j++) {
            input[i][j] /= max;
        }
    }
    return input;
}


std::vector<double> getInput(std::string path, int padding) {
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

    std::cout << std::endl << "input.size() ->" << input.size() << std::endl << std::endl;
    return input;
}


void julius() {
    JuliusImporter juliusImporter("./resources/output.lab");
    auto results = juliusImporter.getJuliusResults();

    int samplingSize = 16000;
    auto input = getInput("./resources/output.wav", 0);

    std::vector<std::vector<double>> julius;
    for (int i=0; i<results.size(); i++) {
        auto result = results[i];
        int from = result.from * samplingSize;
        int to = result.to * samplingSize;
        //std::cout << from << " " << to << " " << to - from << " " << result.unit << std::endl;
        std::vector<double> vector;
        for (int j=from; j<to; j++) {
            vector.push_back(input[j]);
        }
        julius.push_back(vector);
    }
    //julius = normalize(julius);
    std::cout << "julius.size() -> " << julius.size() << std::endl;


    // 全て単一の波で置き換えてみる
    for (int i=1; i<julius.size()-1; i++) {
        auto result = results[i];
        if (result.unit == "a" && i > 40) {
            int targetSize = 140;
            int errorSize = 40;
            auto cycles = VoiceWaveAnalyzer::GetCycles(julius[i], samplingSize, targetSize - errorSize, targetSize + errorSize);
            std::cout << "cycles.size() -> " << cycles.size() << std::endl;

            int from = result.from * samplingSize;

            auto first = cycles[0];
            int begin = first.index;
            auto last = cycles[cycles.size()-1];
            int end = last.index + last.length;
            std::cout << "begin -> " << begin << std::endl;
            std::cout << "end -> " << end << std::endl;

            std::vector<double> vector;
            for (int j=first.index; j<first.index+first.length; j++) {
                vector.push_back(julius[i][j]);
            }
            std::cout << "vector.size() -> " << vector.size() << std::endl;

            for (int j=begin; j<end; j++) {
                //input.erase(input.begin() + from + begin);
            }
            std::cout << "input.size() -> " << input.size() << std::endl;

            for (int j=0; j<cycles.size(); j++) {
                for (int k=0; k<vector.size(); k++) {
                    //input.insert(input.begin() + from+begin + vector.size()*j+k, vector[k]);
                }
            }
            std::cout << "input.size() -> " << input.size() << std::endl;


            std::vector<double> gnuplot2;
            for (int j=from+begin - 4000; j<from+begin+end + 4000; j++) {
                gnuplot2.push_back(input[j]);
            }
            Gnuplot<double>::OutputToGnuplot(gnuplot2, "w l");

            break;
        }
    }




    Wave wav;
    wav.CreateWave(input, samplingSize, 16);
    wav.OutputWave("./output/input.wav");


    std::cout << std::endl << std::endl << std::endl;

    return;




    std::vector<std::vector<double>> gnuplot;
    for (int i=1; i<julius.size()-1; i++) {
        auto result = results[i];
        if (result.unit == "a" && i > 40) {
            gnuplot.push_back(julius[i]);

            int targetSize = 140;
            int errorSize = 40;
            auto cycles = VoiceWaveAnalyzer::GetCycles(julius[i], samplingSize, targetSize - errorSize, targetSize + errorSize);
            std::cout << "cycles.size() -> " << cycles.size() << std::endl;

            std::vector<std::vector<double>> gnuplots;
            for (int j=0; j<cycles.size(); j++) {
                auto cycle = cycles[j];
                std::vector<double> vector;
                for (int k = cycle.index; k < cycle.index + cycle.length; k++) {
                    vector.push_back(julius[i][k]);
                }
                int padding = 10;
                while (vector.size() < targetSize + padding) {
                    vector.push_back(0);
                }
                gnuplots.push_back(vector);
            }

            Gnuplot<double>::Output2DToGnuplot(gnuplots, "w l lc rgb '#60FF0000'");

            break;
        }
    }
    Gnuplot<double>::Output2DToGnuplot(gnuplot, "w l");

    /*
    for (int i=0; i<julius.size(); i++) {
        Wave wav;
        wav.CreateWave(julius[i], samplingSize, 16);
        wav.Normalize();
        wav.OutputWave("./output/output" + std::to_string(i) + "_" + results[i].unit + ".wav");
    }
    */

};


int main() {

    julius();
    return 0;
    
    auto input = getInput("./resources/sample.wav", 40200);
    
    // 周期を解析
    int samplingFrequency = 44100;
    int targetFrequency = 360;
    int errorFrequency = targetFrequency / 10;
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(input, samplingFrequency, targetFrequency-errorFrequency, targetFrequency+errorFrequency);

    // 周期切り出し
    int padding = 10;
    std::vector<std::vector<double>> inputCycles;
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

    Gnuplot<double>::Output2DToGnuplot(inputCycles, "w l lc rgb '#F0FF0000'");


    // クラスタリング
    int countOfCluster = 4;
    int dim = targetFrequency + padding;
    KMeansMethodResult result = KMeansMethod::Clustering(inputCycles, dim, countOfCluster);

    Gnuplot<double>::Output2DToGnuplot(result.clusters, "w l");


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

    Gnuplot<double>::OutputToGnuplot(errors, "w l");

    Gnuplot<int>::OutputToGnuplot(result.indexOfCluster, nullptr);

    // 積分(=エネルギーを見る)
    std::vector<double> energy;
    for (int i = 0; i < inputCycles.size(); i++) {
        std::vector<double> inputCycle = inputCycles[i];
        double e = 0;
        for (int j = 0; j < inputCycle.size(); j++) {
            e += std::abs(inputCycle[j]);
        }
        energy.push_back(e);
    }

    Gnuplot<double>::OutputToGnuplot(energy, "w l");

    return 0;
}


