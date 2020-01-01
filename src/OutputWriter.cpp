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

void OutputWriter::writeMat(float *mat) {
    if (writeResultsToFile) {
        resultWriter << "Size: " << Annealing::size << ". Cell content:" << endl;
        for (int i = 0; i < Annealing::size; i++) {
            resultWriter << i << ">\t";
            for (int j = 0; j < Annealing::size; j++)
                resultWriter << mat[i * Annealing::size + j] << " ";
            resultWriter << endl;
        }
    }
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
        resultWriter << "Started processing block from temperature " << startTemp << ":" << endl;
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }
}


void OutputWriter::outputResultsIntermediate(float startTemp, float currentTemp) {
    if (writeResultsToFile) {  // Step complete, write to full log
        resultWriteMutex.lock();
        resultWriter << "Annealing step complete: Start temperature " << startTemp << ", now " << currentTemp
                     << endl << endl;
        resultWriteMutex.unlock();
    }
}

void OutputWriter::outputResultsOnFinish(float *mat, float *block, int blockSize, float startTemp, int size,
                                         int stepCounter) {
    printMutex.lock();
    cout << startTemp << "\t";
    /*
     * Circle braces - no-anneal run
     * Triangle braces - independent run
     * No braces - dependent run
     */
    for (int setIndex = 0; setIndex < blockSize; ++setIndex)
        cout << " " << Annealing::hamiltonian(mat, block + setIndex * size);
    cout << " [" << stepCounter << " iterations]"
         << endl;
    printMutex.unlock();

    if (writeResultsToFile) {  // Block annealing complete, write to full log
        resultWriteMutex.lock();
        resultWriter << "Finished processing block; Start temperature was " << startTemp << "; Took " << stepCounter
                     << " steps; block data:" << endl;
        writeBlock(mat, block, blockSize);
        resultWriteMutex.unlock();
    }
}
