#include <iostream>
#include <vector>


class FIRFilter {

public:

    FIRFilter(int count, std::vector<double> init) : count(count), bufferIndex(count), wma(0.0), numerator(0.0), total(0.0) {
        buffer = std::vector<double>(count, 0);
        for (int i=0; i<count; i++) {
            next(init[i]);
        }
    }

    void next(double value);
    double getValue();



private:

    int count;

    std::vector<double> buffer;
    int bufferIndex;
    double getBuffer(int n);
    void setBuffer(double value, int n);
    void appendValueToBuffer(double value);

    double wma;
    double numerator;
    double total;

};

