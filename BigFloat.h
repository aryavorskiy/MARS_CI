//
// Created by alexander on 08.11.2019.
//

#ifndef MARS_CI_BIGFLOAT_H
#define MARS_CI_BIGFLOAT_H

#include <cstdlib>
#include <cmath>

class BigFloat {
private:
    float mantissa;
    long exponent;

    void setupPrecision() {
        if (mantissa == 0 || std::isinf(mantissa)) {
            exponent = 0;
            return;
        }
        while (abs(mantissa) > 10) {
            mantissa /= 10;
            exponent += 1;
        }
        while (abs(mantissa) < 1) {
            mantissa *= 10;
            exponent -= 1;
        }
    }

    void add(BigFloat _bigFloat) {
        setupPrecision();
        _bigFloat.setupPrecision();
        if (_bigFloat.mantissa == 0)
            return;
        if (mantissa == 0) {
            mantissa = _bigFloat.mantissa;
            exponent = _bigFloat.exponent;
            return;
        }

        if (exponent < _bigFloat.exponent) {
            long expDiff = _bigFloat.exponent - exponent;
            mantissa /= exp10f(expDiff);
            exponent += expDiff;
        }
        mantissa += _bigFloat.mantissa * exp10f(_bigFloat.exponent - exponent);
        setupPrecision();
    }

    void multiplyBy(BigFloat _bigFloat) {
        mantissa *= _bigFloat.mantissa;
        exponent += _bigFloat.exponent;
        setupPrecision();
    }

public:
    explicit BigFloat(float _base = 0, int _exp = 0) {
        mantissa = _base;
        exponent = _exp;
        setupPrecision();
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

    explicit operator float() {
        return mantissa * exp10f(exponent);
    }

    BigFloat &operator=(float _float) {
        *this = BigFloat(_float);
        return *this;
    }

    bool operator==(float _float) { return this->equals(BigFloat(_float)); }

    BigFloat operator*(BigFloat _bigFloat) {
        this->multiplyBy(_bigFloat);
        return *this;
    }

    BigFloat operator*(float _float) {
        this->multiplyBy(BigFloat(_float));
        return *this;
    }

    BigFloat operator/(float _float) {
        this->multiplyBy(BigFloat(1 / _float));
        return *this;
    }

    void operator+=(float _float) { this->add(BigFloat(_float)); }

    void operator+=(BigFloat _bigFloat) { this->add(_bigFloat); }

    void operator-=(float _float) { this->add(BigFloat(-_float)); }

    void operator-=(BigFloat _bigFloat) { this->add(_bigFloat * -1); }

    void operator*=(float _float) { this->multiplyBy(BigFloat(_float)); }

    void operator*=(BigFloat _bigFloat) { this->multiplyBy(_bigFloat); }

    bool operator>(float _float) { return this->moreThan(BigFloat(_float)); }
};


#endif  //MARS_CI_BIGFLOAT_H
