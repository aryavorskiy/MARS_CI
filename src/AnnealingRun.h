//
// Created by alexander on 03.03.2020.
//

#ifndef MARS_CI_ANNEALINGRUN_H
#define MARS_CI_ANNEALINGRUN_H


#include "lib/Lattice.h"
#include "lib/Block.h"
#include "lib/Set.h"

template<typename T>
struct AnnealingRun {
    float temperature, temperature_step;
    BigFloat interaction_multiplier;
    const Lattice<T> &matrix;
    Block<T> block;
    int step_counter;

    AnnealingRun(Lattice<T> lattice);;

    Set<T> operator[](int index);

    void annealingStep();

    void anneal();
};

const float threshold = 0.001;

template<typename T>
Set<T> AnnealingRun<T>::operator[](int index) {
    return block[index];
}

template<typename T>
void AnnealingRun<T>::annealingStep() {
    bool proceed_iteration = true;
    while (proceed_iteration) {
        proceed_iteration = false;
        for (int set_index = 0; set_index < block.set_count; ++set_index) {
            for (int spin_index = 0; spin_index < block.set_size; ++spin_index) {
                T mean_field = block.meanField(matrix, set_index, spin_index, interaction_multiplier);
                T new_spin_value;
                if (temperature > 0)
                    new_spin_value = tanh(-mean_field / temperature);
                else
                    new_spin_value = mean_field > 0 ? -1 : 1;
                T old_spin_value = block[set_index][spin_index];
                if (fabs(new_spin_value - old_spin_value) > threshold)
                    proceed_iteration = true;
                block[set_index][spin_index] = new_spin_value;
                delete[] &new_spin_value;
                delete[] &mean_field;
            }
        }
    }
    step_counter++;
}

template<typename T>
void AnnealingRun<T>::anneal() {
    while (temperature > 0) {
        temperature -= temperature_step;
        annealingStep();
    }
}

template<typename T>
AnnealingRun<T>::AnnealingRun(Lattice<T> lattice) : matrix(lattice), step_counter(0) {}


#endif //MARS_CI_ANNEALINGRUN_H
