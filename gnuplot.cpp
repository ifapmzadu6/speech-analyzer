
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
    fprintf(gnuplot, "unset key;");
    if (option == nullptr) {
        fprintf(gnuplot, "p \'%s\'", filename);
    }
    else {
        fprintf(gnuplot, "p \'%s\' %s", filename, option);
    }
    pclose(gnuplot);
}



template<typename T>
void Gnuplot<T>::Output2DToGnuplot(std::vector< std::vector<T>> &outputs, const char *option) {
    Output2DToGnuplot(outputs, option, "output.txt");
}

template<typename T>
void Gnuplot<T>::Output2DToGnuplot(std::vector< std::vector<T>> &outputs, const char *option, const char *filename) {
    std::ofstream ofs(filename);

    int col = outputs[0].size();
    for (int i = 0; i < col; i++) {
        ofs << i;
        for (int j = 0; j < outputs.size(); j++) {
            ofs << " " << outputs[j][i];
        }
        ofs << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    fprintf(gnuplot, "unset key;");
    fprintf(gnuplot, "p ");

    if (col == 1) {
        if (option == nullptr) {
            fprintf(gnuplot, "\'%s\'", filename);
        }
        else {
            fprintf(gnuplot, "\'%s\' %s", filename, option);
        }
    }
    else {
        for (int i = 0; i < outputs.size(); i++) {
            if (option == nullptr) {
                fprintf(gnuplot, "\'%s\' u 1:%d", filename, i + 2);
            }
            else {
                fprintf(gnuplot, "\'%s\' using 1:%d %s", filename, i + 2, option);
            }
            if (i < outputs.size()-1) {
                fprintf(gnuplot, ", ");
            }
        }
    }

    pclose(gnuplot);
}


