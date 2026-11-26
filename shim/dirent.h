#pragma once
// Minimal POSIX dirent for Windows/UWP (opendir/readdir/closedir).
#include <windows.h>
#include <string>

#define DT_UNKNOWN 0
#define DT_DIR 4
#define DT_REG 8

struct dirent {
    unsigned char d_type;
    char d_name[MAX_PATH * 3];
};

struct DIR {
    HANDLE h;
    WIN32_FIND_DATAW fd;
    bool first;
    dirent ent;
};

inline DIR* opendir(const char* path) {
    int n = MultiByteToWideChar(CP_UTF8, 0, path, -1, nullptr, 0);
    if (n <= 0) return nullptr;
    std::wstring w(size_t(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, path, -1, &w[0], n);
    w.resize(size_t(n) - 1);
    w += L"\\*";
    DIR* d = new DIR();
    d->h = FindFirstFileExW(w.c_str(), FindExInfoBasic, &d->fd,
                            FindExSearchNameMatch, nullptr, 0);
    if (d->h == INVALID_HANDLE_VALUE) { delete d; return nullptr; }
    d->first = true;
    return d;
}

inline dirent* readdir(DIR* d) {
    if (!d) return nullptr;
    if (d->first) d->first = false;
    else if (!FindNextFileW(d->h, &d->fd)) return nullptr;
    WideCharToMultiByte(CP_UTF8, 0, d->fd.cFileName, -1, d->ent.d_name,
                        int(sizeof(d->ent.d_name)), nullptr, nullptr);
    d->ent.d_type = (d->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        ? DT_DIR : DT_REG;
    return &d->ent;
}

inline int closedir(DIR* d) {
    if (!d) return -1;
    FindClose(d->h);
    delete d;
    return 0;
}
