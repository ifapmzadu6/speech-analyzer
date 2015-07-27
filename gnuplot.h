
#include <iostream>
#include <vector>


template<typename T>
class Gnuplot {
public:

    // ベクトルを表示
    static void OutputToGnuplot(std::vector<T> &output, const char *query, const char *filename);
    static void OutputToGnuplot(std::vector<T> &output, const char *query);

    // 2次元ベクトルを表示
    static void OutputToGnuplot(std::vector<std::vector<T> > &outputs, const char *query, const char *filename);
    static void OutputToGnuplot(std::vector<std::vector<T> > &outputs, const char *query);

};



