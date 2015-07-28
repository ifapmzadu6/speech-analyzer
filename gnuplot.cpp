
#include <fstream>

#include "gnuplot.h"



template<typename T>
void Gnuplot<T>::OutputToGnuplot(std::vector<T> &output, const char *option) {
    OutputToGnuplot(output, option, "output.txt");
}

template<typename T>
void Gnuplot<T>::OutputToGnuplot(std::vector<T> &output, const char *option, const char *filename) {
    std::ofstream ofs(filename);
    for (int i = 0; i < output.size(); i++) {
        ofs << i << " " << output[i] << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    if (option == nullptr) {
        fprintf(gnuplot, "p \'%s\'", filename);
    }
    else {
        fprintf(gnuplot, "p \'%s\' %s", filename, option);
    }
    pclose(gnuplot);
}



template<typename T>
void Gnuplot<T>::Output2DToGnuplot(std::vector< std::vector<T> > &outputs, const char *option) {
    Output2DToGnuplot(outputs, option, "output.txt");
}

template<typename T>
void Gnuplot<T>::Output2DToGnuplot(std::vector< std::vector<T> > &outputs, const char *option, const char *filename) {
    std::ofstream ofs(filename);
    for (int i = 0; i < outputs.size(); i++) {
        ofs << j;
        for (int j = 0; j < outputs[i].size(); j++) {
            ofs << " " << outputs[i][j];
        }
        ofs << std::endl;
    }
    ofs.close();

    int col = outputs[0].size();
    FILE *gnuplot = popen("gnuplot", "w");

    if (option == nullptr) {
        fprintf(gnuplot, "p \'%s\'", filename);
    }
    else {
        fprintf(gnuplot, "p \'%s\' %s", filename, option);
    }

    pclose(gnuplot);
}


