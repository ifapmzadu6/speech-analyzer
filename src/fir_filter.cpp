#include "fir_filter.h"

void FIRFilter::next(double value) {
    numerator += count * value - total;
    total += value - getBuffer(1 - count);
    double denominator = count * (count + 1) / 2.0;
    wma = numerator / denominator;

    appendValueToBuffer(value);
}

double FIRFilter::getValue() { return wma; }

double FIRFilter::getBuffer(int n) {
    int index = (bufferIndex + n) % count;
    return buffer[index];
}

void FIRFilter::setBuffer(double value, int n) {
    int index = (bufferIndex + n) % count;
    buffer[index] = value;
}

void FIRFilter::appendValueToBuffer(double value) {
    bufferIndex++;
    setBuffer(value, 0);
}
