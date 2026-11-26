#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace mxb {

// Glue to the emulator core. Implement these two lambdas with melonDS's
// Savestate class (check the API of the melonDS version you pin).
struct StateBackend {
    std::function<bool(std::vector<uint8_t>& out)> serialize;
    std::function<bool(const std::vector<uint8_t>& in)> deserialize;
};

// 10 slots per game, stored as <dir>/<gameId>.slotN.mxs
// Header = magic + game id, so a state can't load into the wrong ROM.
bool SaveState(const StateBackend&, const std::string& dir,
               const std::string& gameId, int slot);
bool LoadState(const StateBackend&, const std::string& dir,
               const std::string& gameId, int slot);
bool SlotExists(const std::string& dir, const std::string& gameId, int slot);

} // namespace mxb
