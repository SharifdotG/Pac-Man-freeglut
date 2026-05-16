#pragma once

#include <optional>
#include <string>

namespace util {

// Resolve a path relative to the running exe directory, e.g.
// asset_path("assets/levels/level_01.txt") returns the absolute path. The
// returned string uses forward slashes; both std::ifstream and Windows APIs
// accept that.
//
// Why not just use the cwd? Because the user may launch the exe from any
// directory (Explorer double-click, IDE, terminal). Asset paths must be
// stable regardless of cwd.
std::string asset_path(const std::string& relative);

// Read entire text file into a string. Returns nullopt on any I/O error
// (file not found, permission denied, …). Caller logs the failure.
std::optional<std::string> read_text_file(const std::string& path);

// Resolve a path relative to %APPDATA%/pacman-freeglut/. Used for persistent
// user data (hi-score, volume preferences). Falls back to "./pacman-freeglut/"
// if APPDATA isn't set.
std::string appdata_path(const std::string& relative);

// Write content to file, creating parent directories if needed. Returns
// false on any I/O error.
bool write_text_file(const std::string& path, const std::string& content);

}  // namespace util
