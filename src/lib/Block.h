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
class Block {
    /**
     * A SetLink object contains indices of sets that interact with the given one.
     * The links object contains a SetLink for every set in block.
     */
    typedef std::vector<int> SetLink;
private:
    Set<T> *sets = nullptr;

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
     * Get Set from specified index.
     * @param index Index of set in block
     * @return Set object
     */
    Set<T> &operator[](int index);

    /**
     * Set spin value of specified spin and perform additional actions if needed.
     * @param set_index Index of set in block
     * @param spin_index Index of spin in set
     * @param spin_value Value to write
     */
    void setSpin(int set_index, int spin_index, T spin_value);

    /**
     * Get spin count in sets in block.
     * @return Spin count
     */
    int setSize();
};

template<typename T>
Block<T>::Block(int set_count, Set<T> *sets, Block::SetLink *links) :
        sets(sets), set_count(set_count) {
    for (int set_index = 0; set_index < this->set_count; ++set_index) {
        SetType set_type;
        if (links[set_index].empty())
            set_type = INDEPENDENT;
        else if (links[set_index][0] == -1)
            set_type = NO_ANNEAL;
        else
            set_type = DEPENDENT;
        sets[set_index].set_type = set_type;
        for (int link_index : links[set_index]) {
            sets[set_index].createLink(sets[link_index]);
        }
    }
}

template<typename T>
Set<T> &Block<T>::operator[](int index) {
    return sets[index];
}

template<typename T>
void Block<T>::setSpin(int set_index, int spin_index, T spin_value) {
    sets[set_index].setSpin(spin_index, spin_value);
}

template<typename T>
int Block<T>::setSize() {
    return sets[0].size();
}

#endif //MARS_CI_BLOCK_H
