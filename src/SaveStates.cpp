#include "SaveStates.h"
#include "SaveCompat.h"
#include <cstring>
#include <filesystem>

namespace mxb {

static const char kMagic[4] = {'M', 'X', 'S', '1'};

static std::string SlotPath(const std::string& dir, const std::string& id, int slot) {
    return (std::filesystem::path(dir) / (id + ".slot" + std::to_string(slot) + ".mxs")).string();
}

bool SlotExists(const std::string& dir, const std::string& id, int slot) {
    return std::filesystem::exists(SlotPath(dir, id, slot));
}

bool SaveState(const StateBackend& b, const std::string& dir,
               const std::string& id, int slot) {
    if (slot < 0 || slot > 9 || !b.serialize) return false;
    std::vector<uint8_t> payload;
    if (!b.serialize(payload) || payload.empty()) return false;

    std::vector<uint8_t> out(kMagic, kMagic + 4);
    uint32_t idLen = uint32_t(id.size());
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&idLen);
    out.insert(out.end(), p, p + 4);
    out.insert(out.end(), id.begin(), id.end());
    out.insert(out.end(), payload.begin(), payload.end());

    std::filesystem::create_directories(dir);
    return WriteFileAtomic(SlotPath(dir, id, slot), out);
}

bool LoadState(const StateBackend& b, const std::string& dir,
               const std::string& id, int slot) {
    if (slot < 0 || slot > 9 || !b.deserialize) return false;
    auto in = ReadFile(SlotPath(dir, id, slot));
    if (in.size() < 8 || std::memcmp(in.data(), kMagic, 4) != 0) return false;
    uint32_t idLen;
    std::memcpy(&idLen, in.data() + 4, 4);
    if (in.size() < 8 + size_t(idLen)) return false;
    if (std::string(in.begin() + 8, in.begin() + 8 + idLen) != id) return false;
    return b.deserialize(std::vector<uint8_t>(in.begin() + 8 + idLen, in.end()));
}

} // namespace mxb
