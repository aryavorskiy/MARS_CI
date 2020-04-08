//
// Created by alexander on 08.11.2019.
//

#ifndef MARS_CI_BIGFLOAT_H
#define MARS_CI_BIGFLOAT_H

#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <sstream>

/**
 * Represents a floating-point variable with upper bound about 1e1000000000.
 */
class BigFloat {
private:
    double mantissa;
    long exponent;

    void setupPrecision();

    BigFloat add(BigFloat _bigFloat);

    BigFloat multiplyBy(BigFloat _bigFloat);

    BigFloat oneDivideByThis();

    bool equals(BigFloat _bigFloat);

    bool moreThan(BigFloat _bigFloat);

    BigFloat getCopy();

public:
    BigFloat(double _base, double _exp = 0) {
        if (_base == 0) {
            mantissa = _base;
            exponent = 0;
            return;
        }
        mantissa = _base * exp10(_exp - (double) (long) _exp);
        exponent = (long) _exp;
        setupPrecision();
    }

    double log() { return (double) exponent + std::log(mantissa); }

    explicit operator float() {
        this->setupPrecision();
        if (exponent >= 38)  // Prevent overflow
            return FLT_MAX * (float) (mantissa > 0 ? 1 : -1);
        return (float) mantissa * exp10f((float) exponent);
    }

    explicit operator double() {
        this->setupPrecision();
        if (exponent >= 308)  // Prevent overflow
            return DBL_MAX * (double) (mantissa > 0 ? 1 : -1);
        return mantissa * exp10((double) exponent);
    }

    explicit operator std::string() {
        std::ostringstream oss;
        oss << mantissa << "e" << exponent;
        return oss.str();
    }

    BigFloat &operator=(float _float) {
        *this = BigFloat(_float);
        return *this;
    }

    bool operator==(BigFloat _bigFloat) { return this->equals(_bigFloat); }

    bool operator!=(BigFloat _bigFloat) { return not(*this == _bigFloat); }

    BigFloat operator+(BigFloat _bigFloat) { return this->getCopy().add(_bigFloat); }

    BigFloat operator*(BigFloat _bigFloat) { return this->getCopy().multiplyBy(_bigFloat); }

    BigFloat operator/(BigFloat _bigFloat) {
        return this->getCopy().multiplyBy(_bigFloat.getCopy().oneDivideByThis());
    }

    void operator+=(BigFloat _bigFloat) { this->add(_bigFloat); }

    void operator-=(BigFloat _bigFloat) { this->add(_bigFloat * -1); }

    void operator*=(BigFloat _bigFloat) { this->multiplyBy(_bigFloat); }

    bool operator>(BigFloat _bigFloat) { return this->moreThan(_bigFloat); }

    bool operator<(BigFloat _bigFloat) { return not(*this == _bigFloat) and not(*this > _bigFloat); }
};

void BigFloat::setupPrecision() {
    if (mantissa == 0 or std::isinf(mantissa)) {
        exponent = 0;
        return;
    }
    while (std::fabs(mantissa) > 10) {
        mantissa /= 10;
        exponent += 1;
    }
    while (std::fabs(mantissa) < 1) {
        mantissa *= 10;
        exponent -= 1;
    }
}

BigFloat BigFloat::add(BigFloat _bigFloat) {
    if (_bigFloat.mantissa == 0)
        return *this;
    if (mantissa == 0) {
        mantissa = _bigFloat.mantissa;
        exponent = _bigFloat.exponent;
        return *this;
    }

    if (exponent < _bigFloat.exponent) {
        long expDiff = _bigFloat.exponent - exponent;
        mantissa /= exp10f((float) expDiff);
        exponent += expDiff;
    }
    mantissa += _bigFloat.mantissa * exp10f((float) (_bigFloat.exponent - exponent));
    setupPrecision();
    return *this;
}

BigFloat BigFloat::multiplyBy(BigFloat _bigFloat) {
    mantissa *= _bigFloat.mantissa;
    exponent += _bigFloat.exponent;
    setupPrecision();
    return *this;
}

BigFloat BigFloat::oneDivideByThis() {
    mantissa = 1. / mantissa;
    exponent *= -1;
    setupPrecision();
    return *this;
}

bool BigFloat::equals(BigFloat _bigFloat) {
    return _bigFloat.mantissa == mantissa and _bigFloat.exponent == exponent;
}

bool BigFloat::moreThan(BigFloat _bigFloat) {
    return mantissa * _bigFloat.mantissa <= 0 ? mantissa > _bigFloat.mantissa :
           exponent > _bigFloat.exponent ? true :
           _bigFloat.exponent > exponent ? false : mantissa > _bigFloat.mantissa;
}

BigFloat BigFloat::getCopy() {
    BigFloat bf{0};
    bf.mantissa = mantissa;
    bf.exponent = exponent;
    return bf;
}

#endif  //MARS_CI_BIGFLOAT_H
