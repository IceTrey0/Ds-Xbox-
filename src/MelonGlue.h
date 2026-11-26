#pragma once
// Compiled only when MXB_WITH_MELONDS=ON. Written against melonDS tag 1.0.
// Every line marked VERIFY is from memory of that API; CI compiles this
// in an allowed-to-fail job so you can see what needs adjusting.
#include "../SaveStates.h"
#include <cstdint>
#include <string>
#include <vector>

namespace melonDS { class NDS; }

namespace mxb {

// Save states: wraps melonDS::Savestate around NDS::DoSavestate.
StateBackend MakeMelonBackend(melonDS::NDS& nds);

// Battery saves. melonDS calls Platform::WriteNDSSave() when the game
// writes to its save chip; the frontend forwards that call here.
class SaveSink {
public:
    // Loads <saveDir>/<rom>.sav or .dsv (DraStic) via NormalizeSave().
    // Returns raw bytes to hand to the cart as its initial SRAM.
    std::vector<uint8_t> Open(const std::string& romPath, const std::string& saveDir);

    // Call from Platform::WriteNDSSave(savedata, savelen, off, len, ud).
    void OnWrite(const uint8_t* savedata, uint32_t savelen);

    // Call every ~1s and on exit. Always writes raw .sav (never .dsv).
    void Flush();

private:
    std::string path_;
    std::vector<uint8_t> data_;
    bool dirty_ = false;
};

} // namespace mxb
