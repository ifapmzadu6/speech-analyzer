#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <climits>
#include <cmath>

#include "wave.h"
#include "voice_wave_analyzer.h"
#include "kmeans_method.h"
#include "gnuplot.h"
#include "julius_importer.h"
#include "audio.h"


// Input
std::vector<double> getInput(std::string path, int padding);
std::vector<std::vector<double>> normalizeVector(std::vector<std::vector<double>> &input);
std::vector<double> copyVector(std::vector<double> &input, int begin, int length);

// Julius
std::vector<std::vector<double>> getSplittedDataByJulius(std::vector<double> &input, int samplingSize, std::vector<JuliusResult> &juliusResults);
void juliusProcess(std::vector<double> &input, int samplingSize, std::vector<std::vector<double>> &julius, std::vector<JuliusResult> &results);
void enumurateJulius(std::vector<double> &input, int samplingSize, int from, std::vector<double> &julius);

// Clustering
void showClustering(std::vector<double> &input, int samplingSize);


int main() {
    int samplingSize = 16000;

    //auto input = getInput("./resource/a.wav", 15000);
    //showClustering(input, samplingSize);


    auto inputForJulius = getInput("./resource/output.wav", 0);
    JuliusImporter juliusImporter("./resource/output.lab");
    auto juliusResults = juliusImporter.getJuliusResults();
    auto splittedDataByJulius = getSplittedDataByJulius(inputForJulius, samplingSize, juliusResults);

    juliusProcess(inputForJulius, samplingSize, splittedDataByJulius, juliusResults);

    return 0;
}





// Input
std::vector<double> getInput(std::string path, int padding) {
    std::cout << "- \"" + path +  "\" -" << std::endl;
    Wave wav;
    if (wav.InputWave(path) != 0)
        abort();
    wav.Normalize();

    std::vector<double> tmp;
    wav.GetData(&tmp);

    std::vector<double> input;
    for (int i = padding; i < tmp.size() - padding * 2; i++) 
        input.push_back(tmp[i]);
    std::cout << std::endl << "input.size() ->" << input.size() << std::endl << std::endl;
    return input;
}

std::vector<std::vector<double>> normalizeVector(std::vector<std::vector<double>> &input) {
    for (int i=0; i<input.size(); i++) {
        double max = 0;
        for (int j=0; j<input[i].size(); j++) {
            double value = input[i][j];
            if (max < fabs(value))
                max = fabs(value);
        }
        for (int j=0; j<input[i].size(); j++)
            input[i][j] /= max;
    }
    return input;
}

std::vector<double> copyVector(std::vector<double> &input, int begin, int length) {
    std::vector<double> vector;
    for (int j=0; j<length; j++) vector.push_back(input[begin+j]);
    return vector;
}



// Clustering
void showClustering(std::vector<double> &input, int samplingSize) {
    // 周期を解析
    int targetSize = 140;
    int errorSize = 40;
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(input, samplingSize, targetSize-errorSize, targetSize+errorSize);
    std::cout << "cycles.size() -> " << cycles.size() << std::endl;
    std::cout << std::endl;

    // 周期切り出し
    int padding = 14;
    std::vector<std::vector<double>> inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        Cycle cycle = cycles[i];

        std::vector<double> inputCycle;
        for (int j = 0; j < cycle.length; j++)
            inputCycle.push_back(input[cycle.index + j]);

        // 足りない分を0で埋める
        while (inputCycle.size() < targetSize + padding) 
            inputCycle.push_back(0);

        inputCycles.push_back(inputCycle);
    }
    Gnuplot<double>::Output2DToGnuplot(inputCycles, "w l lc rgb '#E0FF0000'");


    // クラスタリング
    int countOfCluster = 5;
    int dim = targetSize + padding;
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
        for (int j = 0; j < dim; j++) 
            error += fabs(inputCycles[i][j] - minCluster[j]) / 2.0;

        errors.push_back(error);
    }

    Gnuplot<double>::OutputToGnuplot(errors, "w l");
    Gnuplot<int>::OutputToGnuplot(result.indexOfCluster, "w p pt 3");


    // 積分(=エネルギーを見る)
    std::vector<double> energy;
    for (int i = 0; i < inputCycles.size(); i++) {
        double e = 0;
        for (int j = 0; j < inputCycles[i].size(); j++)
            e += fabs(inputCycles[i][j]);
        energy.push_back(e);
    }
    Gnuplot<double>::OutputToGnuplot(energy, "w l");
}




// Julius
std::vector<std::vector<double>> getSplittedDataByJulius(std::vector<double> &input, int samplingSize, std::vector<JuliusResult> &juliusResults) {
    std::vector<std::vector<double>> splitted;
    for (int i=0; i<juliusResults.size(); i++) {
        int from = juliusResults[i].from * samplingSize;
        int to = juliusResults[i].to * samplingSize;
        //std::cout << from << " " << to << " " << to - from << " " << result.unit << std::endl;
        std::vector<double> vector;
        for (int j=from; j<to; j++)
            vector.push_back(input[j]);
        splitted.push_back(vector);
    }
    return splitted;
}


void juliusProcess(std::vector<double> &input, int samplingSize, std::vector<std::vector<double>> &julius, std::vector<JuliusResult> &results) {
    for (int i=1; i<julius.size()-1; i++) {
        auto result = results[i];
        if (result.unit == "a" && i > 40) {

            //int from = result.from * samplingSize;
            //enumurateJulius(input, samplingSize, from, julius[i]);
            showClustering(julius[i], samplingSize);

            break;
        }
    }

    Wave wav;
    wav.CreateWave(input, samplingSize, 16);
    wav.OutputWave("./output/input.wav");
}

void enumurateJulius(std::vector<double> &input, int samplingSize, int from, std::vector<double> &julius) {
    int targetSize = 140;
    int errorSize = 40;
    auto cycles = VoiceWaveAnalyzer::GetCycles(julius, samplingSize, targetSize-errorSize, targetSize+errorSize);
    std::cout << "cycles.size() -> " << cycles.size() << std::endl;

    int begin = cycles[0].index;
    int end = cycles[cycles.size()-1].index + cycles[cycles.size()-1].length;
    std::cout << "begin -> " << begin << std::endl;
    std::cout << "end -> " << end << std::endl;

    auto vector = copyVector(julius, cycles[0].index, cycles[0].length);
    std::cout << "vector.size() -> " << vector.size() << std::endl;

    std::vector<double> maxDs;
    // 一時的に最大値を求めてみる
    for(int j=0; j<cycles.size(); j++) {
        double max = -1;
        double min = 1;
        double d = 0;
        for (int k=0; k<cycles[j].length; k++) {
            double value = julius[ cycles[j].index + k ];
            if (value > max) max = value;
            if (value < min) min = value;
            if (fabs(value) > d) d = fabs(value);
        }
        maxDs.push_back(d);
        //std::cout << "max -> " << max << ", min -> " << min << std::endl;
    }

    double max = 0;
    for (int j=0; j<maxDs.size(); j++) 
        if (maxDs[j] > max) max = maxDs[j];

    std::vector<double> rates;
    for (int j=0; j<maxDs.size(); j++)
        rates.push_back(maxDs[j] / max);
    Gnuplot<double>::OutputToGnuplot(rates, "w l");

    for (int j=begin; j<end; j++) 
        input.erase(input.begin() + from + begin);
    std::cout << "input.size() -> " << input.size() << std::endl;
    
    for (int j=0; j<cycles.size(); j++)
        for (int k=0; k<vector.size(); k++)
            input.insert(input.begin() + from+begin + vector.size()*j+k, vector[k] * rates[j]);
    std::cout << "input.size() -> " << input.size() << std::endl;


    std::vector<double> gnuplot2;
    for (int j=-3000; j<end+3000; j++)
        gnuplot2.push_back(input[from+begin+j]);
    Gnuplot<double>::OutputToGnuplot(gnuplot2, "w l");
}

