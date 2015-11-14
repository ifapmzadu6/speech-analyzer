#include <fstream>

#include "julius_importer.h"

std::vector<JuliusResult> JuliusImporter::getJuliusResults()
{
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
};

std::vector<std::string> JuliusImporter::split(const std::string& str, char delim)
{
    std::vector<std::string> res;
    size_t current = 0, found;
    while ((found = str.find_first_of(delim, current)) != std::string::npos) {
        res.push_back(std::string(str, current, found - current));
        current = found + 1;
    }
    res.push_back(std::string(str, current, str.size() - current));
    return res;
}
