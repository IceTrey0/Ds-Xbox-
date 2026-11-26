#include "MelonGlue.h"
#include "../SaveCompat.h"
#include <filesystem>

#include "NDS.h"        // melonDS core
#include "Savestate.h"  // melonDS::Savestate

namespace mxb {

StateBackend MakeMelonBackend(melonDS::NDS& nds) {
    StateBackend b;

    b.serialize = [&nds](std::vector<uint8_t>& out) {
        // VERIFY: Savestate(u32 initialLength) growable constructor.
        melonDS::Savestate state(0x400000);
        if (state.Error) return false;                    // VERIFY: member name
        if (!nds.DoSavestate(&state)) return false;       // VERIFY: returns bool
        const uint8_t* p = static_cast<const uint8_t*>(state.Buffer()); // VERIFY
        out.assign(p, p + state.Length());                // VERIFY
        return true;
    };

    b.deserialize = [&nds](const std::vector<uint8_t>& in) {
        // VERIFY: Savestate(void* buffer, u32 len, bool save)
        melonDS::Savestate state(const_cast<uint8_t*>(in.data()),
                                 uint32_t(in.size()), false);
        if (state.Error) return false;
        return nds.DoSavestate(&state);
    };
    return b;
}

std::vector<uint8_t> SaveSink::Open(const std::string& romPath,
                                    const std::string& saveDir) {
    namespace fs = std::filesystem;
    std::string found = FindSaveFor(romPath, saveDir);
    // Always write back to .sav so the DraStic original stays untouched.
    path_ = (fs::path(saveDir) / (fs::path(romPath).stem().string() + ".sav")).string();
    if (!found.empty()) data_ = NormalizeSave(ReadFile(found));
    return data_;
}

void SaveSink::OnWrite(const uint8_t* savedata, uint32_t savelen) {
    data_.assign(savedata, savedata + savelen);
    dirty_ = true;
}

void SaveSink::Flush() {
    if (!dirty_ || data_.empty() || path_.empty()) return;
    if (WriteFileAtomic(path_, data_)) dirty_ = false;
}

} // namespace mxb
