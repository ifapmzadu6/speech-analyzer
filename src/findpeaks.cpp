
#include "findpeaks.h"

template <typename T>
std::vector<int> FindPeaks<T>::finds(std::vector<T> &input) {
    double average = 0;
    for (int i = 0; i < input.size(); i++) {
        average += input[i];
    }
    average /= input.size();

    int localAverageRange = input.size() / 10;
    std::vector<int> inverted;
    for (int i = 1; i < input.size() - 1; i++) {
        double localAverage = 0;
        for (int j = i - localAverageRange; j <= i + localAverageRange; j++) {
            if (0 <= j && j < input.size()) {
                localAverage += input[j];
            }
        }
        localAverage /= localAverageRange * 2;

        if (input[i] - input[i - 1] > 0 && input[i + 1] - input[i] < 0 && input[i] > 0 && input[i] > localAverage && input[i] > average) {
            inverted.push_back(i);
        }
    }
    return inverted;
}

template <typename T>
void FindPeaks<T>::tests() {
    std::vector<double> input;
    for (int i = 1; i <= 100; i++) {
        double value = sin(2.0 * M_PI * 100 / i) + sin(2.0 * M_PI * 10 * 100 / i) / 5.0;
        input.push_back(value);
    }

    std::vector<int> result = FindPeaks<double>::finds(input);

    const char *filename = "./tmp/output.txt";
    std::ofstream ofs(filename);
    int index = 0;
    for (int i = 0; i < input.size(); i++) {
        ofs << i << " " << input[i] << " ";
        if (result[index] == i) {
            ofs << input[i];
            index++;
        } else {
            ofs << 0;
        }
        ofs << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    fprintf(gnuplot, "unset key;");
    fprintf(gnuplot, "p \'%s\' using 1:2 w l, \'%s\' using 1:3", filename, filename);
    pclose(gnuplot);
}
