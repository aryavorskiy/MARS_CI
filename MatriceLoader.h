//
// Created by alexander on 10/3/19.
//

#ifndef MARS_2_MATRICELOADER_H
#define MARS_2_MATRICELOADER_H

using namespace std;

class MatriceLoader {
public:
    static float *loadAsMatrice(string filename, int *size);

    float *loadAsLattice(string filename, int *size);
};


#endif //MARS_2_MATRICELOADER_H
