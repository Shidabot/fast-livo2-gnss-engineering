/*
Engineering fork maintainer: shida <shida.86@outlook.com>
Local maintenance notice added: 2026-09-13
Derived from FAST-LIVO2 and the inherited FAST-LIVO2-RTK extension.
Original upstream authorship and license notices are preserved below.
*/

#include "runtime_checks.h"
#include <cassert>
#include <limits>

template<class F> void rejects(F check) {
    bool rejected = false;
    try { check(); } catch (const std::invalid_argument &) { rejected = true; }
    assert(rejected);
}
int main() {
    using namespace runtime_checks;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    finiteVector({1, 2, 3}, 3, "translation");
    rejects([] { finiteVector({}, 3, "translation"); });
    rejects([&] { finiteVector({0, nan, 0}, 3, "translation"); });
    rotation({1,0,0,0,1,0,0,0,1}, "rotation");
    rotation({0,-1,0,1,0,0,0,0,1}, "rotation");
    rejects([] { rotation({1,0,0,0,1,0,0,0,-1}, "reflection"); });
    rejects([] { rotation({2,0,0,0,1,0,0,0,1}, "scaled"); });
    rejects([] { positive(0, "noise"); });
    rejects([&] { positive(nan, "noise"); });
    assert(validInterval(1, 2));
    assert(validInterval(1, 1));
    assert(!validInterval(-1, 2));
    assert(!validInterval(2, 1));
    assert(!validInterval(nan, 2));
}
