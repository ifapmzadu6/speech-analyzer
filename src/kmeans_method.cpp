
#include <random>
#include <limits>
#include <cmath>

#include "kmeans_method.h"

KMeansMethodResult KMeansMethod::Clustering(const std::vector<std::vector<double> >& inputs, int dim, int countOfCluster)
{

    int size = inputs.size();
    std::vector<std::vector<double> > clusters(countOfCluster, std::vector<double>(dim, 0));

    std::cout << "- Clustering -" << std::endl;

    // kMeans法++で初期値を決定
    std::vector<int> clusterOfInputs = getInitialClusterOfInputs(inputs, dim, countOfCluster);
    std::cout << "initialized" << std::endl;

    while (true) {
        // 各クラスタの中心を求める
        for (int i = 0; i < countOfCluster; i++) {
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
        }

        bool isChanged = false;

        // 入力をそれぞれ一番近いクラスタに付与
        for (int i = 0; i < size; i++) {
            std::vector<double> input = inputs[i];

            double minDistance = std::numeric_limits<double>::max();
            int minIndex;
            // 一番近いクラスタを検索
            for (int j = 0; j < countOfCluster; j++) {
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
    }

    // 一つも属されていないクラスタを削除する
    for (int i = countOfCluster - 1; i >= 0; i--) {
        int count = 0;
        for (int j = 0; j < size; j++) {
            if (clusterOfInputs[j] == i) {
                count++;
            }
        }
        if (count == 0) {
            clusters.erase(clusters.begin() + i);
            for (int j = 0; j < size; j++) {
                if (clusterOfInputs[j] > i) {
                    clusterOfInputs[j] -= 1;
                }
            }
        }
    }

    std::cout << "clusters.size() -> " << clusters.size() << std::endl;

    KMeansMethodResult result;
    result.clusters = clusters;
    result.indexOfCluster = clusterOfInputs;
    return result;
}

std::vector<int> KMeansMethod::getInitialClusterOfInputs(const std::vector<std::vector<double> >& inputs, int dim, int countOfCluster)
{
    // メルセンヌツイスタの初期化
    std::random_device random_device;
    std::mt19937 mt(random_device());

    std::vector<int> indexOfCluster;
    // まず最初に乱数で一つだけ追加
    std::uniform_int_distribution<int> score(0, inputs.size() - 1);
    indexOfCluster.push_back(score(mt));

    while (indexOfCluster.size() < countOfCluster) {
        std::vector<double> distances;
        for (int i = 0; i < inputs.size(); i++) {
            std::vector<double> input = inputs[i];

            // 一番近いクラスタを探す
            double minDistance = std::numeric_limits<double>::max();
            int nearstCluster;
            for (int j = 0; j < indexOfCluster.size(); j++) {
                int index = indexOfCluster[j];
                double distanceForCluster = 0;
                for (int k = 0; k < dim; k++) {
                    distanceForCluster += pow(inputs[index][k] - input[k], 2.0);
                }
                if (distanceForCluster < minDistance) {
                    minDistance = distanceForCluster;
                    nearstCluster = j;
                }
            }

            // 一番近いクラスタとの距離を算出する
            double distance = 0;
            for (int j = 0; j < dim; j++) {
                distance += pow(input[j] - inputs[nearstCluster][j], 2.0);
            }
            distances.push_back(distance);
        }

        // 重み付き確率計算のために距離を範囲に変換
        double index = 0;
        for (int i = 0; i < distances.size(); i++) {
            double distance = distances[i];
            distances[i] = index;
            index += distance;
        }
        // 0 ~ 最大値までの乱数生成
        std::uniform_real_distribution<double> score(0, index);
        double random = score(mt);
        // 乱数がどこに含まれるかを探索
        int result = -1;
        for (int i = 0; i < distances.size(); i++) {
            if (distances[i] > random) {
                result = i;
                break;
            }
        }
        // 選択されたクラスタを追加
        if (result >= 0) {
            indexOfCluster.push_back(result);
        }
    }

    std::vector<int> clusterOfInputs;
    // 入力をそれぞれ一番近いクラスタに付与
    for (int i = 0; i < inputs.size(); i++) {
        std::vector<double> input = inputs[i];

        double minDistance = std::numeric_limits<double>::max();
        int minIndex;
        // 一番近いクラスタを検索
        for (int j = 0; j < countOfCluster; j++) {
            int index = indexOfCluster[j];
            std::vector<double> cluster = inputs[index];
            double distance = 0;
            for (int k = 0; k < dim; k++) {
                distance += pow(input[k] - cluster[k], 2.0);
            }
            if (distance < minDistance) {
                minDistance = distance;
                minIndex = j;
            }
        }
        clusterOfInputs.push_back(minIndex);
    }
    return clusterOfInputs;
}
