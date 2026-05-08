#pragma once

#include <optional>
#include <string>

#include "world/maze.h"

namespace world {

// Parse an ASCII grid into a Maze. The legend is documented in
// docs/11-level-format.md.
//
// Lenient about line length: rows shorter than kCols are right-padded with
// Empty (handles editors that strip trailing whitespace). Rows longer than
// kCols, fewer than kRows total rows, or unknown characters, all fail the
// parse — returns nullopt and logs the offending line.
//
// On success the returned Maze has its dot count populated and its Spawns
// record filled in.
std::optional<Maze> parse_level(const std::string& contents);

// Convenience wrapper: read the file from disk first.
std::optional<Maze> load_level_file(const std::string& asset_relative_path);

}  // namespace world
