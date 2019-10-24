#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

#include "InputLoader.h"
#include "Annealing.h"

#define VERSION "2.6";
#define BUILD 8;

/*
 * TERMINOLOGY:
 * Mat - describes J_ij lattice
 * Set - describes system mean spin values at certain state
 * Block - several sets which descend simultaneously and interact with each other
 * Link - A std::vector describing set interaction in a block
 */

using namespace std;
using namespace Annealing;

vector<string> matLoadModeStr{"RAND", "FILE_TABLE", "FILE_LIST"};

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
    string loadModeStr;
    cout << "Lattice type?" << endl;
    cin >> loadModeStr;
    while (find(matLoadModeStr.begin(), matLoadModeStr.end(), loadModeStr) == matLoadModeStr.end()) {
        cout << "Expected one of following: ";
        for (const string &loadMode : matLoadModeStr)
            cout << loadMode << ", ";
        cout << endl;
        cin >> loadModeStr;
    }
    float *J;
    string filename;
    switch ((int) (find(matLoadModeStr.begin(), matLoadModeStr.end(), loadModeStr) - matLoadModeStr.begin())) {
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
            cin >> filename;
            J = InputLoader::loadMatFromTable(filename, &size);
            cout << "Lattice loaded, size: " << size << " (check). ";
            break;
        case 2:
            // Load in list mode
            cout << "File path?" << endl;
            cin >> filename;
            J = InputLoader::loadMatFromList(filename, &size);
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
    cin >> interactionQuotient;

    // Enable/disable full log
    cout << "File to save all results (NONE for no saving)?" << endl;
    string resultsFileName;
    cin >> resultsFileName;
    if (resultsFileName != "NONE")
        Annealing::setUpResultWriting(resultsFileName);

    // Finally, start annealing
    int *glExpExternal = new int[threads];
    bool *runningFlags = new bool[threads];
    for (int thrIndex = 0; thrIndex < threads; ++thrIndex)
        runningFlags[thrIndex] = true;
    int launchedThrCount = 0;
    while (launchedThrCount < blockQty)
        for (int thrIndex = 0; thrIndex < threads; thrIndex++)
            if (runningFlags[thrIndex] && launchedThrCount < blockQty) {
                runningFlags[thrIndex] = false;
                if (randomizeBlocks)
                    for (int j = 0; j < blockSize; ++j)
                        setRandomize(Blocks + size * blockSize * thrIndex + size * j);
                else
                    InputLoader::loadBlock(blockFilename, Blocks + size * blockSize * thrIndex, size);

                // Launch new run on a separate thread
                thread(anneal, J, Blocks + size * blockSize * thrIndex, blockSize,
                       tempStart + ((float) launchedThrCount / (float) blockQty) * (tempFinal - tempStart),
                       annealingStep, runningFlags + thrIndex, glExpExternal + thrIndex, allLinks).detach();
                launchedThrCount++;
            }

    bool runningFlag = true;
    while (runningFlag) {
        runningFlag = false;
        for (int i = 0; i < threads; i++)
            runningFlag = (runningFlag || !runningFlags[i]);
    }
    Annealing::onResultsWritten();
}