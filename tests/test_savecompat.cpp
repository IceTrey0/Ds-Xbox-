#include "../src/SaveCompat.h"
#include "../src/SaveStates.h"
#include "../src/TouchCursor.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

int main() {
    using namespace mxb;
    std::vector<uint8_t> raw(8192, 0xAB);
    assert(NormalizeSave(raw) == raw);

    std::vector<uint8_t> dsv = raw;
    const char* m = "|<--Snip above here to create a raw sav by excluding this DeSmuME savedata footer:";
    dsv.insert(dsv.end(), m, m + std::strlen(m));
    dsv.resize(dsv.size() + 40, 0);
    assert(NormalizeSave(dsv) == raw);

    std::vector<uint8_t> junk(1234, 1);
    assert(NormalizeSave(junk).empty());

    std::vector<uint8_t> state;
    StateBackend be{
        [&](std::vector<uint8_t>& o) { o = {1, 2, 3, 4}; return true; },
        [&](const std::vector<uint8_t>& i) { state = i; return true; }};
    assert(SaveState(be, "mxs_test", "NTRJ-0001", 0));
    assert(LoadState(be, "mxs_test", "NTRJ-0001", 0));
    assert(state.size() == 4);
    assert(!LoadState(be, "mxs_test", "OTHER-GAME", 0));

    // --- touch cursor ---
    {
        TouchCursor c;
        c.Update(0.1f, 0.1f, 1.0f);              // inside deadzone
        assert(c.X() == 128.0f && c.Y() == 96.0f);

        c.Update(1.0f, 0.0f, 0.1f);              // full right, 0.1s
        assert(std::fabs(c.X() - (128.0f + 20.0f)) < 0.01f);
        assert(c.Y() == 96.0f);

        TouchCursor u;
        u.Update(0.0f, 1.0f, 0.1f);              // stick up -> y decreases
        assert(u.Y() < 96.0f);

        TouchCursor s;
        s.Update(1.0f, 0.0f, 0.1f, true);        // slow mode moves less
        assert(s.X() < c.X());

        TouchCursor w;
        for (int i = 0; i < 100; i++) w.Update(1.0f, -1.0f, 0.5f);
        assert(w.X() == 255.0f && w.Y() == 191.0f);   // clamped to screen

        auto o = w.Output(true);
        assert(o.down && o.x == 255 && o.y == 191);
    }

    std::puts("all tests passed");
    return 0;
}
