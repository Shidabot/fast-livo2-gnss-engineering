/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#include "visual_utils.h"
#include <cassert>
#include <limits>

int main()
{
    using namespace visual_utils;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    assert(patchInBounds(100, 100, 640, 480, 4, 0));
    assert(!patchInBounds(nan, 100, 640, 480, 4, 0));
    assert(!patchInBounds(4, 100, 640, 480, 4, 0));
    assert(!patchInBounds(636, 100, 640, 480, 4, 0));
    assert(!patchInBounds(100, 476, 640, 480, 4, 0));
    assert(!patchInBounds(20, 100, 640, 480, 4, 3));
    assert(!patchInBounds(100, 100, 640, 480, 4, 31));
    // Every accepted patch must keep all central-difference and bilinear reads in bounds.
    for (int level = 0; level <= 6; ++level) {
        const int scale = 1 << level;
        for (int u = 0; u < 640; ++u) {
            for (int v = 0; v < 480; ++v) {
                if (!patchInBounds(u, v, 640, 480, 4, level)) continue;
                const int x = (u / scale) * scale, y = (v / scale) * scale;
                assert(x - 5 * scale >= 0 && x + 5 * scale < 640);
                assert(y - 5 * scale >= 0 && y + 5 * scale < 480);
            }
        }
    }
    assert(huberWeight(10, 20) == 1.0);
    assert(huberWeight(100, 20) == 0.2);
    assert(huberWeight(-100, 20) == 0.2);
    assert(huberCost(10, 20) == 100);
    assert(huberCost(100, 20) == 3600);
    assert(huberCost(20, 20) == 400);
    assert(nearestDepth(0, 10) == 10);
    assert(nearestDepth(2, 10) == 2);
    assert(nearestDepth(10, 2) == 2);
    assert(nearestDepth(2, -1) == 2);
    assert(nearestDepth(2, static_cast<float>(nan)) == 2);
}
