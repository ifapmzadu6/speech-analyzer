#include <iostream>
#include <vector>


/*
 * Cycle: 周期を表す構造体
 *
 * @index: 開始位置
 * @length: 長さ
 */
struct Cycle {
	int index;
	int length;
}


/*
 * VoiceWaveAnalyzer: Waveフォーマットの音声解析クラス
 */
class VoiceWaveAnalyzer {

	/*
	 * GetCycles: 周期を解析する
	 *
	 * @input: 入力波形
	 * @samplingfrequency: サンプリング周波数（Hz）
	 * @minLength: 求める周期の最小（Hz）
	 * @maxLength: 求める周期の最大（Hz）
	 */
	std::vector<Cycle> GetCycles(
		const std::vector<double> &input,
		const int samplingFrequency,
		const int minLength,
		const int maxLength
		) {
		std::cout << "DFF now in progress..." << std::endl;

		int dsize = 100; // 相関関数の幅（Hz）
		int fixSize = 100; // 零点修正誤差範囲（Hz）


		int index = 0;
		std::vector<int> indexes;
		while (index + maxLength * 2 < input.size()) {
			// 入力を探索し、それぞれの相関関数を求め、
			// 最大となる（波形が似ている）箇所を探し、保存していく。

			double maxR = 0;
			double maxL = minLength;
			// 相関関数の最大になる部分を探索
			for (int i = minLength; i < maxLength; i++) {
				double r = 0;
				for (int j = 0; j < dsize; j++) {
					r += input[index + j] * input[index + j + i]
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
				if ((index + j + 1 < input.size())
					&& (input[index + j] * input[index + j + 1] <= 0)) {
					fixedIndexes.push_back(index + j);
					break;
				}
				if ((index - m - 1 > 0)
					&& (input[index - j] * input[index - j - 1] <= 0)) {
					fixedIndexes.push_back(index - j);
					break;
				}
			}
		}

		std::cout << "零点検出率 : " << 100.0*fixedIndexes.size() / indexes.size() << "%" << std::endl;

		return std::vector<Cycle>;
	}

}
