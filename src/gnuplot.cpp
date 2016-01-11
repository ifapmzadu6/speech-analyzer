#include <fstream>

#include "gnuplot.h"

template <typename T>
void Gnuplot<T>::Output(std::vector<T> &output, std::string title, const char *option) {
    Output(output, title, option, nullptr);
}

template <typename T>
void Gnuplot<T>::Output(std::vector<T> &output, std::string title, const char *option, const char *filename) {
    Output(output, title, option, filename, false, NULL);
}

template <typename T>
void Gnuplot<T>::Output(std::vector<T> &output, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname) {
    if (filename == nullptr) {
        filename = "./tmp/output.txt";
    }

    std::ofstream ofs(filename);
    for (int i = 0; i < output.size(); i++) {
        ofs << i << " " << output[i] << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    if (isPDF) {
        fprintf(gnuplot, "set term pdf; set output \"%s\";", pdfname);
    } else {
        // fprintf(gnuplot, "set title \"%s\";", title);
        fprintf(gnuplot, "set term aqua title \"%s\";", title.c_str());
    }
    fprintf(gnuplot, "unset key;");
    if (option == nullptr) {
        fprintf(gnuplot, "p \'%s\'", filename);
    } else {
        fprintf(gnuplot, "p \'%s\' %s", filename, option);
    }
    pclose(gnuplot);
}

template <typename T>
void Gnuplot<T>::Output2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option) {
    Output2D(outputs, title, option, nullptr);
}

template <typename T>
void Gnuplot<T>::Output2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename) {
    Output2D(outputs, title, option, filename, false, NULL);
}

template <typename T>
void Gnuplot<T>::Output2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname) {
    if (filename == nullptr) {
        filename = "./tmp/output.txt";
    }

    std::ofstream ofs(filename);

    int maxCol = 0;
    for (int i = 0; i < outputs.size(); i++) {
        if (maxCol < outputs[i].size()) {
            maxCol = outputs[i].size();
        }
    }

    for (int i = 0; i < maxCol; i++) {
        ofs << i;
        for (int j = 0; j < outputs.size(); j++) {
            if (i < outputs[j].size()) {
                ofs << " " << outputs[j][i];
            }
        }
        ofs << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    if (isPDF) {
        fprintf(gnuplot, "set term pdf; set output \"%s\";", pdfname);
    } else {
        // fprintf(gnuplot, "set title \"%s\";", title);
        fprintf(gnuplot, "set term aqua title \"%s\";", title.c_str());
    }
    fprintf(gnuplot, "unset key;");
    fprintf(gnuplot, "p ");

    if (maxCol == 1) {
        if (option == nullptr) {
            fprintf(gnuplot, "\'%s\'", filename);
        } else {
            fprintf(gnuplot, "\'%s\' %s", filename, option);
        }
    } else {
        for (int i = 0; i < outputs.size(); i++) {
            if (option == nullptr) {
                fprintf(gnuplot, "\'%s\' u 1:%d", filename, i + 2);
            } else {
                fprintf(gnuplot, "\'%s\' using 1:%d %s", filename, i + 2, option);
            }
            if (i < outputs.size() - 1) {
                fprintf(gnuplot, ", ");
            }
        }
    }

    pclose(gnuplot);
}

template <typename T>
void Gnuplot<T>::OutputCyclize(std::vector<T> &output, std::string title, const char *option) {
    OutputCyclize(output, title, option, nullptr);
}

template <typename T>
void Gnuplot<T>::OutputCyclize(std::vector<T> &output, std::string title, const char *option, const char *filename) {
    OutputCyclize(output, title, option, filename, false, NULL);
}

template <typename T>
void Gnuplot<T>::OutputCyclize(std::vector<T> &output, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname) {
    if (filename == nullptr) {
        filename = "./tmp/output.txt";
    }

    int count = 3;

    std::ofstream ofs(filename);
    for (int i = 0; i < output.size() * count; i++) {
        ofs << i << " " << output[i % output.size()] << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    if (isPDF) {
        fprintf(gnuplot, "set term pdf; set output \"%s\";", pdfname);
    } else {
        // fprintf(gnuplot, "set title \"%s\";", title);
        fprintf(gnuplot, "set term aqua title \"%s\";", title.c_str());
    }
    fprintf(gnuplot, "unset key;");
    if (option == nullptr) {
        fprintf(gnuplot, "p \'%s\'", filename);
    } else {
        fprintf(gnuplot, "p \'%s\' %s", filename, option);
    }
    pclose(gnuplot);
}

template <typename T>
void Gnuplot<T>::OutputCyclize2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option) {
    OutputCyclize2D(outputs, title, option, nullptr);
}

template <typename T>
void Gnuplot<T>::OutputCyclize2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename) {
    if (filename == nullptr) {
        filename = "./tmp/output.txt";
    }
    OutputCyclize2D(outputs, title, option, filename, false, NULL);
}

template <typename T>
void Gnuplot<T>::OutputCyclize2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname) {
    if (filename == nullptr) {
        filename = "./tmp/output.txt";
    }

    int count = 3;

    int maxCol = 0;
    for (int i = 0; i < outputs.size(); i++) {
        if (maxCol < outputs[i].size()) {
            maxCol = outputs[i].size();
        }
    }

    std::ofstream ofs(filename);
    for (int i = 0; i < maxCol * count; i++) {
        ofs << i;
        for (int j = 0; j < outputs.size(); j++) {
            int size = outputs[j].size();
            if (i < size) {
                ofs << " " << outputs[j][i % size];
            }
        }
        ofs << std::endl;
    }
    ofs.close();

    FILE *gnuplot = popen("gnuplot", "w");
    if (isPDF) {
        fprintf(gnuplot, "set term pdf; set output \"%s\";", pdfname);
    } else {
        // fprintf(gnuplot, "set title \"%s\";", title);
        fprintf(gnuplot, "set term aqua title \"%s\";", title.c_str());
    }
    fprintf(gnuplot, "unset key;");
    fprintf(gnuplot, "p ");

    if (maxCol == 1) {
        if (option == nullptr) {
            fprintf(gnuplot, "\'%s\'", filename);
        } else {
            fprintf(gnuplot, "\'%s\' %s", filename, option);
        }
    } else {
        for (int i = 0; i < outputs.size(); i++) {
            if (option == nullptr) {
                fprintf(gnuplot, "\'%s\' u 1:%d", filename, i + 2);
            } else {
                fprintf(gnuplot, "\'%s\' using 1:%d %s", filename, i + 2, option);
            }
            if (i < outputs.size() - 1) {
                fprintf(gnuplot, ", ");
            }
        }
    }

    pclose(gnuplot);
}
