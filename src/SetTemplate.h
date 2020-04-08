//
// Created by alexander on 13.03.2020.
//

#ifndef MARS_CI_SETTEMPLATE_H
#define MARS_CI_SETTEMPLATE_H

#include <sstream>
#include "lib/Set.h"

/**
 * Represents a structure that generates Set object from an arbitrary source.
 * This class is abstract, subclasses provide functionality.
 * @tparam T Spin value type
 */
template<typename T>
class SetTemplate {
public:
    /**
     * Create a Set object that matches the template represented by this.
     * @return Set object
     */
    virtual Set<T> instance() = 0;
};

/**
 * Represents a template of a set initialized by a string.
 * @tparam T Spin value type
 */
template<typename T>
class GivenSetTemplate : public SetTemplate<T> {
private:
    int set_size = 0;
    T *set_values = nullptr;

public:
    /**
     * GivenSetTemplate constructor.
     * @param size Spin count in set
     * @param line String with spin values
     */
    GivenSetTemplate(int size, const std::string &line) : set_size(size), set_values(new T[size]) {
        auto in = std::istringstream(line);
        for (int i = 0; i < set_size; ++i) {
            T buf;
            in >> buf;
            set_values[i] = buf;
        }
    }

    /**
     * Create a Set object that matches the template represented by this.
     * @return Set object
     */
    Set<T> instance() {
        return Set<T>(set_size, set_values, UNDEFINED);
    }
};

template<typename T>
class RandomSetTemplate : public SetTemplate<T> {
private:
    int set_size = 0;
public:
    /**
     * RandomSetTemplate constructor.
     * @param size Spin count in set
     */
    explicit RandomSetTemplate(int size) : set_size(size) {};

    /**
     * Create a Set object that matches the template represented by this.
     * @return Set object
     */
    Set<T> instance() {
        T *set_values = new T[set_size];
        for (int i = 0; i < set_size; ++i)
            set_values[i] = Random::uniform(-1, 1);
        return Set<T>(set_size, set_values, UNDEFINED);
    }
};

#endif //MARS_CI_SETTEMPLATE_H
