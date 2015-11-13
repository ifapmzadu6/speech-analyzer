#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <climits>
#include <cmath>
#include <sstream>

#include "wave.h"
#include "voice_wave_analyzer.h"
#include "kmeans_method.h"
#include "gnuplot.h"
#include "julius_importer.h"
#include "audio.h"
#include "linear_interpolation.h"
#include "fir_filter.h"


// Input
std::vector<double> getInput(std::string path, int padding);
std::vector<std::vector<double>> normalizeVector(std::vector<std::vector<double>> &input);
std::vector<double> copyVector(std::vector<double> &input, int begin, int length);

// Julius
std::vector<std::vector<double>> getSplittedDataByJulius(std::vector<double> &input, int samplingSize, std::vector<JuliusResult> &juliusResults);
void enumurateJulius(std::vector<double> &input, int samplingSize, int from, std::vector<double> &julius);

// Clustering
void showClustering(std::vector<double> &input, int samplingSize);


int main() {

    int samplingSize = 16000;

    //auto input = getInput("./resource/a.wav", 15000);
    //showClustering(input, samplingSize);

    auto inputForJulius = getInput("./resource/nomura.wav", 0);
    JuliusImporter juliusImporter("./resource/nomura.lab");
    auto juliusResults = juliusImporter.getJuliusResults();
    auto splittedDataByJulius = getSplittedDataByJulius(inputForJulius, samplingSize, juliusResults);

    int count = 0;
    for (int i=0; i<juliusResults.size()-1; i++) {
        auto result = juliusResults[i];

        if ((
                    (result.unit == "a")
                    ||
                    (result.unit == "i")
                    ||
                    (result.unit == "u")
                    ||
                    (result.unit == "e")
                    ||
                    (result.unit == "o")
            ) && count < 100) {

            int from = result.from * samplingSize;
            enumurateJulius(inputForJulius, samplingSize, from, splittedDataByJulius[i]);
            //showClustering(splittedDataByJulius[i], samplingSize);

            count++;
        }
    }

    std::vector<double> vectors;
    for (int i=0; i<5; i++) {
        vectors.push_back(inputForJulius[i]);
    }

    FIRFilter filter(vectors.size(), vectors);

    std::vector<double> output;
    for (int i=vectors.size(); i<inputForJulius.size(); i++) {
        filter.next(inputForJulius[i]);
        output.push_back( filter.getValue() );
    }

    Wave wav_;
    wav_.CreateWave(output, samplingSize, 16);
    wav_.OutputWave("./output/heikin.wav");


    Wave wav;
    wav.CreateWave(inputForJulius, samplingSize, 16);
    wav.OutputWave("./output/input.wav");

    return 0;
}


std::vector<double> getInput(std::string path, int padding) {
    std::cout << "- \"" + path +  "\" -" << std::endl;
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


// Clustering
void showClustering(std::vector<double> &input, int samplingSize) {
    std::cout << std::endl;

    // 周期を解析
    int targetSize = 140;
    int errorSize = 70;
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(input, samplingSize, targetSize-errorSize, targetSize+errorSize);
    std::cout << "cycles.size() -> " << cycles.size() << std::endl;
    if (cycles.size() < 3) {
        return;
    }
    int maxLength = -1;
    for (int j=0; j<cycles.size(); j++) {
        if (maxLength < cycles[j].length) {
            maxLength = cycles[j].length;
        }
    }

    // 周期切り出し
    std::vector<std::vector<double>> inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        std::vector<double> inputCycle = copyVector(input, cycles[i].index, cycles[i].length);

        // 足りない分を0で埋める
        //int padding = 14;
        //while (inputCycle.size() < targetSize + padding) inputCycle.push_back(0);

        // 波形の長さを揃える
        inputCycle = LinearInterpolation::convert(inputCycle, maxLength);

        inputCycles.push_back(inputCycle);
    }
    Gnuplot<double>::Output2DToGnuplot(inputCycles, "inputCycles", "w l lc rgb '#E0FF0000'");


    // クラスタリング
    int countOfCluster = 5;
    //int dim = targetSize + padding;
    int dim = inputCycles[0].size();
    KMeansMethodResult result = KMeansMethod::Clustering(inputCycles, dim, countOfCluster);
    Gnuplot<double>::Output2DToGnuplot(result.clusters, "result.clusters", "w l");

    
    // どのクラスターが正解に近いかを表示
    if (true) {
        std::vector<std::vector<double>> clusters = result.clusters;
        std::vector<double> bestCluster;
        double best = std::numeric_limits<double>::max();
        for (int i=0; i<clusters.size(); i++) {
            double d = 0;
            for (int j=0; j<clusters.size(); j++) {
                if (i != j) {
                    for (int k=0; k<clusters[j].size(); k++) {
                        d += pow(clusters[j][k] - clusters[i][k], 2);
                    }
                }
            }
            if (d < best) {
                best = d;
                bestCluster = clusters[i];
            }
        }
        Gnuplot<double>::OutputToGnuplot(bestCluster, "bestCluster", "w l");
    }


    // エラー率を算出
    if (true) {
        std::vector<double> errors;
        for (int i = 0; i < inputCycles.size(); i++) {
            // 一番近いクラスター
            int index = result.indexOfCluster[i];
            std::vector<double> minCluster = result.clusters[index];

            // 二乗誤差を算出
            double error = 0;
            for (int j = 0; j < dim; j++) {
                error += fabs(inputCycles[i][j] - minCluster[j]) / 2.0;
            }

            errors.push_back(error);
        }
        Gnuplot<double>::OutputToGnuplot(errors, "errors", "w l");
        Gnuplot<int>::OutputToGnuplot(result.indexOfCluster, "result.indexOfCluster", "w p pt 3");
    }


    // 積分(=エネルギーを見る)
    if (true) {
        std::vector<double> energy;
        for (int i = 0; i < inputCycles.size(); i++) {
            double e = 0;
            for (int j = 0; j < inputCycles[i].size(); j++) {
                e += fabs(inputCycles[i][j]);
            }
            energy.push_back(e);
        }
        Gnuplot<double>::OutputToGnuplot(energy, "energy", "w l");
    }
}




// Julius
std::vector<std::vector<double>> getSplittedDataByJulius(std::vector<double> &input, int samplingSize, std::vector<JuliusResult> &juliusResults) {
    std::vector<std::vector<double>> splitted;
    for (int i=0; i<juliusResults.size(); i++) {
        int from = juliusResults[i].from * samplingSize;
        int to = juliusResults[i].to * samplingSize;
        //std::cout << from << " " << to << " " << to - from << " " << result.unit << std::endl;
        std::vector<double> vector;
        for (int j=from; j<to; j++) {
            vector.push_back(input[j]);
        }
        splitted.push_back(vector);
    }
    return splitted;
}


void enumurateJulius(std::vector<double> &input, int samplingSize, int from, std::vector<double> &julius) {
	std::stringstream ss;
	ss << from << " ";
	std::string gnuplotSubtitle = ss.str();

    int targetSize = 140;
    int errorSize = 40;
    auto cycles = VoiceWaveAnalyzer::GetCycles(julius, samplingSize, targetSize-errorSize, targetSize+errorSize);
    if (cycles.size() <= 2) {
        return;
    }
    std::cout << "cycles.size() -> " << cycles.size() << std::endl;

    if (cycles.size() < 5) {
        return;
    }



    std::vector<double> amps, tops;
    // 先頭の波を利用する
    auto defaultCycle = cycles[3];
    auto defaultWave = copyVector(julius, defaultCycle.index, defaultCycle.length);



    int maxLength = -1;
    for (int j=0; j<cycles.size(); j++) {
        if (maxLength < cycles[j].length) {
            maxLength = cycles[j].length;
        }
    }
    // 周期切り出し
    std::vector<std::vector<double>> inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        std::vector<double> inputCycle = copyVector(julius, cycles[i].index, cycles[i].length);
        // 足りない分を0で埋める
        //int padding = 14;
        //while (inputCycle.size() < targetSize + padding) inputCycle.push_back(0);
        // 波形の長さを揃える
        inputCycle = LinearInterpolation::convert(inputCycle, maxLength);
        inputCycles.push_back(inputCycle);
    }

    // クラスタリング
    int countOfCluster = 5;
    KMeansMethodResult result = KMeansMethod::Clustering(inputCycles, maxLength, countOfCluster);
    Gnuplot<double>::Output2DToGnuplot(result.clusters, gnuplotSubtitle + "result.clusters", "w l");

    // 一番中心となっているクラスターを見つける
    std::vector<std::vector<double>> clusters = result.clusters;
    double best = std::numeric_limits<double>::max();
    for (int i=0; i<clusters.size(); i++) {
        double d = 0;
        for (int j=0; j<clusters.size(); j++) {
            if (i != j) {
                for (int k=0; k<clusters[j].size(); k++) {
                    d += pow(clusters[j][k] - clusters[i][k], 2);
                }
            }
        }
        if (d < best) {
            best = d;
            defaultWave = clusters[i];
        }
    }
    Gnuplot<double>::OutputToGnuplot(defaultWave, gnuplotSubtitle + "defaultWave", "w l");


    double defaultMax = -1;
    double defaultMin = 1;
    for (int k=0; k<defaultWave.size(); k++) {
        double value = defaultWave[k];
        if (value > defaultMax) {
            defaultMax = value;
        }
        if (value < defaultMin) {
            defaultMin = value;
        }
    }
    double defaultAmp = fabs(defaultMax-defaultMin);

    Wave wav;
    wav.CreateWave(defaultWave, samplingSize, 16);
    wav.OutputWave("./output/output_default_wave.wav");

    if (true) {
        for(int i=0; i<cycles.size(); i++) {
            double max = -1, min = 1;
            for (int k=0; k<cycles[i].length; k++) {
                double value = julius[ cycles[i].index + k ];
                if (value > max) {
                    max = value;
                }
                if (value < min) {
                    min = value;
                }
            }
            double amp = fabs(max-min);

            // 入力の書き換え
            for (int j=0; j<cycles[i].length; j++) {
                input.erase(input.begin() + from + cycles[i].index);
            }
            //std::cout << "input.size() -> " << input.size() << std::endl;

            auto wave = LinearInterpolation::convert(defaultWave, cycles[i].length);
            double rate = amp / defaultAmp;
            double diff = max - defaultMax;
            for (int k=0; k<cycles[i].length; k++) {
                int index = from + cycles[i].index + k;
                input.insert(input.begin() + index, wave[k] * rate - diff);
            }
            //std::cout << "input.size() -> " << input.size() << std::endl;
        }
    }

    
    // 波形の表示
    if (true) {
        std::vector<double> gnuplot2;
        int margin = 100;
        for (int j=0; j<julius.size()+margin*2; j++) {
            gnuplot2.push_back( input[from+j-margin] );
        }
        Gnuplot<double>::OutputToGnuplot(gnuplot2, gnuplotSubtitle + "gnuplot2", "w l");
    }

}


std::vector<std::vector<double>> normalizeVector(std::vector<std::vector<double>> &input) {
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

std::vector<double> copyVector(std::vector<double> &input, int begin, int length) {
    std::vector<double> vector;
    for (int j=0; j<length; j++) {
        vector.push_back(input[begin+j]);
    }
    return vector;
}


