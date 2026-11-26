#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace mxb {

bool IsValidSaveSize(size_t n);

// Accepts raw .sav, DeSmuME-style .dsv (with footer), or a .dsv with
// trailing junk, and returns a raw save image. Empty vector on failure.
std::vector<uint8_t> NormalizeSave(const std::vector<uint8_t>& in);

std::vector<uint8_t> ReadFile(const std::string& path);
bool WriteFileAtomic(const std::string& path, const std::vector<uint8_t>& data);

// Looks for <saveDir>/<rom stem>.sav, then .dsv. Returns "" if none.
std::string FindSaveFor(const std::string& romPath, const std::string& saveDir);

} // namespace mxb
