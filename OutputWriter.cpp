//
// Created by alexander on 08.11.2019.
//

#include "OutputWriter.h"

mutex OutputWriter::resultWriteMutex;
mutex OutputWriter::printMutex;
ofstream OutputWriter::resultWriter;
bool OutputWriter::writeResultsToFile = false;

void OutputWriter::setUpResultWriting(const string &fileName) {
    writeResultsToFile = true;
    resultWriter = ofstream(fileName);
}

void OutputWriter::writeLine(const string &line) {
    resultWriter << line << "\n";
}

void OutputWriter::writeBlock(float *mat, float *block, int blockSize) {
    for (int setIndex = 0; setIndex < blockSize; ++setIndex) {
        resultWriter << "Set #" << setIndex << "; Hamiltonian: "
                     << Annealing::hamiltonian(mat, block + Annealing::size * setIndex)
                     << "; Data:\n";
        for (int j = 0; j < Annealing::size; ++j) {
            resultWriter << block[setIndex * Annealing::size + j] << " ";
        }
        resultWriter << endl;
    }
    resultWriter << endl;
}

void OutputWriter::onResultsWritten(const string &postfix) {
    if (writeResultsToFile) {
        resultWriter << postfix;
        resultWriter.close();
    }
}

void OutputWriter::outputResultsOnStart(float *mat, float *block, int blockSize, float startTemp) {
    if (writeResultsToFile) {  // Block annealing started, write to full log
        resultWriteMutex.lock();
        ostringstream sHeader = ostringstream();
        sHeader << "Started processing block from temperature " << startTemp << ":";
        writeLine(sHeader.str());
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }
}


void OutputWriter::outputResultsIntermediate(float startTemp, float currentTemp) {
    if (writeResultsToFile) {  // Step complete, write to full log
        resultWriteMutex.lock();
        ostringstream stepComplete = ostringstream();
        stepComplete << "Annealing step complete: Start temperature " << startTemp << ", now " << currentTemp
                     << endl;
        writeLine(stepComplete.str());
        resultWriteMutex.unlock();
    }
}

void OutputWriter::outputResultsOnFinish(float *mat, float *block, int blockSize, float startTemp, int size,
                                         vector<vector<int>> allLinks, int stepCounter) {
    printMutex.lock();
    cout << startTemp << "\t";
    bool noInteraction = true;
    for (const vector<int> &link : allLinks)
        noInteraction = noInteraction && link.empty();
    /*
     * Circle braces - no-anneal run
     * Triangle braces - independent run
     * No braces - dependent run
     */
    for (int setIndex = 0; setIndex < blockSize; ++setIndex)
        if (!allLinks[setIndex].empty() && allLinks[setIndex][0] == -1)
            cout << " (" << Annealing::hamiltonian(mat, block + setIndex * size) << ")";
        else if (!allLinks[setIndex].empty() || noInteraction)
            cout << " " << Annealing::hamiltonian(mat, block + setIndex * size);
        else
            cout << " <" << Annealing::hamiltonian(mat, block + setIndex * size) << ">";
    cout << " [" << stepCounter << " iterations]"
         << endl;
    printMutex.unlock();

    if (writeResultsToFile) {  // Block annealing complete, write to full log
        resultWriteMutex.lock();
        ostringstream fHeader = ostringstream();
        fHeader << "Finished processing block; Start temperature was " << startTemp << "; Took " << stepCounter
                << " steps; block data:";
        writeLine(fHeader.str());
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }
}
