//
// Created by alexander on 08.11.2019.
//

#ifndef MARS_CI_OUTPUTWRITER_H
#define MARS_CI_OUTPUTWRITER_H

#include <sstream>
#include <fstream>
#include <mutex>
#include <vector>
#include <iostream>
#include "Annealing.h"


namespace OutputWriter {
    extern std::mutex resultWriteMutex;
    extern std::mutex printMutex;
    extern std::ofstream resultWriter;
    extern bool writeResultsToFile;

    void setUpResultWriting(const std::string &fileName);

    void writeLine(const std::string &line);

    void writeBlock(float *mat, float *block, std::vector<std::vector<int>> allLinks, int blockSize);

    void onResultsWritten(const std::string &postfix = "");

    void outputResultsOnStart(float *mat, float *block, int blockSize, float startTemp);

    void outputResultsIntermediate(float startTemp, float currentTemp);

    void
    outputResultsOnFinish(float *mat, float *block, int blockSize, float startTemp, int size,
                          std::vector<std::vector<int>> allLinks,
                          int stepCounter);
};


#endif //MARS_CI_OUTPUTWRITER_H
