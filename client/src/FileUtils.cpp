#include <fstream>
#include <iostream>
#include "FileUtils.h"

bool writeSummaryToFile(const std::string &filePath, const std::string &content)
{
    std::ofstream outFile(filePath);
    if (!outFile)
    {
        std::cerr << "Error: Could not open file " << filePath << " for writing." << std::endl;
        return false;
    }
    outFile << content;
    outFile.close();
    return true;
}
