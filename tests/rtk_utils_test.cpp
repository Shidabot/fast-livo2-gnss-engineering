/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#include "rtk_utils.h"
#include <cassert>
#include <limits>

int main()
{
    using namespace rtk_utils;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    assert(validFix(22.0, 114.0, 10.0, 2200, 100.0, 0.02, 0.04));
    assert(!validFix(nan, 114.0, 10.0, 2200, 100.0, 0.02, 0.04));
    assert(!validFix(91.0, 114.0, 10.0, 2200, 100.0, 0.02, 0.04));
    assert(!validFix(22.0, 181.0, 10.0, 2200, 100.0, 0.02, 0.04));
    assert(!validFix(22.0, 114.0, 10.0, -1, 100.0, 0.02, 0.04));
    assert(!validFix(22.0, 114.0, 10.0, 2200, 604800.0, 0.02, 0.04));
    assert(!validFix(22.0, 114.0, 10.0, 2200, 100.0, 0.0, 0.04));
    assert(std::abs(variance(0.2, 0.01) - 0.04) < 1e-12);
    assert(variance(0.02, 0.01) == 0.01);
    assert(variance(nan, 0.01) == 0.01);
    assert(supportedTime(1.0, 0.0, 2.0, 2.0));
    assert(!supportedTime(-1.0, 0.0, 2.0, 2.0));
    assert(!supportedTime(3.0, 0.0, 2.0, 2.0));
    assert(!supportedTime(1.0, 0.0, 3.0, 2.0));
    assert(!supportedTime(nan, 0.0, 2.0, 2.0));
}
