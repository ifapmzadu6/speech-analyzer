
#include <fstream>

#include "gnuplot.h"



// 明示的テンプレート宣言
template void Gnuplot<int>::OutputToGnuplot(std::vector<int> &output, const char *query);
template void Gnuplot<double>::OutputToGnuplot(std::vector<double> &output, const char *query);
template void Gnuplot<int>::OutputToGnuplot(std::vector<int> &output, const char *query, const char *filename);
template void Gnuplot<double>::OutputToGnuplot(std::vector<double> &output, const char *query, const char *filename);



template<typename T>
void Gnuplot<T>::OutputToGnuplot(std::vector<T> &output, const char *query, const char *filename) {
    std::ofstream ofs(filename);
    for (int i = 0; i < output.size(); i++) {
        ofs << i << " " << output[i] << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    fprintf(gnuplot, "%s", query);
    pclose(gnuplot);
}

template<typename T>
void Gnuplot<T>::OutputToGnuplot(std::vector<T> &output, const char *query) {
    OutputToGnuplot(output, query, "output.txt");
}


template<typename T>
void Gnuplot<T>::OutputToGnuplot(std::vector<std::vector<T> > &outputs, const char *query, const char *filename) {
    std::ofstream ofs(filename);
    for (int i = 0; i < outputs.size(); i++) {
        std::vector<T> output = outputs[i];
        for (int j = 0; j < output.size(); j++) {
            ofs << j << " " << output[j] << std::endl;
        }
        ofs << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    fprintf(gnuplot, "%s", query);
    pclose(gnuplot);
}

template<typename T>
void Gnuplot<T>::OutputToGnuplot(std::vector<std::vector<T> > &outputs, const char *query) {
    OutputToGnuplot(outputs, query, "output.txt");
}


