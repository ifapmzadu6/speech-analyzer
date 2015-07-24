#include <iostream>
#include <fstream>


template <typename T>
class Gnuplot {
public:

    static void OutputToGnuplot(std::vector<std::vector<T> > &outputs, const char *query, const char *filename) {
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

    static void OutputToGnuplot(std::vector<std::vector<T> > &outputs, const char *query) {
        OutputToGnuplot(outputs, query, "output.txt");
    }


    static void OutputToGnuplot(std::vector<T> &output, const char *query, const char *filename) {
        std::ofstream ofs(filename);
        for (int i = 0; i < output.size(); i++) {
            ofs << i << " " << output[i] << std::endl;
        }
        ofs.close();

        FILE *gnuplot = popen("gnuplot", "w");
        fprintf(gnuplot, "%s", query);
        pclose(gnuplot);
    }

    static void OutputToGnuplot(std::vector<T> &output, const char *query) {
        OutputToGnuplot(output, query, "output.txt");
    }

};



