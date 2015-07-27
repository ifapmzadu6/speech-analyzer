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
};


/*
 * VoiceWaveAnalyzer: Waveフォーマットの音声解析クラス
 */
class VoiceWaveAnalyzer {

    public:

	/*
	 * GetCycles: 周期を解析する
	 *
	 * @input: 入力波形
	 * @samplingfrequency: サンプリング周波数（Hz）
	 * @minLength: 求める周期の最小（Hz）
	 * @maxLength: 求める周期の最大（Hz）
	 */
	static std::vector<Cycle> GetCycles(
		const std::vector<double> &input,
		const int samplingFrequency,
		const int minLength,
		const int maxLength
		);

};

