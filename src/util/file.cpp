#include "util/file.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace util {

namespace {

std::string g_exe_dir;

void ensure_exe_dir_resolved() {
    if (!g_exe_dir.empty()) {
        return;
    }
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) {
        // Fall back to cwd; not ideal, but at least we don't crash.
        g_exe_dir = ".";
        return;
    }
    std::string path(buf, n);
    // Normalize backslashes — Windows accepts forward slashes everywhere we
    // care about, and downstream code logs these paths.
    for (char& ch : path) {
        if (ch == '\\')
            ch = '/';
    }
    const auto pos = path.find_last_of('/');
    g_exe_dir = (pos == std::string::npos) ? "." : path.substr(0, pos);
}

}  // namespace

std::string asset_path(const std::string& relative) {
    ensure_exe_dir_resolved();
    return g_exe_dir + "/" + relative;
}

std::optional<std::string> read_text_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return std::nullopt;
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::string appdata_path(const std::string& relative) {
    const char* env = std::getenv("APPDATA");
    std::string base = env ? env : ".";
    for (char& ch : base) {
        if (ch == '\\')
            ch = '/';
    }
    return base + "/pacman-fg/" + relative;
}

bool write_text_file(const std::string& path, const std::string& content) {
    // Ensure parent directory exists. Errors are tolerated — the open()
    // below will report any real problem.
    const auto last_sep = path.find_last_of("/\\");
    if (last_sep != std::string::npos) {
        std::error_code ec;
        std::filesystem::create_directories(path.substr(0, last_sep), ec);
    }
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f)
        return false;
    f.write(content.data(), static_cast<std::streamsize>(content.size()));
    return f.good();
}

}  // namespace util
