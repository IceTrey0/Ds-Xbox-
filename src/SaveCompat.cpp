#include "SaveCompat.h"
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace fs = std::filesystem;

namespace mxb {

static const size_t kSizes[] = {
    512, 8192, 32768, 65536, 131072, 262144, 524288, 1048576,
    2097152, 4194304, 8388608, 16777216, 33554432};

bool IsValidSaveSize(size_t n) {
    return std::find(std::begin(kSizes), std::end(kSizes), n) != std::end(kSizes);
}

std::vector<uint8_t> NormalizeSave(const std::vector<uint8_t>& in) {
    if (IsValidSaveSize(in.size())) return in;

    // DeSmuME footer marker (also seen in some .dsv files)
    static const char marker[] = "|<--Snip above here to create a raw sav";
    auto it = std::search(in.begin(), in.end(), marker, marker + sizeof(marker) - 1);
    if (it != in.end()) {
        size_t n = size_t(it - in.begin());
        if (IsValidSaveSize(n)) return std::vector<uint8_t>(in.begin(), in.begin() + n);
    }

    // Fallback: small trailing footer of unknown format after a valid size.
    for (size_t i = sizeof(kSizes) / sizeof(kSizes[0]); i-- > 0;) {
        if (kSizes[i] < in.size()) {
            if (in.size() - kSizes[i] <= 512)
                return std::vector<uint8_t>(in.begin(), in.begin() + kSizes[i]);
            break;
        }
    }
    return {};
}

std::vector<uint8_t> ReadFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

bool WriteFileAtomic(const std::string& path, const std::vector<uint8_t>& data) {
    std::string tmp = path + ".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f) return false;
        f.write(reinterpret_cast<const char*>(data.data()), std::streamsize(data.size()));
        if (!f) return false;
    }
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        fs::remove(path, ec);
        ec.clear();
        fs::rename(tmp, path, ec);
    }
    return !ec;
}

std::string FindSaveFor(const std::string& romPath, const std::string& saveDir) {
    fs::path stem = fs::path(romPath).stem();
    for (const char* ext : {".sav", ".dsv"}) {
        fs::path p = fs::path(saveDir) / (stem.string() + ext);
        if (fs::exists(p)) return p.string();
    }
    return "";
}

} // namespace mxb
