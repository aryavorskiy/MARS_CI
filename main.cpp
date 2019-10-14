#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

#include "InputLoader.h"
#include "MultiThreadedAnnealing.h"

#define VERSION "2.5";
#define BUILD 12;

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

string getTimeString(double time) {
    if (time <= 0)
        return "0 h 0 m 0 s";
    ostringstream oss;
    int d = (int) (time / (24 * 3600));
    int h = (int) ((time - d * 24 * 3600) / 3600);
    int m = (int) ((time - d * 24 * 3600 - h * 3600) / 60);
    int s = (int) (time - d * 24 * 3600 - h * 3600 - m * 60);
    if (d != 0) {
        oss << d << " d ";
    }
    oss << h << " h " << m << " m " << s << " s";
    return oss.str();
}

int main() {
    cout << "MARS analysis by A. Yavorski, CPU edition, version " << VERSION
    cout << ", build " << BUILD
    cout << endl;
    float tempStart = 10;
    float tempFinal = 10;
    cout << "Start temp?" << endl;
    cin >> tempStart;
    cout << "Final temp?" << endl;
    cin >> tempFinal;
    float annealingStep = 0.01;
    cout << "Annealing step?" << endl;
    cin >> annealingStep;
    cout << "Lattice type?" << endl;
    string loadModeStr;
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
            srand(10);
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

    cout << "Block quantity?" << endl;
    int blockQty;
    cin >> blockQty;

    cout << "Links file location (NONE for no interaction)?" << endl;
    string linksFilename;
    cin >> linksFilename;
    vector<vector<int>> allLinks;
    if (linksFilename == "NONE")
        allLinks = vector<vector<int>>(size, vector<int>{});
    else
        allLinks = InputLoader::loadLinks(linksFilename);

    cout << "Interaction quotient (decimal log)?" << endl;
    cin >> interactionQuotient;

    cout << "File to save all results (NONE for no saving)?" << endl;
    string resultsFileName;
    cin >> resultsFileName;
    if (resultsFileName != "NONE")
        Annealing::setUpResultWriting(resultsFileName);

    int *glExpExternal = new int[threads];
    bool *f = new bool[threads];
    for (int i = 0; i < threads; ++i)
        f[i] = true;
    int launchedThrCount = 0;
    double start_time = time(nullptr);
    while (launchedThrCount < blockQty)
        for (int thrIndex = 0; thrIndex < threads; thrIndex++)
            if (f[thrIndex] && launchedThrCount < blockQty) {
                f[thrIndex] = false;
                if (randomizeBlocks)
                    for (int j = 0; j < blockSize; ++j)
                        setRandomize(Blocks + size * blockSize * thrIndex + size * j);
                else
                    InputLoader::loadBlock(blockFilename, Blocks + size * blockSize * thrIndex, size);

                // Launch new run on a separate thread
                thread(anneal, J, &(Blocks[size * blockSize * thrIndex]), blockSize,
                       tempStart + (launchedThrCount / (float) blockQty) * (tempFinal - tempStart),
                       annealingStep, f + thrIndex, glExpExternal + thrIndex, allLinks).detach();
                launchedThrCount++;
            }

    bool flag_wait = true;
    while (flag_wait) {
        flag_wait = false;
        for (int i = 0; i < threads; i++)
            flag_wait = (flag_wait || !f[i]);
    }
    Annealing::onResultsWritten();
    cout << "Calculations complete in " << getTimeString(time(nullptr) - start_time) << endl;
}