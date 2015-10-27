#include <iostream>
#include <vector>


struct KMeansMethodResult {
    std::vector<std::vector<double>> clusters;
    std::vector<int> indexOfCluster;
};


class KMeansMethod {
public:

    static KMeansMethodResult Clustering(const std::vector<std::vector<double>> &inputs,
            const int dim,
            const double countOfCluster);


private:

    static std::vector<int> getInitialClusterOfInputs(const std::vector<std::vector<double>> &inputs,
            const int dim,
            const double countOfCluster);

};


