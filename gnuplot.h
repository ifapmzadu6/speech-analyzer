
#include <iostream>
#include <vector>

template<typename T>
class Gnuplot {

public:

    // ベクトルを表示
    static void OutputToGnuplot(std::vector<T> &output, const char *option);
    static void OutputToGnuplot(std::vector<T> &output, const char *option, const char *filename);

    // 2次元ベクトルを表示
    static void Output2DToGnuplot(std::vector< std::vector<T> > &outputs, const char *option);
    static void Output2DToGnuplot(std::vector< std::vector<T> > &outputs, const char *option, const char *filename);

};

