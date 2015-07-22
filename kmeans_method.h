#include <iostream>
#include <vector>
#include <random>
#include <limits>

class KMeansMethod {
public:

    static std::vector<std::vector<double> > Clustering(std::vector<std::vector<double> > inputs,
            int dim,
            double countOfCluster) {
        
        int size = inputs.size();

        std::cout << "k-Means法を開始します。" << std::endl;


        // 最次元ごとの最大値、最小値の取得
        std::vector<double> mins;
        std::vector<double> maxs;
        for (int i = 0; i < dim; i++) {
            double min = std::numeric_limits<double>::max();
            mins.push_back(min);
            double max = std::numeric_limits<double>::min();
            maxs.push_back(max);
        }
        for (int i = 0; i < size; i++) {
            std::vector<double> input = inputs[i];

            for (int j = 0; j < dim; j++) {
                double value = input[j];

                if (value > maxs[j]) {
                    maxs[j] = value;
                }
                if (value < mins[j]) {
                    mins[j] = value;
                }
            }
        }

        // メルセンヌツイスタの初期化
        std::random_device random_device;
        std::mt19937 mt(random_device());

        // クラスタを乱数で初期化
        std::vector<std::vector<double> > clusters;
        for (int i = 0; i < countOfCluster; i++) {
            std::vector<double> vector;
            for (int j = 0; j < dim; j++) {
                // 最大値、最小値の範囲内の乱数器生成
                std::uniform_real_distribution<double> score(mins[j], maxs[j]);
                vector.push_back(score(mt));
            }
            clusters.push_back(vector);
        }

        // 入力がどこのクラスタに属しているか
        std::vector<int> clusterOfInputs(size, -1);

        while (true) {
            std::cout << "試行しています。" << std::endl;

            bool isChanged = false;

            // 入力をそれぞれ一番近いクラスタに付与
            for (int i = 0; i < size; i++) {
                std::vector<double> input = inputs[i];

                double minDistance = std::numeric_limits<double>::max();
                int minIndex;
                // 一番近いクラスタを検索
                for (int j = 0; j < clusters.size(); j++) {
                    double distance = 0;
                    std::vector<double> cluster = clusters[j];
                    for (int k = 0; k < dim; k++) {
                        distance += pow(input[k] - cluster[k], 2.0);
                    }

                    if (distance < minDistance) {
                        minDistance = distance;
                        minIndex = j;
                    }
                }

                // 距離が一番小さいところに置き換える
                if (clusterOfInputs[i] != minIndex) {
                    clusterOfInputs[i] = minIndex;

                    isChanged = true;
                }
            }

            // 変化がなければ処理を終了
            if (isChanged == false) {
                break;
            }

            // 各クラスタの中心を求める
            for (int i = clusters.size() - 1; i >= 0; i--) {

                std::vector<double> center(dim, 0);
                int count = 0;

                // i番目のクラスタに含まれている入力を検索
                for (int j = 0; j < size; j++) {
                    if (clusterOfInputs[j] == i) {

                        // クラスタの中心を求めるために加算
                        for (int k = 0; k < dim; k++) {
                            center[k] += inputs[j][k];
                        }

                        // 数を保存
                        count++;
                    }
                }

                if (count > 0) {
                    // クラスタの中心を求めるために数で割る
                    for (int j = 0; j < dim; j++) {
                        center[j] /= count;
                    }

                    // クラスタの決定
                    for (int j = 0; j < dim; j++) {
                        clusters[i][j] = center[j];
                    }
                }
                else {
                    clusters.erase(clusters.begin() + i);
                }

            }
        }

        // 要素がすべて0のクラスタを除去
        for (int i = clusters.size()-1; i >= 0; i--) {
            double r = 0;
            for (int j = 0; j < clusters[i].size(); j++) {
                r += clusters[i][j];
            }
            if (r == 0) {
                clusters.erase(clusters.begin() + i);
            }
        }

        std::cout << clusters.size() << " 個のクラスに分類されました。" << std::endl << std::endl;

        return clusters;
    }

};


