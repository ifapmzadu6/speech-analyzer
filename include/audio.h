#include <iostream>
#include <fstream>
#include <vector>
#include <fftw3.h>

struct AudioComplex {
    double re;
    double im;
};

class Audio {
public:
    /**
     * 実データ1次元離散フーリエ変換（One-Dimensioanal DFTs of Real Data）
     */
    static std::vector<AudioComplex> dft_r2c_1d_vector(std::vector<double> input_vector, int fft_size, unsigned flags);

    /**
     * 実データ1次元離散コサイン変換
     */
    static std::vector<double> dct_r2r_1d_vector(std::vector<double> input_vector);

    /**
     * 実データ1次元離散コサイン逆変換
     */
    static std::vector<double> dct_r2r_1d_reverse_vector(std::vector<double> input_vector);
};
