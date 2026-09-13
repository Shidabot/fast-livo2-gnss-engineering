/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace rtk_utils {
inline bool validFix(double latitude, double longitude, double altitude,
                     int week, double tow, double h_acc, double v_acc)
{
    return std::isfinite(latitude) && std::abs(latitude) <= 90.0 &&
           std::isfinite(longitude) && std::abs(longitude) <= 180.0 &&
           std::isfinite(altitude) && week >= 0 && std::isfinite(tow) &&
           tow >= 0.0 && tow < 604800.0 &&
           std::isfinite(h_acc) && h_acc > 0.0 &&
           std::isfinite(v_acc) && v_acc > 0.0;
}

// PVT accuracy fields are distances in metres, not variances.
inline double variance(double accuracy, double floor)
{
    return std::isfinite(accuracy) && accuracy > 0.0
        ? std::max(floor, accuracy * accuracy) : floor;
}

inline bool supportedTime(double t, double before, double after, double max_gap)
{
    return std::isfinite(t) && std::isfinite(before) && std::isfinite(after) &&
           before <= t && t <= after && after >= before &&
           after - before <= max_gap;
}
} // namespace rtk_utils
