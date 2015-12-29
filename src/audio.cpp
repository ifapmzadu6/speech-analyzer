#include "audio.h"

/**
 * 実データ1次元離散フーリエ変換（One-Dimensioanal DFTs of Real Data）
 */
std::vector<AudioComplex> Audio::dft_r2c_1d_vector(
    std::vector<double> input_vector, int fft_size, unsigned flags) {
    int n = (int)input_vector.size();
    double *input = (double *)malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++) {
        input[i] = input_vector[i];
    }
    fftw_complex *output =
        (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * n);

    fftw_plan plan =
        fftw_plan_dft_r2c_1d(fft_size, input, output, FFTW_ESTIMATE);
    fftw_execute(plan);

    std::vector<AudioComplex> output_vector;
    for (int i = 0; i < n / 2 + 1; i++) {
        AudioComplex complex = {output[i][0], output[i][1]};
        output_vector.push_back(complex);
    }

    if (plan) fftw_destroy_plan(plan);
    fftw_free(output);
    free(input);

    return output_vector;
}

/**
 * 実データ1次元離散コサイン変換
 */
std::vector<double> Audio::dct_r2r_1d_vector(std::vector<double> input_vector) {
    int n = (int)input_vector.size();
    double *input = (double *)malloc(sizeof(double) * n);
    double *output = (double *)malloc(sizeof(double) * n);

    for (int i = 0; i < n; i++) {
        input[i] = input_vector[i];
    }

    fftw_plan plan =
        fftw_plan_r2r_1d(n, input, output, FFTW_REDFT10, FFTW_ESTIMATE);
    fftw_execute(plan);

    std::vector<double> output_vector;
    for (int i = 0; i < n; i++) {
        output_vector.push_back(output[i]);
    }

    if (plan) fftw_destroy_plan(plan);
    free(output);
    free(input);

    return output_vector;
}

/**
 * 実データ1次元離散コサイン逆変換
 */
std::vector<double> Audio::dct_r2r_1d_reverse_vector(
    std::vector<double> input_vector) {
    int n = (int)input_vector.size();
    double *input = (double *)malloc(sizeof(double) * n);
    double *output = (double *)malloc(sizeof(double) * n);

    for (int i = 0; i < n; i++) {
        input[i] = input_vector[i];
    }

    fftw_plan plan =
        fftw_plan_r2r_1d(n, input, output, FFTW_REDFT01, FFTW_ESTIMATE);
    fftw_execute(plan);

    std::vector<double> output_vector;
    for (int i = 0; i < n; i++) {
        output_vector.push_back(output[i]);
    }

    if (plan) fftw_destroy_plan(plan);
    free(output);
    free(input);

    return output_vector;
}
