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
#define BUILD 1;

/*
 * TERMINOLOGY:
 * Mat - describes J_ij lattice
 * Set - describes system mean spin values at certain state
 * Block - several sets which descend simultaneously and interact with each other
 * Link - A std::vector describing set interaction in a block
 */

using namespace std;
using namespace Annealing;

vector<string> matLoadModes{"RAND", "FILE_TABLE", "FILE_LIST"};
vector<string> hamiltonianModes{"LOG", "NO_LOG"};

int main() {
    cout << "MARS analysis by A. Yavorski, CPU edition, version " << VERSION
    cout << ", build " << BUILD
    cout << endl; // Wtf?! Doesn't work when in one line! FIXME

    // Load temperature bounds
    float tempStart = 10, tempFinal = 10, annealingStep = 0.01;
    cout << "Start temp?" << endl;
    cin >> tempStart;
    cout << "Final temp?" << endl;
    cin >> tempFinal;
    cout << "Annealing step?" << endl;
    cin >> annealingStep;

    // Load lattice
    string matLoadMode;
    cout << "Lattice type?" << endl;
    cin >> matLoadMode;
    while (find(matLoadModes.begin(), matLoadModes.end(), matLoadMode) == matLoadModes.end()) {
        cout << "Expected one of following: ";
        for (const string &loadMode : matLoadModes)
            cout << loadMode << ", ";
        cout << endl;
        cin >> matLoadMode;
    }
    float *J;
    string matFilename;
    switch ((int) (find(matLoadModes.begin(), matLoadModes.end(), matLoadMode) - matLoadModes.begin())) {
        case 0:
            // Random lattice
            cout << "Lattice size?" << endl;
            cin >> size;
            J = new float[size * size];
            srand(10);  // Invariant seed for easier testing
            matRandomize(J);
            break;
        case 1:
            // Load in table mode
            cout << "File path?" << endl;
            cin >> matFilename;
            J = InputLoader::loadMatFromTable(matFilename, &size);
            cout << "Lattice loaded, size: " << size << " (check). ";
            break;
        case 2:
            // Load in list mode
            cout << "File path?" << endl;
            cin >> matFilename;
            J = InputLoader::loadMatFromList(matFilename, &size);
            cout << "Lattice loaded, size: " << size << " (check). ";
            break;
    }

    // Load block configuration
    cout << "Thread quantity?" << endl;
    int threads;
    cin >> threads;
    cout << "Block file location (Enter block size to create a random block)?" << endl;
    string blockFilename;
    cin >> blockFilename;
    int blockSize = 0;
    float *Blocks;
    bool randomizeBlocks;
    try {
        blockSize = stoi(blockFilename);
        Blocks = new float[size * blockSize * threads];
        randomizeBlocks = true;
    } catch (exception &e) {
        auto ifs = ifstream(blockFilename);
        ifs >> blockSize;
        ifs.close();
        Blocks = new float[size * blockSize * threads];
        randomizeBlocks = false;
        cout << "Block loaded, size: " << blockSize << " (check). ";
    }
    int blockQty;
    cout << "Block quantity?" << endl;
    cin >> blockQty;

    // Load link configuration
    string linksFilename;
    vector<vector<int>> allLinks;
    cout << "Links file location (NONE for no interaction)?" << endl;
    cin >> linksFilename;
    if (linksFilename == "NONE")
        allLinks = vector<vector<int >>(size, vector<int>{});
    else
        allLinks = InputLoader::loadLinks(linksFilename);

    // Interaction quotient
    cout << "Interaction quotient (decimal log)?" << endl;
    float fQuotient;
    cin >> fQuotient;
    interactionQuotient = BigFloat(exp10f(fQuotient - (int) fQuotient), (int) fQuotient);

    // Hamiltonian mode: log or not log
    string hamiltonianMode;
    cout << "Hamiltonian mode?" << endl;
    cin >> hamiltonianMode;
    while (find(hamiltonianModes.begin(), hamiltonianModes.end(), hamiltonianMode) == hamiltonianModes.end()) {
        cout << "Expected one of following: ";
        for (const string &hamMode : hamiltonianModes)
            cout << hamMode << ", ";
        cout << endl;
        cin >> hamiltonianMode;
    }
    bool hamiltonianLog = hamiltonianModes[0] == hamiltonianMode;

    cout << "Temperature threshold?" << endl;
    cin >> Annealing::temperatureInteractionThreshold;

    // Enable/disable full log
    cout << "File to save all results (NONE for no saving)?" << endl;
    string resultsFileName;
    cin >> resultsFileName;
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
                        setRandomize(Blocks + size * blockSize * thrIndex + size * j);
                else
                    InputLoader::loadBlock(blockFilename, Blocks + size * blockSize * thrIndex, size);

                // Launch new run on a separate thread
                thread(anneal, J, Blocks + size * blockSize * thrIndex, blockSize,
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
