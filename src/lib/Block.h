//
// Created by aryavorskiy on 02.03.2020.
//

#ifndef MARS_CI_BLOCK_H
#define MARS_CI_BLOCK_H

#include <cassert>
#include <string>
#include <vector>

#include "Lattice.h"
#include "BigFloat.h"
#include "Set.h"

/**
 * Represents a union of several spin sets that interact with each other.
 * Set interaction is described by the links field.
 * @tparam T Spin value type
 */
template<typename T>
struct Block {
    /**
     * A set_link object contains indices of sets that interact with the given one.
     * The links object contains a set_link for every set in block.
     */
    typedef std::vector<int> SetLink;
private:
    Set<T> *sets = nullptr;
    SetLink *links = nullptr;

public:
    int set_count = 0;

    /**
     * Default Block constructor
     */
    Block() = default;

    /**
     * Block constructor.
     * @param set_size Quantity of spins in sets
     * @param set_count Quantity of sets in block
     * @param block_values Spin values array pointer
     * @param links Link array pointer
     */
    Block(int set_count, Set<T> *sets, SetLink *links);;

    /**
     * Calculates mean field value for specified spin in specified set
     * @param lattice Lattice describing spin interactions
     * @param set_index Index of set in block
     * @param spin_index Index of spin in set
     * @param interaction_multiplier Number describing set interaction
     * @return Mean field value
     */
    BigFloat meanField(Lattice<T> lattice, int set_index, int spin_index, BigFloat interaction_multiplier);

    /**
     * Get Set from specified index.
     * @param index Index of set in block
     * @return Set object
     */
    Set<T> operator[](int index);

    /**
     * Set spin value of specified spin and perform additional actions if needed.
     * @param set_index Index of set in block
     * @param spin_index Index of spin in set
     * @param spin_value Value to write
     */
    void SetSpin(int set_index, int spin_index, T spin_value);

    /**
     * Get spin count in sets in block.
     * @return Spin count
     */
    int set_size();
};

template<typename T>
Block<T>::Block(int set_count, Set<T> *sets, Block::SetLink *links) :
        sets(sets), links(links), set_count(set_count) {
    for (int set_index = 0; set_index < this->set_count; ++set_index) {
        SetType set_type;
        if (links[set_index].empty())
            set_type = INDEPENDENT;
        else if (links[set_index][0] == -1)
            set_type = NO_ANNEAL;
        else
            set_type = DEPENDENT;
        sets[set_index].set_type = set_type;
    }
}


template<typename T>
BigFloat Block<T>::meanField(Lattice<T> matrix, int set_index, int spin_index, BigFloat interaction_multiplier) {
    assert(matrix.size() == this->set_size());
    BigFloat mean_field{};

    // Set interaction in block
    if (interaction_multiplier != 0) {
        for (int link_element : links[set_index]) {
            mean_field += interaction_multiplier * (float) (0.5 * logf(
                    (1 + sets[link_element][spin_index]) /
                    (1 - sets[link_element][spin_index])
            ));
        }
    }

    // Spin interaction in set
    for (int i = 0; i < sets[set_index].size(); ++i) {
        if (i != spin_index)
            mean_field += sets[set_index][i] * matrix(i, spin_index);
    }
    return mean_field;
}

template<typename T>
Set<T> Block<T>::operator[](int index) {
    return sets[index];
}

template<typename T>
void Block<T>::SetSpin(int set_index, int spin_index, T spin_value) {
    sets[set_index][spin_index] = spin_value;
}

template<typename T>
int Block<T>::set_size() {
    return sets[0].size();
}

#endif //MARS_CI_BLOCK_H
