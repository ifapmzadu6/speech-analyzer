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
#include "findpeaks.h"

struct UnitWave {
    std::string unit;
    std::string before;
    std::string after;
    std::vector<double> wave;
};

std::vector<UnitWave> getUnitWaves(int samplingSize);
std::vector<std::vector<UnitWave>> getTryphones(
    std::vector<UnitWave> &unitWaves);

// Julius
UnitWave *enumurateJulius(
    std::vector<double> &input, int samplingSize, int from,
    std::vector<std::vector<double>> &splittedDataByJulius,
    std::vector<JuliusResult> &juliusResults, int indexOfJulius);

// Clustering
void showClustering(std::vector<double> &input, int samplingSize);

int main() {

    int samplingSize = 16000;
    std::vector<UnitWave> unitWaves = getUnitWaves(samplingSize);
    std::vector<std::vector<UnitWave>> tryphones = getTryphones(unitWaves);

    for (int i = 0; i < tryphones.size(); i++) {
        std::vector<UnitWave> units = tryphones[i];
        if (units[0].unit != "a" || units.size() < 3) {
            continue;
        }
        std::string title =
            units[0].before + "-" + units[0].unit + "-" + units[0].after;

        std::vector<std::vector<double>> waves;
        for (int j = 0; j < units.size(); j++) {
            auto wave = LinearInterpolation::convert(units[j].wave, 300);
            waves.push_back(wave);
        }

        int defaultIndex = KernelDensityEstimation::IndexOfMaxDensity(waves);
        std::vector<double> bestWave = waves[defaultIndex];
        std::vector<std::vector<double>> org;
        for (int j = 0; j < waves.size(); j++) {
            std::vector<double> vec =
                Util::MiximizeCrossCorrelation(waves[j], bestWave);
            vec = Util::ZerofyFirstAndLast(vec);
            org.push_back(vec);

            //org.push_back(waves[j]);
        }

        // 面積で正則化
        // org = Util::NormalizeSummation(org);
        // org = Util::NormalizeVectors(org);
        if (true) {
            Gnuplot<double>::Output2D(org, title, "w l lc rgb '#E0FF0000'");
        }

        if (false) {
            int countOfCluster = 10;
            int dim = waves[0].size();
            KMeansMethodResult result =
                KMeansMethod::Clustering(org, dim, countOfCluster);
            Gnuplot<double>::Output2D(result.clusters, title, "w l");
        }

        int bestIndex = KernelDensityEstimation::IndexOfMaxDensity(org);
        if (true) {
            Wave wav;
            wav.CreateWave(org[bestIndex], samplingSize, 16);
            wav.OutputWave("./output/" + title + ".wav");
        }
        if (false) {
            Gnuplot<double>::Output(org[bestIndex], title, "w l");
        }
    }

    return 0;
}

std::vector<UnitWave> getUnitWaves(int samplingSize) {
    std::vector<UnitWave> unitWaves;
    for (int h = 1; h <= 199; h++) {
        // 音声データからUnitWaveの取り出し
        auto inputForJulius =
            Util::GetInput("./resource/" + Util::toString(h) + ".wav", 0);
        JuliusImporter juliusImporter("./resource/" + Util::toString(h) +
                                      ".lab");
        auto juliusResults = juliusImporter.getJuliusResults();
        auto splittedDataByJulius = Util::GetSplittedDataByJulius(
            inputForJulius, samplingSize, juliusResults);
        for (int i = 0; i < juliusResults.size(); i++) {
            auto result = juliusResults[i];
            if (((result.unit == "a") || (result.unit == "i") ||
                 (result.unit == "u") || (result.unit == "e") ||
                 (result.unit == "o"))) {
                int from = result.from * samplingSize;
                UnitWave *unitWave =
                    enumurateJulius(inputForJulius, samplingSize, from,
                                    splittedDataByJulius, juliusResults, i);
                if (unitWave != nullptr) {
                    unitWaves.push_back(*unitWave);
                }
            }
        }
    }
    return unitWaves;
}

std::vector<std::vector<UnitWave>> getTryphones(
    std::vector<UnitWave> &unitWaves) {
    // * + 母音 + * で分類
    std::vector<std::vector<UnitWave>> tryphones;
    for (int i = 0; i < unitWaves.size(); i++) {
        UnitWave unitWave = unitWaves[i];
        int indexOfTryphones = -1;
        for (int j = 0; j < tryphones.size(); j++) {
            UnitWave firstUnitWave = tryphones[j][0];
            if (unitWave.unit == firstUnitWave.unit &&
                unitWave.before == firstUnitWave.before &&
                unitWave.after == firstUnitWave.after) {
                indexOfTryphones = j;
            }
        }
        if (indexOfTryphones >= 0) {  // 既存のものがあった場合
            std::vector<UnitWave> tryphone = tryphones[indexOfTryphones];
            tryphone.push_back(unitWave);
            tryphones[indexOfTryphones] = tryphone;
        } else {  // 既存のものがない場合
            std::vector<UnitWave> tryphone;
            tryphone.push_back(unitWave);
            tryphones.push_back(tryphone);
        }
    }
    if (true) {
        std::cout << "Total: " << unitWaves.size() << std::endl;
        for (int i = 0; i < tryphones.size(); i++) {
            std::vector<UnitWave> units = tryphones[i];
            std::cout << units[0].before << "-" << units[0].unit << "-"
                      << units[0].after << ": " << units.size() << std::endl;
        }
    }
    return tryphones;
}

// Julius
UnitWave *enumurateJulius(
    std::vector<double> &input, int samplingSize, int from,
    std::vector<std::vector<double>> &splittedDataByJulius,
    std::vector<JuliusResult> &juliusResults, int indexOfJulius) {
    std::stringstream ss;
    ss << from << " " << juliusResults[indexOfJulius - 1].unit << "-"
       << juliusResults[indexOfJulius].unit << "-"
       << juliusResults[indexOfJulius + 1].unit << "  ";
    std::string gnuplotSubtitle = ss.str();

    std::vector<double> julius = splittedDataByJulius[indexOfJulius];

    std::vector<int> peaks = FindPeaks<double>::finds(julius);
    int firstPeak = 0;
    double minValue = 100;
    for (int i = 0; i < peaks.size(); i++) {
        int range = julius.size() / 5;
        double value = pow(1 - julius[peaks[i]], 2) + pow(peaks[i] / range, 2);
        if (peaks[i] < range && minValue > value) {
            minValue = value;
            firstPeak = peaks[i];
        }
    }
    firstPeak = std::max(firstPeak-50, 0);

    int targetSize = 140;
    int errorSize = 40;
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(julius, samplingSize, firstPeak,
                                               targetSize - errorSize,
                                               targetSize + errorSize);
    if (cycles.size() < 3) {
        return nullptr;
    }
    //std::cout << "cycles.size() -> " << cycles.size() << std::endl;

    // 先頭の波を利用する
    int maxLength = -1;
    for (int j = 0; j < cycles.size(); j++) {
        if (maxLength < cycles[j].length) {
            maxLength = cycles[j].length;
        }
    }
    // 周期切り出し
    std::vector<std::vector<double>> inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        std::vector<double> inputCycle =
            Util::CopyVector(julius, cycles[i].index, cycles[i].length);
        // 波形の長さを揃える
        inputCycle = LinearInterpolation::convert(inputCycle, maxLength);
        // 一定の条件以下では除外する
        // 面積
        double s = 0;
        for (int i=0; i<inputCycle.size(); i++) {
            s += fabs(inputCycle[i]);
        }
        if (s > 20) {
            inputCycles.push_back(inputCycle);
        }
    }
    if (inputCycles.size() == 0) {
        return nullptr;
    }
    if (false) {
        Gnuplot<double>::Output2D(inputCycles, gnuplotSubtitle + "inputCycles",
                                  "w l lc rgb '#E0FF0000'");
    }

    // 一番重心となっているものを見つける
    int bestClusterIndex =
        KernelDensityEstimation::IndexOfMaxDensity(inputCycles);
    std::vector<double> defaultWave = inputCycles[bestClusterIndex];

    // 基準波形の表示
    if (false) {
        Gnuplot<double>::OutputCyclize(defaultWave,
                                       gnuplotSubtitle + "defaultWave", "w l");
    }

    // 基準波形の保存
    if (false) {
        Wave wav;
        wav.CreateWave(defaultWave, samplingSize, 16);
        wav.OutputWave("./output/output_default_wave.wav");
    }

    // 波形の表示
    if (false) {
        std::vector<double> gnuplot2;
        int margin = 100;
        for (int j = 0; j < julius.size() + margin * 2; j++) {
            gnuplot2.push_back(input[from + j - margin]);
        }
        Gnuplot<double>::Output(gnuplot2, gnuplotSubtitle + "gnuplot2", "w l");
    }

    // 基準波形
    UnitWave *unitWave = new UnitWave;
    unitWave->unit = juliusResults[indexOfJulius].unit;
    unitWave->before = juliusResults[indexOfJulius - 1].unit;
    unitWave->after = juliusResults[indexOfJulius + 1].unit;
    unitWave->wave = defaultWave;
    return unitWave;
}

/*
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

            auto wave = LinearInterpolation::convert(defaultWave,
cycles[i].length);
            double rate = amp / defaultAmp;
            double diff = max - defaultMax;
            for (int k = 0; k < cycles[i].length; k++) {
                int index = from + cycles[i].index + k;
                input.insert(input.begin() + index, wave[k] * rate - diff);
            }
        }
    }
*/

// Clustering
void showClustering(std::vector<double> &input, int samplingSize) {
    // 周期を解析
    int targetSize = 140;
    int errorSize = 70;
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(
        input, samplingSize, 0, targetSize - errorSize, targetSize + errorSize);
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
    std::vector<std::vector<double>> inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        std::vector<double> inputCycle =
            Util::CopyVector(input, cycles[i].index, cycles[i].length);
        // 波形の長さを揃える
        inputCycle = LinearInterpolation::convert(inputCycle, maxLength);
        inputCycles.push_back(inputCycle);
    }
    // inputCycles = Util::NormalizeSummation(inputCycles);
    Gnuplot<double>::Output2D(inputCycles, "inputCycles",
                              "w l lc rgb '#E0FF0000'");

    // クラスタリング
    int countOfCluster = 5;
    // int dim = targetSize + padding;
    int dim = inputCycles[0].size();
    KMeansMethodResult result =
        KMeansMethod::Clustering(inputCycles, dim, countOfCluster);
    Gnuplot<double>::Output2D(result.clusters, "result.clusters", "w l");

    // どのクラスターが正解に近いかを表示
    if (false) {
        std::vector<std::vector<double>> clusters = result.clusters;
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
        Gnuplot<int>::Output(result.indexOfCluster, "result.indexOfCluster",
                             "w p pt 3");
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
