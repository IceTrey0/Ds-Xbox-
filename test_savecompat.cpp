#include "../src/SaveCompat.h"
#include "../src/SaveStates.h"
#include <cassert>
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
    std::puts("all tests passed");
    return 0;
}
