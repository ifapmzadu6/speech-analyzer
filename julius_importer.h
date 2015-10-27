#include <iostream>
#include <vector>
#include <string>


struct JuliusResult {
    double from;
    double to;
    std::string unit;
};


class JuliusImporter {
public:

    std::string filepath;

    JuliusImporter() { abort(); };

    JuliusImporter(std::string filepath) : filepath(filepath) {};

    std::vector<JuliusResult> getJuliusResults();

private:
    static const int samplingSize = 16000;
    static const int frameSize = 400;
    static const int frameShiftSize = 160;

};

