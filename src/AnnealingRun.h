//
// Created by alexander on 03.03.2020.
//

#ifndef MARS_CI_ANNEALINGRUN_H
#define MARS_CI_ANNEALINGRUN_H

#include "lib/Lattice.h"
#include "lib/Block.h"
#include "lib/Set.h"

/**
 * Represents a single run of the algorithm.
 * Contains all required information
 * @tparam T Spin and Lattice element value type
 */
template<typename T>
struct AnnealingRun {
    float temperature = 10, temperature_step = 1, temperature_threshold = 1;
    BigFloat interaction_multiplier = 0;
    const Lattice<T> &lattice;
    Block<T> block;
    int step_counter = 0;

    /**
     * Minimal AnnealingRun constructor.
     * @param lattice Lattice object
     */
    explicit AnnealingRun(Lattice<T> &lattice) : lattice(lattice) {}

    /**
     * Get a Set from the Block object.
     * @param index Set index in block
     * @return Set object
     */
    Set<T> operator[](int index);

    /**
     * Perform a single annealing step so that the spin values correspond the mean-field equation.
     */
    void annealingStep();

    /**
     * Perform a full annealing operation.
     */
    void anneal();
};

constexpr float threshold = 0.001;

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
            // Update stored probability values
            for (int link_index = 0; link_index < block[set_index].linkedSets(); ++link_index)
                block[set_index].recalculateProbabilities(link_index);

            for (int spin_index = 0; spin_index < block.setSize(); ++spin_index) {
                // Calculate mean field
                BigFloat mean_field{0};
                if (temperature > temperature_threshold and temperature > 0)
                    mean_field = block[set_index].meanField(spin_index, lattice, interaction_multiplier);
                else
                    mean_field = block[set_index].meanField(spin_index, lattice, BigFloat(0));

                // Calculate new spin value
                T new_spin_value;
                if (temperature > 0)
                    new_spin_value = tanh((T) (mean_field / -temperature));
                else
                    new_spin_value = mean_field > 0 ? -1 : 1;

                // Check threshold
                T old_spin_value = block[set_index][spin_index];
                if (fabs(new_spin_value - old_spin_value) > threshold)
                    proceed_iteration = true;

                // Write spin value
                block.setSpin(set_index, spin_index, new_spin_value);
            }
        }
        step_counter++;
    }
}

template<typename T>
void AnnealingRun<T>::anneal() {
    while (temperature > 0) {
        temperature -= temperature_step;
        annealingStep();
    }
}


#endif //MARS_CI_ANNEALINGRUN_H
