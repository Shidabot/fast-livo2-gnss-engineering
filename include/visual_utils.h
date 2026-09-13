/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#pragma once
#include <cmath>

namespace visual_utils {
// Includes bilinear interpolation and central-difference gradient neighbours.
inline bool patchInBounds(double u, double v, int cols, int rows,
                          int half_patch, int level)
{
    if (!std::isfinite(u) || !std::isfinite(v) || level < 0 || level > 12 ||
        half_patch < 1 || cols <= 0 || rows <= 0 || u < 0.0 || v < 0.0 ||
        u >= cols || v >= rows) return false;
    const double scale = static_cast<double>(1 << level);
    const double x = std::floor(u / scale) * scale;
    const double y = std::floor(v / scale) * scale;
    return x - (half_patch + 1.0) * scale >= 0.0 &&
           y - (half_patch + 1.0) * scale >= 0.0 &&
           x + (half_patch + 1.0) * scale < cols &&
           y + (half_patch + 1.0) * scale < rows;
}
inline double huberWeight(double residual, double delta)
{
    return std::abs(residual) <= delta ? 1.0 : delta / std::abs(residual);
}
inline double huberCost(double residual, double delta)
{
    const double a = std::abs(residual);
    return a <= delta ? residual * residual : 2.0 * delta * a - delta * delta;
}
inline float nearestDepth(float current, float candidate)
{
    if (!std::isfinite(candidate) || candidate <= 0.0f) return current;
    return current <= 0.0f || candidate < current ? candidate : current;
}
} // namespace visual_utils
