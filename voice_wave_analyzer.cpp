
#include "voice_wave_analyzer.h"


std::vector<Cycle> VoiceWaveAnalyzer::GetCycles(
		const std::vector<double> &input,
		const int samplingFrequency,
		const int minLength,
		const int maxLength
		) {
		std::cout << "音声の周期切り出しを行います。" << std::endl;

        int inputSize = input.size(); // 入力のサンプル数

		int dsize = (minLength + maxLength) / 2; // 相関関数の幅（Hz）
		int fixSize = (maxLength - minLength) / 2; // 零点修正誤差範囲（Hz）

		int index = 0;
		std::vector<int> indexes;
		while (index + maxLength * 2 < inputSize) {
			// 入力を探索し、それぞれの相関関数を求め、
			// 最大となる（波形が似ている）箇所を探し、保存していく。

			double maxR = std::numeric_limits<double>::min();
			double maxL = minLength;
			// 相関関数の最大になる部分を探索
			for (int i = minLength; i < maxLength; i++) {
				double r = 0;
				for (int j = 0; j < dsize; j++) {
					r += input[index + j] * input[index + j + i];
				}
				if (r > maxR) {
					// 最大の部分を保存
					maxR = r;
					maxL = i;
				}
			}
			// indexesに現在のindexを保存
			indexes.push_back(index);
			// 最大の分だけindexを移動
			index += maxL;
		}

		// indexesの零点を修正
		std::vector<int> fixedIndexes;
		for (int i = 0; i < indexes.size(); i++) {
			int index = indexes[i];

			for (int j = 0; j < fixSize; j++) {
                // indexの前後を調べ、符号が逆になる箇所を探す

				if ((index + j + 1 < inputSize)
					&& (input[index + j] * input[index + j + 1] <= 0)) {
					fixedIndexes.push_back(index + j);
					break;
				}
				else if ((index - j - 1 > 0)
					&& (input[index - j - 1] * input[index - j] <= 0)) {
					fixedIndexes.push_back(index - j - 1);
					break;
				}
			}
		}

		std::cout << "零点検出率 : " << 100.0 * fixedIndexes.size() / indexes.size() << "%" << std::endl;

        std::vector<Cycle> cycles;
        for (int i = 0; i < fixedIndexes.size() - 1; i++) {
            int length = fixedIndexes[i + 1] - fixedIndexes[i];
            if ((length > minLength) && (length < maxLength)) {
                Cycle cycle;
                cycle.index = fixedIndexes[i];
                cycle.length = length; 

                cycles.push_back(cycle);
            }
        }

        std::cout << "範囲外率 : " << 100.0 * cycles.size() / fixedIndexes.size() << "%" << std::endl;
        std::cout << "周期 : " << cycles.size() << "個" << std::endl << std::endl;

		return cycles;
	}

