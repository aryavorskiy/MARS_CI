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
 * Represents a floating-point variable with upper bound about 1e(2^32).
 */
class BigFloat {
private:
    double mantissa;
    long exponent;

    void setupPrecision() {
        if (mantissa == 0 || std::isinf(mantissa)) {
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

    BigFloat add(BigFloat _bigFloat) {
        setupPrecision();
        _bigFloat.setupPrecision();
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

    BigFloat multiplyBy(BigFloat _bigFloat) {
        mantissa *= _bigFloat.mantissa;
        exponent += _bigFloat.exponent;
        setupPrecision();
        return *this;
    }

    BigFloat oneDivideByThis() {
        mantissa = 1. / mantissa;
        exponent *= -1;
        setupPrecision();
        return *this;
    }


    bool equals(BigFloat _bigFloat) {
        setupPrecision();
        _bigFloat.setupPrecision();
        return _bigFloat.mantissa == mantissa && _bigFloat.exponent == exponent;
    }

    bool moreThan(BigFloat _bigFloat) {
        setupPrecision();
        _bigFloat.setupPrecision();
        return mantissa * _bigFloat.mantissa <= 0 ? mantissa > _bigFloat.mantissa :
               exponent > _bigFloat.exponent ? true :
               _bigFloat.exponent > exponent ? false : mantissa > _bigFloat.mantissa;
    }


    BigFloat getInstance() {
        BigFloat bf{0};
        bf.mantissa = mantissa;
        bf.exponent = exponent;
        bf.setupPrecision();
        return bf;
    }

public:
    BigFloat(double _base, double _exp = 0) {
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
        oss << mantissa << "E" << exponent;
        return oss.str();
    }

    BigFloat &operator=(float _float) {
        *this = BigFloat(_float);
        return *this;
    }

    bool operator==(BigFloat _bigFloat) { return this->equals(_bigFloat); }

    bool operator!=(BigFloat _bigFloat) { return not(*this == _bigFloat); }

    BigFloat operator+(BigFloat _bigFloat) { return this->getInstance().add(_bigFloat); }

    BigFloat operator*(BigFloat _bigFloat) { return this->getInstance().multiplyBy(_bigFloat); }

    BigFloat operator/(BigFloat _bigFloat) {
        return this->getInstance().multiplyBy(_bigFloat.getInstance().oneDivideByThis());
    }

    void operator+=(BigFloat _bigFloat) { this->add(_bigFloat); }

    void operator-=(BigFloat _bigFloat) { this->add(_bigFloat * -1); }

    void operator*=(BigFloat _bigFloat) { this->multiplyBy(_bigFloat); }

    bool operator>(BigFloat _bigFloat) { return this->moreThan(_bigFloat); }

    bool operator<(BigFloat _bigFloat) { return not(*this == _bigFloat) and not(*this > _bigFloat); }
};


#endif  //MARS_CI_BIGFLOAT_H
