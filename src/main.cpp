#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

#include "InputLoader.h"
#include "Annealing.h"
#include "OutputWriter.h"
#include "BigFloat.h"

#define VERSION "3.0";
#define BUILD 4;

/*
 * TERMINOLOGY:
 * Mat - describes J_ij lattice
 * Set - describes system mean spin values at certain state
 * Block - several sets which descend simultaneously and interact with each other
 * Link - A std::std::vector describing set interaction in a block
 */

std::vector<std::string> matLoadModes{"RAND", "FILE_TABLE", "FILE_LIST"};
std::vector<std::string> hamiltonianModes{"LOG", "NO_LOG"};

int main() {
    std::cout << "MARS analysis by A. Yavorski, CPU edition, version " << VERSION
    std::cout << ", build " << BUILD
    std::cout << std::endl; // Wtf?! Doesn't work when in one line! FIXME

    // Load temperature bounds
    float tempStart = 10, tempFinal = 10, annealingStep = 0.01;
    std::cout << "Start temp?" << std::endl;
    std::cin >> tempStart;
    std::cout << "Final temp?" << std::endl;
    std::cin >> tempFinal;
    std::cout << "Annealing step?" << std::endl;
    std::cin >> annealingStep;

    // Load lattice
    std::string matLoadMode;
    std::cout << "Lattice type?" << std::endl;
    std::cin >> matLoadMode;
    while (find(matLoadModes.begin(), matLoadModes.end(), matLoadMode) == matLoadModes.end()) {
        std::cout << "Expected one of following: ";
        for (const std::string &loadMode : matLoadModes)
            std::cout << loadMode << ", ";
        std::cout << std::endl;
        std::cin >> matLoadMode;
    }
    float *J;
    std::string matFilename;
    switch ((int) (find(matLoadModes.begin(), matLoadModes.end(), matLoadMode) - matLoadModes.begin())) {
        case 0:
            // Random lattice
            std::cout << "Lattice size?" << std::endl;
            std::cin >> Annealing::size;
            J = new float[Annealing::size * Annealing::size];
            srand(10);  // Invariant seed for easier testing
            Annealing::matRandomize(J);
            break;
        case 1:
            // Load in table mode
            std::cout << "File path?" << std::endl;
            std::cin >> matFilename;
            J = InputLoader::loadMatFromTable(matFilename, &Annealing::size);
            std::cout << "Lattice loaded, size: " << Annealing::size << " (check). ";
            break;
        case 2:
            // Load in list mode
            std::cout << "File path?" << std::endl;
            std::cin >> matFilename;
            J = InputLoader::loadMatFromList(matFilename, &Annealing::size);
            std::cout << "Lattice loaded, size: " << Annealing::size << " (check). ";
            break;
    }

    // Load block configuration
    std::cout << "Thread quantity?" << std::endl;
    int threads;
    std::cin >> threads;
    std::cout << "Block file location (Enter block size to create a random block)?" << std::endl;
    std::string blockFilename;
    std::cin >> blockFilename;
    int blockSize = 0;
    float *Blocks;
    bool randomizeBlocks;
    try {
        blockSize = stoi(blockFilename);
        Blocks = new float[Annealing::size * blockSize * threads];
        randomizeBlocks = true;
    } catch (std::exception &e) {
        auto ifs = std::ifstream(blockFilename);
        ifs >> blockSize;
        ifs.close();
        Blocks = new float[Annealing::size * blockSize * threads];
        randomizeBlocks = false;
        std::cout << "Block loaded, size: " << blockSize << " (check). ";
    }
    int blockQty;
    std::cout << "Block quantity?" << std::endl;
    std::cin >> blockQty;

    // Load link configuration
    std::string linksFilename;
    std::vector<std::vector<int>> allLinks;
    std::cout << "Links file location (NONE for no interaction)?" << std::endl;
    std::cin >> linksFilename;
    if (linksFilename == "NONE")
        allLinks = std::vector<std::vector<int >>(Annealing::size, std::vector<int>{});
    else
        allLinks = InputLoader::loadLinks(linksFilename);

    // Interaction quotient
    std::cout << "Interaction quotient (decimal log)?" << std::endl;
    float fQuotient;
    std::cin >> fQuotient;
    Annealing::interactionQuotient = BigFloat(exp10f(fQuotient - (int) fQuotient), (int) fQuotient);

    // Hamiltonian mode: log or not log
    std::string hamiltonianMode;
    std::cout << "Hamiltonian mode?" << std::endl;
    std::cin >> hamiltonianMode;
    while (find(hamiltonianModes.begin(), hamiltonianModes.end(), hamiltonianMode) == hamiltonianModes.end()) {
        std::cout << "Expected one of following: ";
        for (const std::string &hamMode : hamiltonianModes)
            std::cout << hamMode << ", ";
        std::cout << std::endl;
        std::cin >> hamiltonianMode;
    }
    bool hamiltonianLog = hamiltonianModes[0] == hamiltonianMode;

    std::cout << "Temperature threshold?" << std::endl;
    std::cin >> Annealing::temperatureInteractionThreshold;

    // Enable/disable full log
    std::cout << "File to save all results (NONE for no saving)?" << std::endl;
    std::string resultsFileName;
    std::cin >> resultsFileName;
    if (resultsFileName != "NONE")
        OutputWriter::setUpResultWriting(resultsFileName);

    // Start annealing
    bool *runningFlags = new bool[threads];
    for (int thrIndex = 0; thrIndex < threads; ++thrIndex)
        runningFlags[thrIndex] = true;
    int launchedThrCount = 0;
    while (launchedThrCount < blockQty)
        for (int thrIndex = 0; thrIndex < threads; thrIndex++)
            if (runningFlags[thrIndex] && launchedThrCount < blockQty) {  // Free place for thread detected
                runningFlags[thrIndex] = false;
                if (randomizeBlocks)
                    for (int j = 0; j < blockSize; ++j)
                        Annealing::setRandomize(Blocks + Annealing::size * blockSize * thrIndex + Annealing::size * j);
                else
                    InputLoader::loadBlock(blockFilename, Blocks + Annealing::size * blockSize * thrIndex,
                                           Annealing::size);

                // Launch new run on a separate thread
                std::thread(Annealing::anneal, J, Blocks + Annealing::size * blockSize * thrIndex, blockSize,
                            tempStart + ((float) launchedThrCount / (float) blockQty) * (tempFinal - tempStart),
                            annealingStep, runningFlags + thrIndex, allLinks, hamiltonianLog).detach();
                launchedThrCount++;
            }

    bool runningFlag = true;
    while (runningFlag) {
        runningFlag = false;
        for (int i = 0; i < threads; i++)
            runningFlag = (runningFlag || !runningFlags[i]);
    }
    OutputWriter::onResultsWritten();
}
