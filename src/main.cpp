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

std::vector<UnitWave> getUnitWaves(int samplingSize);
std::vector<std::vector<UnitWave>> getTryphones(std::vector<UnitWave> &unitWaves);
std::vector<std::vector<std::vector<double>>> getTryphoneWaves(std::vector<std::vector<UnitWave>> &tryphones);
std::vector<std::vector<double>> getBestTryphoneWaves(std::vector<std::vector<std::vector<double>>> &tryphoneWaves);

void display(int samplingSize, std::vector<std::vector<UnitWave>> tryphones, std::vector<std::vector<std::vector<double>>> tryphoneWaves);

int main() {
    int samplingSize = 44100;
    std::vector<UnitWave> unitWaves = getUnitWaves(samplingSize);
    std::vector<std::vector<UnitWave>> tryphones = getTryphones(unitWaves);
    std::vector<std::vector<std::vector<double>>> tryphoneWaves = getTryphoneWaves(tryphones);
    std::vector<std::vector<double>> bestTryphoneWaves = getBestTryphoneWaves(tryphoneWaves);

    display(samplingSize, tryphones, tryphoneWaves);


    std::random_device random;     // 非決定的な乱数生成器
    std::mt19937 mt(random());


    // ここから音声書き換え開始

    for (int x = 0; x < 20; x++) {


        auto inputForJulius = Util::GetInput("./resource/" + Util::toString(x) + ".wav", 0);
        JuliusImporter juliusImporter("./resource/" + Util::toString(x) + ".lab");
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

            int targetSize = 360;
            int errorSize = 100;
            std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(wave, firstPeak, targetSize - errorSize, targetSize + errorSize);
            std::cout << "cycles.size -> " << cycles.size() << std::endl;
            if (cycles.size() == 0) {
                continue;
            }

            std::vector<std::vector<double>> gnuplotForWaves;
            std::vector<double> raw = Util::CopyVector(inputForJulius, from, length);
            gnuplotForWaves.push_back(raw);

            // 周期の平均を求める
            int ave = 0;
            for (int i=0; i<cycles.size(); i++) {
                ave += cycles[i].length;
            }
            ave /= cycles.size();

            // 書き換え
            std::cout << "from -> " << from << std::endl;
            // 周期切り出し
            std::vector<std::vector<double>> inputCycles;
            for (int j = cycles.size() - 1; j >= 0; j--) {
                std::vector<double> inputCycle = Util::CopyVector(wave, cycles[j].index, cycles[j].length);
                // 波形の長さを揃える
                std::vector<double> insertCycle = LanczosResampling::convert(bestCycle, inputCycle.size());
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

                    /*
                    double s = 0;
                    for (int k = 0; k < insertCycle.size(); k++) {
                        s += insertCycle[k] * inputCycle[k];
                    }
                    */

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


                double a[] = {-0.351122, -0.111853, -0.255851, -0.0581233, -0.234467, -0.265311, -0.118256, -0.097458, -0.0786425, -0.0296716, -0.19441, -0.212395, -0.28919, -0.180009, -0.239616, -0.378104, -0.193956, -0.258381, -0.18245, -0.235317, -0.253358, -0.0940457, -0.179313, -0.0888235, -0.257358, -0.259781, -0.0530198, -0.207192, 0.00645033, -0.308445, 0.0343763, -0.171692, -0.251578, -0.19813, -0.23865, -0.11978, -0.158947, -0.325924, -0.230247, -0.321325, -0.172056, -0.331152, -0.262191, -0.301628, -0.248162, -0.236645, -0.141336, -0.230764, -0.165187, -0.2698, -0.138824, -0.172824, -0.219195, -0.0634493, -0.169656, 0.00490209, -0.226388, -0.109116, -0.190443, -0.142988, -0.105298, -0.109618, -0.0833494, -0.0389769, -0.124257, -0.0879722, -0.133807, -0.203257, -0.217476, -0.307368, -0.209818, -0.314736, -0.217952, -0.206833, -0.160318, -0.228663, -0.240829, -0.283528, -0.1901, -0.204024, -0.22001, -0.243563, -0.128181, -0.248743, 0.0149341, -0.234582, -0.0241282, -0.264342, 0.0901691, -0.134041, -0.200728, -0.125252, -0.183426, -0.270162, -0.227446, -0.387895, -0.254352, -0.131365, -0.363331, -0.234734, -0.19855, -0.0295338, 0.085759, -0.20984, -0.0398378, -0.233351, -0.181343, -0.00396053, -0.360552, -0.0163003, -0.118958, 0.108509, -0.10485, -0.0717384, -0.132603, -0.035308, -0.2979, -0.144735, -0.325056, -0.229785, -0.176667, -0.197842, -0.275708, -0.152403, -0.285847, -0.00952997, -0.2384, -0.0582076, -0.238453, -0.232807, -0.076291, -0.233017, -0.0614082, -0.0744728, -0.127865, -0.114148, -0.207964, -0.171666, -0.131935, -0.150799, -0.186121, -0.141336, -0.104849, -0.0461638, -0.178339, -0.106144, -0.234392, -0.202757, -0.365833, -0.23141, -0.288851, -0.388146, -0.276504, -0.393135, -0.243498, -0.245083, -0.350028, -0.254627, -0.3173, -0.215324, -0.191164, -0.26149, -0.26403, -0.162859, -0.142302, 0.0230437, -0.306165, -0.15488, -0.214674, -0.169888, -0.0938774, -0.281181, -0.205653, -0.142605, -0.147995, -0.043449, -0.227592, -0.0439917, -0.181894, -0.253826, -0.183701, -0.242394, -0.222109, -0.216682, -0.230606, -0.213059, -0.289293, -0.320914, -0.306576, -0.213125, -0.2979, -0.149826, -0.270563, -0.138042, -0.00377881, -0.212219, -0.14953, -0.220173, -0.144965, -0.146879, -0.199903, -0.172916, -0.139038, -0.128784, -0.150463, -0.110487, -0.20253, -0.157569, -0.235142, -0.150718, -0.296612, -0.231053, -0.224748, -0.291044, -0.206432, -0.309167, -0.297006, -0.211939, -0.273936, -0.285848, -0.30753, -0.251837, -0.191237, -0.183405, -0.229802};
                double i[] = {0.144601, 0.0551218, 0.0354765, -0.104412, 0.0717517, -0.0020455, 0.0569961, -0.139327, -0.0318074, -0.0335677, -0.256689, 0.140048, -0.271064, -0.0977146, -0.120591, -0.162995, -0.28635, -0.191594, -0.225654, -0.118685, -0.0410557, -0.220764, -0.146141, -0.109102, -0.0237362, -0.0840189, 0.0946811, -0.0212468, -0.0695815, 0.00421758, 0.00682199, -0.11263, 0.283529, -0.0666158, 0.211734, 0.0260132, 0.135318, 0.155965, 0.153496, 0.193822, 0.236154, -0.0117161, 0.107801, 0.172713, -0.0523672, 0.140819, -0.192481, 0.0594975, -0.192788, 0.0279684, 0.0157044, -0.0759703, -0.140967, -0.151277, -0.262858, 0.0628695, -0.347345, -0.0430721, -0.291233, -0.242949, -0.225068, -0.350439, -0.176968, -0.301258, -0.280538, -0.203682, -0.327777, -0.106712, -0.237097, -0.320208, -0.26714, -0.334148, -0.218371, -0.240442, -0.291456, -0.256444, -0.3098, -0.264186, -0.197664, -0.237774, -0.22439, -0.340711, -0.237452, -0.248414, -0.216966, -0.25576, -0.180976, -0.217862, -0.222532, -0.312698, -0.168289, -0.166494, -0.156654, -0.114631, -0.217367, -0.146303, 0.052171, -0.0471155, 0.0630926, -0.20512, 0.126916, 0.133691, 0.0948766, 0.0674181, 0.276133, -0.0481521, -0.0722013, -0.095891, -0.0965751, -0.141749, -0.00727026, 0.0436066, -0.0402527, -0.0216684, -0.126096, -0.139976, -0.0958226, -0.26495, -0.116424, -0.217331, -0.111796, -0.109069, -0.12207, -0.00795441, 0.0172078, -0.0224595, 0.0190125, -0.0588333, -0.0612939, -0.136082, 0.0860381, -0.0574457, -0.189385, -0.0685867, -0.271774, 0.066467, 0.0966409, -0.01607, -0.0474911, -0.00118943, -0.161205, -0.0471245, 0.20702, 0.204725, 0.0182526, -0.0505942, 0.0117525, 0.0683617, -0.00876141, 0.0858903, -0.324508, -0.0983965, 0.132218, -0.00551106, 0.145529, -0.115311, -0.00697318, 0.0893533, -0.160129, 0.175371, -0.253645, -0.224172, -0.0980131, -0.208768, -0.0433265, -0.286695, -0.184526, -0.018873, -0.180905, 0.174988, -0.157432, -0.0465981, -0.214561, -0.15773, -0.142633, -0.216546, -0.215, -0.261467, -0.178403, -0.258084, -0.185238, -0.304068, -0.115423, -0.280094, -0.213439, -0.160114, -0.102729, -0.223109, -0.201086, -0.217631, -0.100752, -0.0943138, -0.0425807, -0.134356, 0.0408204, -0.10436, 0.140515, 0.359818, 0.298745, -0.0613339, 0.11434, 0.0506183, 0.194575, -0.122763, 0.0965726, -0.115441, -0.0590018, -0.00812092, -0.185979, -0.0968364, -0.288052, -0.219617, -0.162576, -0.10783, -0.167023, -0.178208, -0.0671541, -0.164732, -0.109481, -0.206205, -0.12819, -0.226919, -0.129596, -0.0940416, -0.200412, -0.107547, -0.214327, -0.108791, -0.214156, -0.0937166, -0.19046, -0.0320172, -0.0330259, -0.0446938, -0.130586, 0.0144222, -0.188927, 0.0685936, -0.056888, 0.0749018, -0.022599, -0.0745328, 0.13351, 0.11206, -0.0843648, 0.108272, -0.161937, 0.0412202, -0.15388, -0.0307452, -0.0727632, -0.139066, -0.0599673, -0.0874494, -0.236046, 0.0403215, -0.153359, 0.0122511, -0.0952161, 0.117852, -0.107721, -0.0975832, -0.126095, -0.137658, -0.181786, 0.0186184, -0.102061, -0.106867, -0.17584, -0.236808, -0.140871, -0.0599265, -0.0935283, -0.076688, -0.198651, -0.00132937, -0.162428, 0.0270801, -0.0612105, -0.113221, -0.0912328, 0.0451351, 0.0199519, 0.0130363, 0.0232189, -0.154692, 0.00573088, -0.0850369, 0.00169659, -0.183365, -0.073653, -0.281415, 0.0167341, -0.279289, -0.0416398, -0.228606, -0.186883, -0.247837, -0.168914, -0.289319, -0.165526, -0.294641, -0.233902, -0.270914, -0.321212, -0.197716, -0.296659, -0.25559, -0.283845, -0.383534, -0.133095, -0.33042, -0.29853, -0.36649, -0.422724, -0.319012, -0.358625, -0.34209, -0.35747, -0.445288, -0.321847, -0.403012, -0.327652, -0.406034, -0.46049, -0.328356, -0.440356, -0.397599, -0.408127, -0.464127, -0.364838, -0.462508};
                double u[] = {-0.372473, -0.295404, -0.265264, -0.25616, -0.332883, -0.273496, -0.378704, -0.299408, -0.355215, -0.224018, -0.3585, -0.128594, -0.384237, -0.332636, -0.326952, -0.30299, -0.286543, -0.312375, -0.322633, -0.316802, -0.31325, -0.2861, -0.257471, -0.0488403, -0.26784, 0.0204305, -0.195554, -0.151676, -0.169235, -0.20296, -0.135802, -0.201285, -0.0948788, -0.0419771, -0.0945302, -0.0525356, -0.114481, -0.238957, -0.24041, -0.174698, -0.193869, -0.266395, -0.254576, -0.283686, -0.263987, -0.22602, -0.206047, -0.0684163, -0.275427, 0.0463078, -0.145668, 0.0143795, -0.129863, -0.250745, -0.0300957, -0.143693, 0.117242, -0.344431, -0.0283936, -0.189577, 0.0467256, -0.265539, -0.0231729, -0.385701, -0.126348, -0.347333, -0.0825768, -0.348987, -0.0531523, -0.155078, -0.0581678, -0.213781, -0.144959, -0.267707, -0.0978331, -0.213644, 0.09794, -0.366037, -0.179469, -0.10011, -0.123353, -0.0579844, -0.261532, 0.0383644, -0.339219, -0.0357845, -0.342945, -0.12287, -0.272435, 0.231754, -0.146995, -0.0884422, -0.115143, -0.0912606, -0.207836, -0.124498, -0.210271, -0.0310941, -0.1657, -0.17239, -0.230993, -0.0385165, -0.101643, -0.251816, -0.0209958, -0.207027, 0.0208141, -0.186103, -0.147298, -0.151818, -0.11467, -0.230264, 0.0291192, -0.295825, -0.148732, -0.197714, -0.204856, -0.0555243, -0.0168781, -0.209104, -0.0953728, -0.223313, -0.00632025, -0.174638, -0.163752, -0.245621, -0.151972, -0.0747341, -0.0689993, -0.0980913, -0.182255, -0.145636, -0.0476839, -0.251059, -0.244606, -0.221807, -0.141665, -0.130081, -0.136187, -0.173889, -0.122595, 0.0817107, -0.122806, -0.0477299, -0.158701, -0.121228, 0.0233145, 0.000475164, -0.0222147, -0.232781, -0.186019, -0.219682, -0.1116, -0.37124, 0.0763921, -0.209511, -0.101286, -0.105975, -0.150562, -0.132734, -0.22316, -0.102964, -0.0294409, -0.246015, -0.11423, -0.216292, -0.0453664, -0.119708, -0.154004, -0.0881876, -0.110922, -0.0106073, -0.27728, 0.00812785, -0.28703, -0.142409, -0.216009, -0.036573, -0.263487, -0.11324, -0.154516, -0.0956096, -0.0668644, -0.108096, -0.0982421, -0.126836, -0.135642, -0.0675528, -0.0923917, -0.1637, -0.151136, -0.0799278, 0.127124, -0.266855, -0.171428, -0.160728, -0.116957, -0.319053, 0.0964053, -0.140521, -0.132629, -0.117714, -0.241936, -0.15416, -0.136494, -0.1245, 0.0669069, -0.0912081, -0.14339, -0.12346, -0.0188548, -0.0488223, -0.258351, -0.0175104, -0.277813, -0.205554, -0.168484, -0.276403, -0.0788172, -0.0516944, -0.21233, -0.124877, -0.249952, -0.237578, -0.164039, -0.112526, -0.218079, -0.135092, -0.184257, -0.236364, -0.0377485, -0.104703, 0.0990661, -0.117398, 0.0585376, -0.329977, -0.0985398, -0.294961, -0.154064, -0.324728, -0.0430928, -0.2962, -0.138387, -0.180901, -0.0195373, -0.265272, -0.096444, -0.319068, -0.0104445, -0.210374, -0.247524, -0.268135, -0.214338, -0.0422527, -0.00608879, 0.12053, -0.176805, -0.029793, -0.197678, -0.158624, -0.318667, -0.00897707, -0.319116, -0.072034, -0.234186, -0.0411993};
                double e[] = {-0.110681, -0.0613764, -0.316519, -0.421981, -0.490132, -0.159733, 0.141058, -0.0548383, 0.178977, -0.280747, -0.038184, -0.370446, 0.0627192, -0.355091, -0.0824631, 0.101037, -0.412155, 0.0504217, -0.328929, 0.109292, 0.275736, -0.010862, -0.107419, -0.195303, -0.289756, -0.227037, -0.345945, -0.227534, 0.270617, -0.0721199, 0.0498688, -0.261515, 0.133063, -0.342254, -0.193183, 0.194153, -0.274395, 0.134725, -0.130041, 0.0995793, -0.206136, -0.399863, -0.0814013, -0.332694, -0.168625, 0.0118198, 0.0710519, 0.049387, -0.0351365, -0.0520416, 0.0452648, -0.339911, -0.00848884, -0.353211, 0.0937723, 0.0138805, -0.0730571, 0.0253839, -0.0794645, -0.102664, -0.0314603, -0.247014, -0.0244803, -0.154601, 0.218021, 0.0695379, -0.106319, 0.153452, 0.0184131, 0.163053, 0.363918, 0.138185, 0.24991, 0.254302, 0.0164452, 0.222584, -0.156355, 0.199821, -0.17074, 0.233556, 0.206137, 0.089034, -0.0213618, 0.111545, 0.104621, 0.240989, 0.114825, 0.229752, 0.318389, 0.271549, 0.378342, 0.15928, 0.370989, -0.0352908, 0.313266, -0.119547, 0.340977, -0.0950637, 0.152587, -0.0602568, -0.103829, -0.0764375, -0.149736, -0.204323, 0.00392312, -0.25907, -0.0403808, -0.171415, -0.154025, 0.126868, -0.161985, 0.162129, 0.105911, -0.0335457, 0.202505, 0.0862286, 0.104934, 0.00130558, 0.0233895, 0.00734697, -0.209424, 0.0660721, -0.0898558, -0.111085, 0.0786071, -0.029341, 0.229793, -0.0644053, -0.000223183, 0.202716, -0.0161783, 0.337219, -0.251199, -0.0344054, -0.284755, 0.0976651, -0.222943, 0.0694657, -0.369253, -0.0689775, -0.0101144, -0.140527, 0.0512501, -0.365501, -0.170109, -0.254043, -0.207782, -0.119681, -0.428863, -0.198053, -0.251589, 0.168846, -0.149789, -0.43568, 0.0820999, -0.247284, -0.252518, -0.141571, -0.402717, -0.0949592};
                double o[] = {-0.0596446, -0.170213, 0.234955, -0.108173, 0.236049, -0.117841, 0.088658, -0.108031, -0.100142, 0.103137, -0.0365104, 0.105577, -0.257629, 0.203164, -0.0989459, -0.147994, -0.0500975, -0.014803, 0.229153, -0.0759565, 0.131786, 0.0491561, 0.0271128, 0.10985, -0.147008, -0.0586265, -0.102126, -0.212728, -0.214012, 0.0934174, -0.166335, 0.118628, -0.0916544, 0.156992, -0.234261, -0.0496002, -0.228184, 0.143746, -0.225771, 0.0312469, -0.132252, 0.153151, -0.138619, -0.0358514, -0.0280577, -0.033698, 0.0729046, 0.0924664, -0.084436, 0.238553, 0.0931223, 0.163556, -0.134803, 0.0164674, -0.0800958, 0.066747, -0.0569999, 0.100945, -0.125866, -0.156725, -0.130035, -0.0285048, -0.0785993, -0.0633429, -0.107329, -0.100704, -0.178823, -0.222479, -0.161586, -0.137719, -0.216677, -0.226952, -0.108609, -0.17115, -0.29087, -0.0903399, -0.059071, -0.0945675, -0.200302, -0.0626432, -0.149944, -0.0311541, -0.0687541, -0.231255, -0.145197, -0.107491, -0.0292965, 0.000671073, -0.104734, -0.144026, -0.0752135, -0.260302, -0.124474, 0.000399217, -0.183649, -0.217014, -0.219762, -0.205942, 0.0156814, -0.143914, 0.0928507, -0.229781, -0.0714369, -0.366635, 0.0317738, -0.145627, -0.0275653, -0.148525, -0.10582, 0.00642719, -0.0795231, 0.075867, -0.164331, -0.0992131, -0.218123, -0.139473, -0.317017, -0.058996, 0.00183258, -0.193678, -0.114899, -0.169145, -0.0404492, -0.0110642, -0.104454, -0.238597, -0.0988602, -0.241091, -0.0907257, -0.0590191, -0.0469102, -0.168672, -0.0237575, 0.03582, -0.226171, -0.0414167, -0.159451, -0.0968202, -0.136537, 0.12494, 0.124226, -0.178133, -0.110029, 0.0326509, -0.0581607, -0.184754, -0.122384, -0.176094, -0.175303, -0.125306, -0.0432769, -0.116332, -0.291237, -0.171145, -0.148936, -0.14473, -0.346695, -0.300901, -0.170199, -0.211235, -0.287233, -0.29211, -0.212529, -0.25585, -0.339051, -0.351805, -0.247089, -0.307578, -0.307298, -0.365652, -0.329659, -0.263652, -0.190336, -0.296123, -0.199918, -0.296881, -0.411994, -0.325004, -0.337861};

                double *target;
                if (unit == "a") {
                    target = a;
                }
                else if (unit == "i") {
                    target = i;
                }
                else if (unit == "u") {
                    target = u;
                }
                else if (unit == "e") {
                    target = e;
                }
                else if (unit == "o") {
                    target = o;
                }
                else {
                    target = nullptr;
                }

                /*
                std::uniform_int_distribution<> scope(0, 10);
                int len = scope(mt);
                if (len % 2 == 0) {
                    len *= -1;
                }
                */

                int len = ave + target[j] * 50;
                //int len = ave;
                //
                //insertCycle = LanczosResampling::convert(insertCycle, len);

                // 挿入
                for (int k = insertCycle.size() - 1; k >= 0; k--) {
                    inputForJulius.insert(inputForJulius.begin() + from + cycles[j].index, insertCycle[k]);
                }

                if (false) {
                    Gnuplot<double>::Output(inputCycle, before + "-" + unit + "-" + after, "w l");
                }
            }

            if (true) {
                std::vector<double> modified = Util::CopyVector(inputForJulius, from, length);
                //Gnuplot<double>::Output(modified, before + "-" + unit + "-" + after, "w l");

                //gnuplotForWaves.push_back(modified);
                std::string filename = "./pdf/image-" + before + "-" + unit + "-" + after + ".pdf";
                //Gnuplot<double>::Output2D(gnuplotForWaves, before + "-" + unit + "-" + after, "w l", nullptr, true, filename.c_str());
                Gnuplot<double>::Output(gnuplotForWaves[0], before + "-" + unit + "-" + after, "w l", nullptr, true, filename.c_str());
            }
        }

        // 音声ファイルとして保存
        if (true) {
            Wave wav;
            wav.CreateWave(inputForJulius, samplingSize, 16);
            // wav.CreateWave(filtered, samplingSize, 16);
            wav.OutputWave("./output/output" + Util::toString(x) + ".wav");
        }
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

        if (false) {  // Gnuplotで表示
            int bestIndex = KernelDensityEstimation::IndexOfMaxDensity(waves);
            std::vector<std::vector<double>> vecs;
            for (int j = 0; j < waves.size(); j++) {
                vecs.push_back(waves[j]);
            }
            for (int j = 0; j < 10; j++) {
                vecs.push_back(waves[bestIndex]);  // 線を濃くする
            }
            Gnuplot<double>::Output2D(vecs, title, "w l lc rgb '#E0FF0000'");

            std::string pdfname = "./pdf/" + title + ".pdf";
            Gnuplot<double>::Output2D(vecs, title, "w l lc rgb '#E0FF0000'", nullptr, true, pdfname.c_str());
        }

        if (false) {  // クラスタリング
            int countOfCluster = 10;
            int dim = waves[0].size();
            KMeansMethodResult result = KMeansMethod::Clustering(waves, dim, countOfCluster);
            Gnuplot<double>::Output2D(result.clusters, title, "w l");
        }

        if (false) {  // 音声ファイルとして保存
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

std::vector<UnitWave> getUnitWaves(int samplingSize) {
    std::vector<UnitWave> unitWaves;

    for (int h = 0; h <= 399; h++) {
        auto inputForJulius = Util::GetInput("./resource/" + Util::toString(h) + ".wav", 0);
        JuliusImporter juliusImporter("./resource/" + Util::toString(h) + ".lab");
        auto juliusResults = juliusImporter.getJuliusResults();
        auto splittedData = Util::GetSplittedDataByJulius(inputForJulius, samplingSize, juliusResults);

        for (int i = 1; i + 1 < juliusResults.size(); i++) {
            std::vector<double> data = splittedData[i];

            int targetSize = 360;
            int errorSize = 100;
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
                if (s > 40) {
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
