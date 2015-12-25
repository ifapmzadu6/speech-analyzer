#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <climits>
#include <cmath>
#include <sstream>
#include <random>

#include "wave.h"
#include "voice_wave_analyzer.h"
#include "kmeans_method.h"
#include "gnuplot.h"
#include "julius_importer.h"
#include "audio.h"
#include "linear_interpolation.h"
#include "kernel_density_estimation.h"
#include "util.h"

// Julius
void enumurateJulius(std::vector<double>& input, int samplingSize, int from,
    std::vector<std::vector<double> >& splittedDataByJulius, std::vector<JuliusResult>& juliusResults, int indexOfJulius);

// Clustering
void showClustering(std::vector<double>& input, int samplingSize);

struct UnitWave {
    std::string unit;
    std::string before;
    std::string after;
    std::vector<double> wave;
};

std::vector<UnitWave> unitWaves;

int main()
{
    int samplingSize = 16000;

    // auto input = getInput("./resource/a.wav", 15000);
    // showClustering(input, samplingSize);

    for (int h = 1; h <= 13; h++) {
        std::stringstream ss;
        ss << h;

        auto inputForJulius = Util::GetInput("./resource/" + ss.str() + ".wav", 0);
        JuliusImporter juliusImporter("./resource/" + ss.str() + ".lab");
        auto juliusResults = juliusImporter.getJuliusResults();
        auto splittedDataByJulius = Util::GetSplittedDataByJulius(inputForJulius, samplingSize, juliusResults);

        for (int i = 0; i < juliusResults.size(); i++) {
            auto result = juliusResults[i];
            if (((result.unit == "a") || (result.unit == "i") || (result.unit == "u") || (result.unit == "e") || (result.unit == "o"))) {
                int from = result.from * samplingSize;
                enumurateJulius(inputForJulius, samplingSize, from,
                    splittedDataByJulius, juliusResults, i);
                // showClustering(splittedDataByJulius[i], samplingSize);
            }
        }
        Wave wav;
        wav.CreateWave(inputForJulius, samplingSize, 16);
        wav.OutputWave("./output/input.wav");
    }


    // * + 母音 + * で分類
    std::vector<std::vector<UnitWave>> tryphones;
    for (int i=0; i<unitWaves.size(); i++) {
        UnitWave unitWave = unitWaves[i];

        int indexOfTryphones = -1;
        for (int j=0; j<tryphones.size(); j++) {
            UnitWave firstUnitWave = tryphones[j][0];
            if (unitWave.unit == firstUnitWave.unit &&
                    unitWave.before == firstUnitWave.before &&
                    unitWave.after == firstUnitWave.after) {
                indexOfTryphones = j;
            }
        }
        if (indexOfTryphones >= 0) { // 既存のものがあった場合
            std::vector<UnitWave> tryphone = tryphones[indexOfTryphones];
            tryphone.push_back(unitWave);
            tryphones[indexOfTryphones] = tryphone;
        }
        else { // 既存のものがない場合
            std::vector<UnitWave> tryphone;
            tryphone.push_back(unitWave);
            tryphones.push_back(tryphone);
        }
    }

    if (true) {
        std::cout << "Total: " << unitWaves.size() << std::endl;
        for (int i=0; i<tryphones.size(); i++) {
            std::vector<UnitWave> units = tryphones[i];
            std::cout << units[0].before << "-" << units[0].unit << "-" << units[0].after << ": " << units.size() << std::endl;
        }
    }

    for (int i=0; i<tryphones.size(); i++) {
        std::vector<UnitWave> units = tryphones[i];
        if (units[0].unit != "a") {
            continue;
        }

        std::vector<std::vector<double> > waves;
        std::vector<int> indexesOfWaves;
        for (int i = 0; i < units.size(); i++) {
            auto wave = LinearInterpolation::convert(units[i].wave, 300);
            waves.push_back(wave);
            indexesOfWaves.push_back(i);
        }

        int bestIndex = KernelDensityEstimation::IndexOfMaxDensity(waves);
        std::vector<double> bestWave = waves[bestIndex];
        std::vector<std::vector<double> > org;
        for (int i = 0; i < waves.size(); i++) {
            std::vector<double> vec = Util::MiximizeCrossCorrelation(waves[i], bestWave);
            vec = Util::ZerofyFirstAndLast(vec);
            //vec = Util::NormalizeVector(vec);
            org.push_back(vec);
        }
        org = Util::NormalizeSummation(org);
        Gnuplot<double>::Output2D(org, "unitWaves",
                "w l lc rgb '#E0FF0000'");

        int countOfCluster = 10;
        int dim = waves[0].size();
        KMeansMethodResult result = KMeansMethod::Clustering(org, dim, countOfCluster);
        Gnuplot<double>::Output2D(result.clusters, "org.result.clusters", "w l");

        int bestClusterIndex = KernelDensityEstimation::IndexOfMaxDensity(org);
        Gnuplot<double>::Output(org[bestClusterIndex], "most best waveform", "w l");
    }

    return 0;
}

// Clustering
void showClustering(std::vector<double>& input, int samplingSize)
{
    std::cout << std::endl;

    // 周期を解析
    int targetSize = 140;
    int errorSize = 70;
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(
        input, samplingSize, targetSize - errorSize, targetSize + errorSize);
    std::cout << "cycles.size() -> " << cycles.size() << std::endl;
    if (cycles.size() < 3) {
        return;
    }
    int maxLength = -1;
    for (int j = 0; j < cycles.size(); j++) {
        if (maxLength < cycles[j].length) {
            maxLength = cycles[j].length;
        }
    }

    // 周期切り出し
    std::vector<std::vector<double> > inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        std::vector<double> inputCycle = Util::CopyVector(input, cycles[i].index, cycles[i].length);

        // 足りない分を0で埋める
        // int padding = 14;
        // while (inputCycle.size() < targetSize + padding) inputCycle.push_back(0);

        // 波形の長さを揃える
        inputCycle = LinearInterpolation::convert(inputCycle, maxLength);

        inputCycles.push_back(inputCycle);
    }
    inputCycles = Util::NormalizeSummation(inputCycles);
    Gnuplot<double>::Output2D(inputCycles, "inputCycles",
        "w l lc rgb '#E0FF0000'");

    // クラスタリング
    int countOfCluster = 5;
    // int dim = targetSize + padding;
    int dim = inputCycles[0].size();
    KMeansMethodResult result = KMeansMethod::Clustering(inputCycles, dim, countOfCluster);
    Gnuplot<double>::Output2D(result.clusters, "result.clusters", "w l");

    // どのクラスターが正解に近いかを表示
    if (false) {
        std::vector<std::vector<double> > clusters = result.clusters;
        std::vector<double> bestCluster;
        double best = std::numeric_limits<double>::max();
        for (int i = 0; i < clusters.size(); i++) {
            double d = 0;
            for (int j = 0; j < clusters.size(); j++) {
                for (int k = 0; k < clusters[j].size(); k++) {
                    d += pow(clusters[j][k] - clusters[i][k], 2);
                }
            }
            if (d < best) {
                best = d;
                bestCluster = clusters[i];
            }
        }
        Gnuplot<double>::Output(bestCluster, "bestCluster", "w l");
    }
    /*
    if (true) {
        int index = -1;
        std::vector<std::vector<double> > clusters = result.clusters;
        for (int i=0; i<clusters.size(); i++) {
            for (int j=0; j<inputCycles.size(); j++) {

                for (int k=0; k<inputCycles[j].size(); k++) {
                    inputCycles[j][k]
                }

            }

            if () {
            }
        }
        double correctRate = 0;
    }
    */

    // エラー率を算出
    if (false) {
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
        Gnuplot<double>::Output(errors, "errors", "w l");
        Gnuplot<int>::Output(result.indexOfCluster,
            "result.indexOfCluster", "w p pt 3");
    }

    // 積分(=エネルギーを見る)
    if (false) {
        std::vector<double> energy;
        for (int i = 0; i < inputCycles.size(); i++) {
            double e = 0;
            for (int j = 0; j < inputCycles[i].size(); j++) {
                e += fabs(inputCycles[i][j]);
            }
            energy.push_back(e);
        }
        Gnuplot<double>::Output(energy, "energy", "w l");
    }
}

// Julius
void enumurateJulius(std::vector<double>& input, int samplingSize, int from,
    std::vector<std::vector<double> >& splittedDataByJulius, std::vector<JuliusResult>& juliusResults, int indexOfJulius)
{
    std::stringstream ss;
    ss << from << " " << juliusResults[indexOfJulius - 1].unit << "-" << juliusResults[indexOfJulius].unit << "-" << juliusResults[indexOfJulius + 1].unit << "  ";
    std::string gnuplotSubtitle = ss.str();

    std::vector<double> julius = splittedDataByJulius[indexOfJulius];

    int targetSize = 140;
    int errorSize = 40;
    auto cycles = VoiceWaveAnalyzer::GetCycles(
        julius, samplingSize, targetSize - errorSize, targetSize + errorSize);
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
    auto defaultWave = Util::CopyVector(julius, defaultCycle.index, defaultCycle.length);

    if (true) {
        int maxLength = -1;
        for (int j = 0; j < cycles.size(); j++) {
            if (maxLength < cycles[j].length) {
                maxLength = cycles[j].length;
            }
        }
        // 周期切り出し
        std::vector<std::vector<double> > inputCycles;
        for (int i = 0; i < cycles.size(); i++) {
            std::vector<double> inputCycle = Util::CopyVector(julius, cycles[i].index, cycles[i].length);
            // 波形の長さを揃える
            inputCycle = LinearInterpolation::convert(inputCycle, maxLength);
            inputCycles.push_back(inputCycle);
        }
        if (false) {
            Gnuplot<double>::Output2D(inputCycles, gnuplotSubtitle + "inputCycles",
                "w l lc rgb '#E0FF0000'");
        }

        // 一番重心となっているものを見つける
        int bestClusterIndex = KernelDensityEstimation::IndexOfMaxDensity(inputCycles);
        defaultWave = inputCycles[bestClusterIndex];
    }
    if (true) {
        UnitWave unitWave;
        unitWave.unit = juliusResults[indexOfJulius].unit;
        unitWave.before = juliusResults[indexOfJulius - 1].unit;
        unitWave.after = juliusResults[indexOfJulius + 1].unit;
        unitWave.wave = defaultWave;
        unitWaves.push_back(unitWave);
    }
    if (false) {
        Gnuplot<double>::OutputCyclize(defaultWave, gnuplotSubtitle + "defaultWave",
            "w l");
    }
    /*
    if (juliusResults[indexOfJulius].unit == "a") {
        if (true) {
            Gnuplot<double>::Output(defaultWave, gnuplotSubtitle + "defaultWave",
                "w l");
        }
    }
    */

    double defaultMax = -1;
    double defaultMin = 1;
    for (int k = 0; k < defaultWave.size(); k++) {
        double value = defaultWave[k];
        if (value > defaultMax) {
            defaultMax = value;
        }
        if (value < defaultMin) {
            defaultMin = value;
        }
    }
    double defaultAmp = fabs(defaultMax - defaultMin);

    // 基準波形の保存
    if (false) {
        Wave wav;
        wav.CreateWave(defaultWave, samplingSize, 16);
        wav.OutputWave("./output/output_default_wave.wav");
    }

    // 入力の書き換え
    if (false) {
        for (int i = 0; i < cycles.size(); i++) {
            double max = -1, min = 1;
            for (int k = 0; k < cycles[i].length; k++) {
                double value = julius[cycles[i].index + k];
                if (value > max) {
                    max = value;
                }
                if (value < min) {
                    min = value;
                }
            }
            double amp = fabs(max - min);

            // 入力の書き換え
            for (int j = 0; j < cycles[i].length; j++) {
                input.erase(input.begin() + from + cycles[i].index);
            }

            auto wave = LinearInterpolation::convert(defaultWave, cycles[i].length);
            double rate = amp / defaultAmp;
            double diff = max - defaultMax;
            for (int k = 0; k < cycles[i].length; k++) {
                int index = from + cycles[i].index + k;
                input.insert(input.begin() + index, wave[k] * rate - diff);
            }
        }
    }

    // 波形の表示
    if (false) {
        std::vector<double> gnuplot2;
        int margin = 100;
        for (int j = 0; j < julius.size() + margin * 2; j++) {
            gnuplot2.push_back(input[from + j - margin]);
        }
        Gnuplot<double>::Output(gnuplot2, gnuplotSubtitle + "gnuplot2",
            "w l");
    }
}
