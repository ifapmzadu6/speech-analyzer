#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

struct JuliusResult {
    double from;
    double to;
    std::string unit;
};

class JuliusImporter {
    public:
        std::string filepath;

        JuliusImporter() { abort(); };

        JuliusImporter(std::string filepath) : filepath(filepath){};

        std::vector<JuliusResult> getJuliusResults() {
            std::ifstream ifs(filepath);
            if (ifs.fail()) {
                std::cerr << "Can't open " << filepath << std::endl;
                abort();
            }
            std::vector<JuliusResult> results;
            std::string str;
            while (getline(ifs, str)) {
                std::vector<std::string> splited = split(str, ' ');
                if (splited.size() >= 3) {
                    JuliusResult result = JuliusResult();
                    result.from = std::stod(splited[0]);
                    result.to = std::stod(splited[1]);
                    result.unit = splited[2];
                    results.push_back(result);
                }
            }
            return results;
        }

    private:
        static const int samplingSize = 16000;
        static const int frameSize = 400;
        static const int frameShiftSize = 160;

        std::vector<std::string> split(const std::string &str, char delim) {
            std::vector<std::string> res;
            size_t current = 0, found;
            while ((found = str.find_first_of(delim, current)) != std::string::npos) {
                res.push_back(std::string(str, current, found - current));
                current = found + 1;
            }
            res.push_back(std::string(str, current, str.size() - current));
            return res;
        }
};
