//
// Created by alexander on 08.11.2019.
//

#include "OutputWriter.h"

std::mutex OutputWriter::resultWriteMutex;
std::mutex OutputWriter::printMutex;
std::ofstream OutputWriter::resultWriter;
bool OutputWriter::writeResultsToFile = false;

void OutputWriter::setUpResultWriting(const std::string &fileName) {
    writeResultsToFile = true;
    resultWriter = std::ofstream(fileName);
}

void OutputWriter::writeLine(const std::string &line) {
    resultWriter << line << "\n";
}

void OutputWriter::writeBlock(float *mat, float *block, std::vector<std::vector<int>> allLinks, int blockSize) {
    for (int setIndex = 0; setIndex < blockSize; ++setIndex) {
        resultWriter << "Set #" << setIndex << "; ";
        if (not allLinks.empty()) {
            if (allLinks[setIndex].empty())
                resultWriter << "Type: Independent; ";
            else if (allLinks[setIndex][0] == -1)
                resultWriter << "Type: No-anneal; ";
            else
                resultWriter << "Type: Dependent; ";
        }
        resultWriter << "Hamiltonian: "
                     << Annealing::hamiltonian(mat, block + Annealing::size * setIndex)
                     << "; Data:\n";
        for (int j = 0; j < Annealing::size; ++j) {
            resultWriter << block[setIndex * Annealing::size + j] << " ";
        }
        resultWriter << std::endl;
    }
    resultWriter << std::endl;
}

void OutputWriter::onResultsWritten(const std::string &postfix) {
    if (writeResultsToFile) {
        resultWriter << postfix;
        resultWriter.close();
    }
}

void OutputWriter::outputResultsOnStart(float *mat, float *block, int blockSize, float startTemp) {
    if (writeResultsToFile) {  // Block annealing started, write to full log
        resultWriteMutex.lock();
        std::ostringstream sHeader = std::ostringstream();
        sHeader << "Started processing block from temperature " << startTemp << ":";
        writeLine(sHeader.str());
        writeBlock(mat, block, {}, blockSize);
        resultWriteMutex.unlock();
    }
}


void OutputWriter::outputResultsIntermediate(float startTemp, float currentTemp) {
    if (writeResultsToFile) {  // Step complete, write to full log
        resultWriteMutex.lock();
        std::ostringstream stepComplete = std::ostringstream();
        stepComplete << "Annealing step complete: Start temperature " << startTemp << ", now " << currentTemp
                     << std::endl;
        writeLine(stepComplete.str());
        resultWriteMutex.unlock();
    }
}

void OutputWriter::outputResultsOnFinish(float *mat, float *block, int blockSize, float startTemp, int size,
                                         std::vector<std::vector<int>> allLinks, int stepCounter) {
    printMutex.lock();
    std::cout << startTemp << "\t";
    bool noInteraction = true;
    for (const std::vector<int> &link : allLinks)
        noInteraction = noInteraction && link.empty();
    /*
     * Triangle braces - independent run
     * Circle braces - no-anneal run
     * No braces - dependent run
     */
    for (int setIndex = 0; setIndex < blockSize; ++setIndex)
        if (allLinks[setIndex].empty())
            std::cout << " <" << Annealing::hamiltonian(mat, block + setIndex * size) << ">";
        else if (allLinks[setIndex][0] == -1)
            std::cout << " (" << Annealing::hamiltonian(mat, block + setIndex * size) << ")";
        else
            std::cout << " " << Annealing::hamiltonian(mat, block + setIndex * size);

    std::cout << " [" << stepCounter << " iterations]"
              << std::endl;
    printMutex.unlock();

    if (writeResultsToFile) {  // Block annealing complete, write to full log
        resultWriteMutex.lock();
        std::ostringstream fHeader = std::ostringstream();
        fHeader << "Finished processing block; Start temperature was " << startTemp << "; Took " << stepCounter
                << " steps; block data:";
        writeLine(fHeader.str());
        writeBlock(mat, block, allLinks, blockSize);
        resultWriteMutex.unlock();
    }
}
