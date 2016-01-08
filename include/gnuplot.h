#pragma once

#include <iostream>
#include <vector>
#include <string>

template <typename T>
class Gnuplot {

   public:

    // ベクトルを表示
    static void Output(std::vector<T> &output, std::string title, const char *option);
    static void Output(std::vector<T> &output, std::string title, const char *option, const char *filename);
    static void Output(std::vector<T> &output, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname);

    // 2次元ベクトルを表示
    static void Output2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option);
    static void Output2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename);
    static void Output2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname);

    // ベクトルを周期表示
    static void OutputCyclize(std::vector<T> &outputs, std::string title, const char *option);
    static void OutputCyclize(std::vector<T> &outputs, std::string title, const char *option, const char *filename);
    static void OutputCyclize(std::vector<T> &outputs, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname);

    // 2次元ベクトルを周期表示
    static void OutputCyclize2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option);
    static void OutputCyclize2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename);
    static void OutputCyclize2D(std::vector<std::vector<T>> &outputs, std::string title, const char *option, const char *filename, bool isPDF, const char *pdfname);

};
