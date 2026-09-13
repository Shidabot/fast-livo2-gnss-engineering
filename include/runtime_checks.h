/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace runtime_checks {
inline void require(bool condition, const std::string &message) {
    if (!condition) throw std::invalid_argument(message);
}
inline void positive(double value, const std::string &name) {
    require(std::isfinite(value) && value > 0.0, name + " must be finite and positive");
}
inline void finiteVector(const std::vector<double> &v, size_t size, const std::string &name) {
    require(v.size() == size, name + " has incorrect length");
    for (double x : v) require(std::isfinite(x), name + " contains non-finite values");
}
inline void rotation(const std::vector<double> &r, const std::string &name) {
    finiteVector(r, 9, name);
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
        double dot = 0.0;
        for (int k = 0; k < 3; ++k) dot += r[k*3+i] * r[k*3+j];
        require(std::abs(dot - (i == j ? 1.0 : 0.0)) < 1e-3, name + " must be orthonormal");
    }
    const double det = r[0]*(r[4]*r[8]-r[5]*r[7]) - r[1]*(r[3]*r[8]-r[5]*r[6]) + r[2]*(r[3]*r[7]-r[4]*r[6]);
    require(std::abs(det - 1.0) < 1e-3, name + " must have determinant +1");
}
inline bool validInterval(double begin, double end) {
    return std::isfinite(begin) && std::isfinite(end) && begin >= 0.0 && end >= begin;
}
}
