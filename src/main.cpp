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
#include "lanczos_resampling.h"
#include "kernel_density_estimation.h"
#include "util.h"
#include "findpeaks.h"
#include "fir_filter.h"

struct UnitWave {
    std::string unit;
    std::string before;
    std::string after;
    std::vector<double> wave;
};

std::vector<UnitWave> getUnitWaves(int samplingSize, int extendedSamplingSize);
std::vector<std::vector<UnitWave>> getTryphones(std::vector<UnitWave> &unitWaves);
std::vector<std::vector<std::vector<double>>> getTryphoneWaves(std::vector<std::vector<UnitWave>> &tryphones);
std::vector<std::vector<double>> getBestTryphoneWaves(std::vector<std::vector<std::vector<double>>> &tryphoneWaves);

void display(int samplingSize, std::vector<std::vector<UnitWave>> tryphones, std::vector<std::vector<std::vector<double>>> tryphoneWaves);

int main() {
    int samplingSize = 16000;
    int extendedSamplingSize = 16000 * 8;
    std::vector<UnitWave> unitWaves = getUnitWaves(samplingSize, extendedSamplingSize);
    std::vector<std::vector<UnitWave>> tryphones = getTryphones(unitWaves);
    std::vector<std::vector<std::vector<double>>> tryphoneWaves = getTryphoneWaves(tryphones);
    std::vector<std::vector<double>> bestTryphoneWaves = getBestTryphoneWaves(tryphoneWaves);

    display(extendedSamplingSize, tryphones, tryphoneWaves);

    // ここから音声書き換え開始
    auto inputForJulius = Util::GetInput("./resource/0.wav", 0);
    JuliusImporter juliusImporter("./resource/0.lab");
    auto juliusResults = juliusImporter.getJuliusResults();
    auto splittedData = Util::GetSplittedDataByJulius(inputForJulius, samplingSize, juliusResults);

    for (int i = juliusResults.size() - 2; i >= 1; i--) {
        std::string unit = juliusResults[i].unit;
        if (!(unit == "a" || unit == "i" || unit == "u" || unit == "e" || unit == "o")) {
            continue;
        }
        std::string before = juliusResults[i - 1].unit;
        std::string after = juliusResults[i + 1].unit;
        std::vector<double> wave = splittedData[i];
        int from = samplingSize * juliusResults[i].from;
        int length = samplingSize * (juliusResults[i].to - juliusResults[i].from);
        if (false) {
            Gnuplot<double>::Output(wave, before + "-" + unit + "-" + after, "w l");
        }
        std::cout << before << "-" << unit << "-" << after << " : " << wave.size() << std::endl;

        // 一致しているトライフォンモデルを探す
        int indexOfTryphones = -1;
        for (int j = 0; j < tryphones.size(); j++) {
            std::vector<UnitWave> tryphone = tryphones[j];
            UnitWave unitWave = tryphone[0];
            if (unitWave.unit == unit && unitWave.before == before && unitWave.after == after) {
                indexOfTryphones = j;
                break;
            }
        }
        if (indexOfTryphones < 0) {
            std::cout << "cannot find tryphone!" << std::endl;
            continue;
        }
        std::vector<double> bestCycle = bestTryphoneWaves[indexOfTryphones];
        if (wave.size() < bestCycle.size()) {
            std::cout << "Size of wave is too small!" << std::endl;
            continue;
        }
        if (false) {
            Gnuplot<double>::Output(bestCycle, "[bestCycle]" + before + "-" + unit + "-" + after, "w l");
        }
        std::cout << "indexOfTryphones -> " << indexOfTryphones << std::endl;

        std::vector<double> crossCorrelation = Util::GetCrossCorrelation(wave, bestCycle);
        if (false) {
            Gnuplot<double>::Output(crossCorrelation, "[crossCorrelation]" + before + "-" + unit + "-" + after, "w l");
        }

        std::vector<int> peaks = FindPeaks<double>::finds(crossCorrelation);
        int firstPeak = 0;
        double minValue = 100;
        for (int j = 0; j < peaks.size(); j++) {
            int range = crossCorrelation.size() / 5;
            double value = pow(1 - crossCorrelation[peaks[j]], 2) + pow(peaks[j] / range, 2);
            if (peaks[i] < range && minValue > value) {
                minValue = value;
                firstPeak = peaks[j];
            }
        }
        // firstPeak = std::max(firstPeak-100, 0);
        firstPeak = 0;

        // TODO: Remove
        if (false) {
            int index = 0;
            std::vector<double> vec;
            for (int j = 0; j < crossCorrelation.size(); j++) {
                if (index < peaks.size() && peaks[index] == j) {
                    vec.push_back(crossCorrelation[j]);
                    index++;
                } else {
                    vec.push_back(0);
                }
            }
            std::vector<std::vector<double>> tmp;
            tmp.push_back(crossCorrelation);
            tmp.push_back(vec);
            Gnuplot<double>::Output2D(tmp, "[crossCorrelation+peak]" + before + "-" + unit + "-" + after, "w l");
        }

        //int targetSize = 130 * double(extendedSamplingSize) / double(samplingSize);
        //int errorSize = 30 * double(extendedSamplingSize) / double(samplingSize);
        int targetSize = 130;
        int errorSize = 30;
        std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(wave, firstPeak, targetSize - errorSize, targetSize + errorSize);
        std::cout << "cycles.size -> " << cycles.size() << std::endl;
        if (cycles.size() == 0) {
            continue;
        }

        std::vector<std::vector<double>> gnuplotForWaves;
        std::vector<double> raw = Util::CopyVector(inputForJulius, from, length);
        gnuplotForWaves.push_back(raw);

        // 書き換え
        std::cout << "from -> " << from << std::endl;
        // 周期切り出し
        std::vector<std::vector<double>> inputCycles;
        for (int j = cycles.size() - 1; j >= 0; j--) {
            std::vector<double> inputCycle = Util::CopyVector(wave, cycles[j].index, cycles[j].length);
            // 波形の長さを揃える
            std::vector<double> insertCycle = LanczosResampling::convert(bestCycle, cycles[j].length);
            // 一定の条件以下では除外する
            // 面積
            double s = 0;
            for (int k = 0; k < inputCycle.size(); k++) {
                s += fabs(inputCycle[k]);
            }
            if (s <= 20) {
                continue;
            }

            // 波長を合わせる
            {
                double inputMax = -1;
                double inputMin = 1;
                for (int k = 0; k < inputCycle.size(); k++) {
                    double value = inputCycle[k];
                    if (inputMax < value) {
                        inputMax = value;
                    }
                    if (inputMin > value) {
                        inputMin = value;
                    }
                }
                double insertMax = -1;
                double insertMin = 1;
                for (int k = 0; k < insertCycle.size(); k++) {
                    double value = insertCycle[k];
                    if (insertMax < value) {
                        insertMax = value;
                    }
                    if (insertMin > value) {
                        insertMin = value;
                    }
                }
                double rate = (insertMax - insertMin) / (inputMax - inputMin);
                for (int k = 0; k < insertCycle.size(); k++) {
                    insertCycle[k] /= rate;
                }

                double s = 0;
                for (int k = 0; k < insertCycle.size(); k++) {
                    s += insertCycle[k] * inputCycle[k];
                }

                // if (s < 0) {
                //    std::cout << "reverse!" << std::endl;
                insertCycle = Util::MiximizeCrossCorrelation(insertCycle, inputCycle);
                insertCycle = Util::ZerofyFirstAndLast(insertCycle);
                //}

                double max = -1;
                for (int k = 0; k < insertCycle.size(); k++) {
                    double value = insertCycle[k];
                    if (max < value) {
                        max = value;
                    }
                }
                double diff = inputMax - max;
                for (int k = 0; k < insertCycle.size(); k++) {
                    insertCycle[k] += diff;
                }
            }

            std::cout << "length -> " << cycles[j].length << std::endl;
            // 除去
            for (int k = 0; k < inputCycle.size(); k++) {
                inputForJulius.erase(inputForJulius.begin() + from + cycles[j].index);
            }

            // 挿入
            for (int k = insertCycle.size() - 1; k >= 0; k--) {
                inputForJulius.insert(inputForJulius.begin() + from + cycles[j].index, insertCycle[k]);
            }

            if (false) {
                Gnuplot<double>::Output(inputCycle, before + "-" + unit + "-" + after, "w l");
            }
        }

        if (false) {
            std::vector<double> modified = Util::CopyVector(inputForJulius, from, length);
            // Gnuplot<double>::Output(modified, before + "-" + unit + "-" + after, "w l");

            gnuplotForWaves.push_back(modified);
            std::string filename = "./pdf/image" + Util::toString(i) + ".pdf";
            Gnuplot<double>::Output2D(gnuplotForWaves, before + "-" + unit + "-" + after, "w l", nullptr, true, filename.c_str());
        }
    }

    // 音声ファイルとして保存
    if (true) {
        Wave wav;
        wav.CreateWave(inputForJulius, samplingSize, 16);
        // wav.CreateWave(filtered, samplingSize, 16);
        wav.OutputWave("./output/output.wav");
    }

    return 0;
}

void display(int samplingSize, std::vector<std::vector<UnitWave>> tryphones, std::vector<std::vector<std::vector<double>>> tryphoneWaves) {
    for (int i = 0; i < tryphones.size(); i++) {
        std::vector<UnitWave> units = tryphones[i];
        std::vector<std::vector<double>> waves = tryphoneWaves[i];
        if (units[0].unit != "a" || units.size() < 3) {
            continue;
        }
        std::string title = units[0].before + "-" + units[0].unit + "-" + units[0].after;

        // 表示
        if (true) {
            int bestIndex = KernelDensityEstimation::IndexOfMaxDensity(waves);
            std::vector<std::vector<double>> vecs;
            for (int j = 0; j < waves.size(); j++) {
                vecs.push_back(waves[j]);
            }
            for (int j = 0; j < 10; j++) {
                vecs.push_back(waves[bestIndex]);  // 線を濃くする
            }
            Gnuplot<double>::Output2D(vecs, title, "w l lc rgb '#E0FF0000'");

            // std::string pdfname = "./pdf/" + title + ".pdf";
            // Gnuplot<double>::Output2D(vecs, title, "w l lc rgb '#E0FF0000'", nullptr, true, pdfname.c_str());
        }

        // クラスタリング
        if (false) {
            int countOfCluster = 10;
            int dim = waves[0].size();
            KMeansMethodResult result = KMeansMethod::Clustering(waves, dim, countOfCluster);
            Gnuplot<double>::Output2D(result.clusters, title, "w l");
        }

        // 音声ファイルとして保存
        if (false) {
            int bestIndex = KernelDensityEstimation::IndexOfMaxDensity(waves);
            Wave wav;
            wav.CreateWave(waves[bestIndex], samplingSize, 16);
            wav.OutputWave("./output/" + title + ".wav");
        }
    }
}

std::vector<std::vector<double>> getBestTryphoneWaves(std::vector<std::vector<std::vector<double>>> &tryphoneWaves) {
    std::vector<std::vector<std::vector<double>>> convertedTryphoneWaves;
    for (int i = 0; i < tryphoneWaves.size(); i++) {
        std::vector<std::vector<double>> vecs;
        for (int j = 0; j < tryphoneWaves[i].size(); j++) {
            vecs.push_back(LanczosResampling::convert(tryphoneWaves[i][j], 600));
        }
        convertedTryphoneWaves.push_back(vecs);
    }

    std::vector<std::vector<double>> bestTryphoneWaves;
    for (int i = 0; i < convertedTryphoneWaves.size(); i++) {
        std::vector<std::vector<double>> tryphoneWave = convertedTryphoneWaves[i];
        int indexOfMaxDeisity = KernelDensityEstimation::IndexOfMaxDensity(tryphoneWave);

        bestTryphoneWaves.push_back(tryphoneWave[indexOfMaxDeisity]);
    }
    return bestTryphoneWaves;
}

std::vector<std::vector<std::vector<double>>> getTryphoneWaves(std::vector<std::vector<UnitWave>> &tryphones) {
    std::vector<std::vector<std::vector<double>>> tryphoneWaves;
    for (int i = 0; i < tryphones.size(); i++) {
        std::vector<UnitWave> units = tryphones[i];

        std::vector<std::vector<double>> tmpwaves;
        for (int j = 0; j < units.size(); j++) {
            auto wave = LanczosResampling::convert(units[j].wave, 200);
            tmpwaves.push_back(wave);
        }

        int defaultIndex = KernelDensityEstimation::IndexOfMaxDensity(tmpwaves);
        std::vector<double> bestWave = tmpwaves[defaultIndex];
        std::vector<std::vector<double>> waves;
        for (int j = 0; j < tmpwaves.size(); j++) {
            std::vector<double> vec = Util::MiximizeCrossCorrelation(tmpwaves[j], bestWave);
            vec = Util::ZerofyFirstAndLast(vec);
            waves.push_back(vec);
        }
        // 面積で正則化
        // waves = Util::NormalizeSummation(waves);
        waves = Util::NormalizeVectors(waves);
        tryphoneWaves.push_back(waves);
    }
    return tryphoneWaves;
}

std::vector<std::vector<UnitWave>> getTryphones(std::vector<UnitWave> &unitWaves) {
    // * + 母音 + * で分類
    std::vector<std::vector<UnitWave>> tryphones;
    for (int i = 0; i < unitWaves.size(); i++) {
        UnitWave unitWave = unitWaves[i];
        int indexOfTryphones = -1;
        for (int j = 0; j < tryphones.size(); j++) {
            UnitWave firstUnitWave = tryphones[j][0];
            if (unitWave.unit == firstUnitWave.unit && unitWave.before == firstUnitWave.before && unitWave.after == firstUnitWave.after) {
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
        /*
        for (int i = 0; i < tryphones.size(); i++) {
            std::vector<UnitWave> units = tryphones[i];
            std::cout << units[0].before << "-" << units[0].unit << "-" << units[0].after << ": " << units.size() << std::endl;
        }
        */
    }
    return tryphones;
}

std::vector<UnitWave> getUnitWaves(int samplingSize, int extendedSamplingSize) {
    std::vector<UnitWave> unitWaves;

    for (int h = 0; h <= 199; h++) {
        auto inputForJulius = Util::GetInput("./resource/" + Util::toString(h) + ".wav", 0);
        JuliusImporter juliusImporter("./resource/" + Util::toString(h) + ".lab");
        auto juliusResults = juliusImporter.getJuliusResults();

        auto splittedData = Util::GetSplittedDataByJulius(inputForJulius, samplingSize, juliusResults);
        for (int i=0; i<splittedData.size(); i++) {
            double upsamplingRatio = double(extendedSamplingSize) / double(samplingSize);
            splittedData[i] = LanczosResampling::convert(splittedData[i], splittedData[i].size() * upsamplingRatio);
        }

        for (int i = 1; i + 1 < juliusResults.size(); i++) {
            std::vector<double> data = splittedData[i];

            int targetSize = 130 * double(extendedSamplingSize) / double(samplingSize);
            int errorSize = 30 * double(extendedSamplingSize) / double(samplingSize);
            std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(data, 0, targetSize - errorSize, targetSize + errorSize);
            if (cycles.size() <= 2) {
                continue;
            }

            // 周期切り出し
            std::vector<std::vector<double>> inputCycles;
            for (int j = 0; j < cycles.size(); j++) {
                std::vector<double> inputCycle = Util::CopyVector(data, cycles[j].index, cycles[j].length);
                // 面積、一定の条件以下では除外する
                double s = 0;
                for (int k = 0; k < inputCycle.size(); k++) {
                    s += fabs(inputCycle[k]);
                }
                if (s > 15) {
                    inputCycles.push_back(inputCycle);
                }
            }
            if (inputCycles.size() <= 2) {
                continue;
            }

            for (int j = 0; j < inputCycles.size(); j++) {
                // 基準波形
                UnitWave *unitWave = new UnitWave;
                unitWave->unit = juliusResults[i].unit;
                unitWave->before = juliusResults[i - 1].unit;
                unitWave->after = juliusResults[i + 1].unit;
                unitWave->wave = inputCycles[j];

                unitWaves.push_back(*unitWave);
            }
        }
    }
    if (true) {
        int a = 0;
        int i = 0;
        int u = 0;
        int e = 0;
        int o = 0;
        for (int j = 0; j < unitWaves.size(); j++) {
            if (unitWaves[j].unit == "a") {
                a++;
            } else if (unitWaves[j].unit == "i") {
                i++;
            } else if (unitWaves[j].unit == "u") {
                u++;
            } else if (unitWaves[j].unit == "e") {
                e++;
            } else if (unitWaves[j].unit == "o") {
                o++;
            }
        }
        std::cout << "Total: " << unitWaves.size() << std::endl;
        std::cout << "a: " << a << std::endl;
        std::cout << "i: " << i << std::endl;
        std::cout << "u: " << u << std::endl;
        std::cout << "e: " << e << std::endl;
        std::cout << "o: " << o << std::endl;
    }
    return unitWaves;
}
