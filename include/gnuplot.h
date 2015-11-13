
#include <iostream>
#include <vector>
#include <string>

template<typename T>
class Gnuplot {

public:

    // ベクトルを表示
    static void OutputToGnuplot(std::vector<T> &output, std::string title, const char *option);
    static void OutputToGnuplot(std::vector<T> &output, std::string title, const char *option, const char *filename);

    // 2次元ベクトルを表示
    static void Output2DToGnuplot(std::vector< std::vector<T>> &outputs, std::string title, const char *option);
    static void Output2DToGnuplot(std::vector< std::vector<T>> &outputs, std::string title, const char *option, const char *filename);

};

