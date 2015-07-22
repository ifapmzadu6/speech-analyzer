#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "wave.h"
#include "voice_wave_analyzer.h"
#include "kmeans_method.h"



std::vector<double> getInput() {
    std::vector<double> input;
    Wave wav;
    if (wav.InputWave("sample.wav") != 0) {
        abort();
    }
    wav.Normalize();
    wav.GetData(input);

    for (int i=0; i<40200; i++) {
        input.erase(input.begin());
        input.pop_back();
    }

    return input;
}



int main() {

    int samplingFrequency = 44100;
    int targetFrequency = 360;
    int errorFrequency = 40;
    std::vector<double> input = getInput();
    
    std::cout << "入力: " << input.size() << " 個のサンプル" << std::endl << std::endl;

    // 周期を解析
    std::vector<Cycle> cycles = VoiceWaveAnalyzer::GetCycles(input,
            samplingFrequency,
            targetFrequency-errorFrequency,
            targetFrequency+errorFrequency);

    // 周期切り出し
    std::vector<std::vector<double> > inputCycles;
    for (int i = 0; i < cycles.size(); i++) {
        Cycle cycle = cycles[i];
        std::vector<double> inputCycle;
        for (int j = 0; j < cycle.length; j++) {
            inputCycle.push_back(input[cycle.index + j]);
        }
        // 足りない分を0で埋める
        while (inputCycle.size() < targetFrequency + 20) {
            inputCycle.push_back(0.0);
        }

        inputCycles.push_back(inputCycle);
    }

    std::vector<std::vector<double> > clusters = KMeansMethod::Clustering(inputCycles,
            targetFrequency + 20,
            5000);


    std::ofstream ofs("output.txt");
    for (int i = 0; i < clusters.size(); i++) {
        std::vector<double> cluster = clusters[i];
        for (int j = 0; j < cluster.size(); j++) {
            ofs << j << " " << cluster[j] << std::endl;
        }
        ofs << std::endl;
    }
    ofs.close();

    /*
    std::ofstream ofs("output.txt");
    for (int i = 0; i < inputCycles.size(); i++) {
        std::vector<double> inputCycle = inputCycles[i];
        for (int j = 0; j < inputCycle.size(); j++) {
            ofs << j << " " << inputCycle[j] << std::endl;
        }
        ofs << std::endl;
    }
    ofs.close();
    */ 


    FILE *gnuplot = popen("gnuplot", "w");
    fprintf(gnuplot, "plot 'output.txt' w l");
    //fprintf(gnuplot, "plot 'output.txt' w l lc rgb '#F0FF0000'");
    pclose(gnuplot);

    
    return 0;
}


/*
   gnuplot = popen("gnuplot", "w");
   fprintf(gnuplot, "plot 'output_wave.txt' w l lc rgb '#FF0000'");
   pclose(gnuplot);
   */

/*
    std::ofstream wave_ofs("output_wave.txt");
    int under = 70000;
    int upper = 80000;
    for (int i = under; i < upper; i++) {
        wave_ofs << i << " " << input[i] << std::endl;
    }

    wave_ofs << std::endl;
    for (int i = 0; i < cycles.size(); i++) {
        Cycle cycle = cycles[i];
        int index = cycle.index;
        if (index > under && index < upper) {
            wave_ofs << index << " -1" << std::endl;
            wave_ofs << index << " 1" << std::endl;
            wave_ofs << std::endl;
        }
        int indexLength = cycle.index + cycle.length;
        if (indexLength > under && indexLength < upper) {
            wave_ofs << indexLength << " -1" << std::endl;
            wave_ofs << indexLength << " 1" << std::endl;
            wave_ofs << std::endl;
        }
    }

    wave_ofs.close();
    */



