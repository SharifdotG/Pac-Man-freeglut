#include "world/level_loader.h"

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "util/file.h"

namespace world {

namespace {

bool parse_cell(char ch, int col, int row, Maze& maze, Spawns& spawns) {
    switch (ch) {
        case ' ':
        case '\t':
            maze.set(col, row, TileType::Empty);
            return true;
        case '#':
            maze.set(col, row, TileType::Wall);
            return true;
        case '.':
            maze.set(col, row, TileType::Dot);
            return true;
        case 'o':
        case 'O':
            maze.set(col, row, TileType::PowerPellet);
            return true;
        case '-':
        case '_':
            maze.set(col, row, TileType::GhostDoor);
            return true;
        case 'P':
            maze.set(col, row, TileType::Empty);
            spawns.pac_col = col;
            spawns.pac_row = row;
            return true;
        case 'B':
            maze.set(col, row, TileType::Empty);
            spawns.blinky_col = col;
            spawns.blinky_row = row;
            return true;
        case 'I':
            maze.set(col, row, TileType::Empty);
            spawns.inky_col = col;
            spawns.inky_row = row;
            return true;
        case 'N':  // piNky — 'P' is taken.
            maze.set(col, row, TileType::Empty);
            spawns.pinky_col = col;
            spawns.pinky_row = row;
            return true;
        case 'C':
            maze.set(col, row, TileType::Empty);
            spawns.clyde_col = col;
            spawns.clyde_row = row;
            return true;
        default:
            return false;
    }
}

std::vector<std::string> split_lines(const std::string& s) {
    std::vector<std::string> out;
    std::string buf;
    buf.reserve(64);
    for (char ch : s) {
        if (ch == '\r')
            continue;  // tolerate CRLF
        if (ch == '\n') {
            out.push_back(std::move(buf));
            buf.clear();
            continue;
        }
        buf.push_back(ch);
    }
    if (!buf.empty()) {
        out.push_back(std::move(buf));
    }
    return out;
}

}  // namespace

std::optional<Maze> parse_level(const std::string& contents) {
    auto lines = split_lines(contents);

    // Trim trailing blank rows (final newline or accidental blanks at end).
    while (!lines.empty() && lines.back().find_first_not_of(" \t") == std::string::npos) {
        lines.pop_back();
    }

    if (static_cast<int>(lines.size()) != kRows) {
        std::printf("[pacman:ERROR] level: expected %d rows, got %zu\n", kRows, lines.size());
        return std::nullopt;
    }

    Maze maze;
    Spawns spawns;

    for (int row = 0; row < kRows; ++row) {
        std::string line = lines[static_cast<std::size_t>(row)];
        if (static_cast<int>(line.size()) > kCols) {
            std::printf("[pacman:ERROR] level row %d: %d cols, max %d -- '%s'\n",
                        row,
                        static_cast<int>(line.size()),
                        kCols,
                        line.c_str());
            return std::nullopt;
        }
        // Right-pad with spaces so editors that strip trailing whitespace
        // don't break the parse.
        if (static_cast<int>(line.size()) < kCols) {
            line.append(static_cast<std::size_t>(kCols - static_cast<int>(line.size())), ' ');
        }

        for (int col = 0; col < kCols; ++col) {
            const char ch = line[static_cast<std::size_t>(col)];
            if (!parse_cell(ch, col, row, maze, spawns)) {
                std::printf(
                    "[pacman:ERROR] level row %d col %d: bad char '%c' "
                    "(0x%02X)\n",
                    row,
                    col,
                    ch,
                    static_cast<unsigned>(ch) & 0xFFu);
                return std::nullopt;
            }
        }
    }

    if (!spawns.complete()) {
        std::printf("[pacman:ERROR] level: missing one of P/B/I/N/C spawn markers\n");
        return std::nullopt;
    }

    maze.set_spawns(spawns);
    maze.recount_dots();
    return maze;
}

std::optional<Maze> load_level_file(const std::string& asset_relative_path) {
    const std::string full = util::asset_path(asset_relative_path);
    auto text = util::read_text_file(full);
    if (!text) {
        std::printf("[pacman:ERROR] level: cannot read '%s'\n", full.c_str());
        return std::nullopt;
    }
    auto maze = parse_level(*text);
    if (maze) {
        std::printf("[pacman] level loaded: %s (%d dots/pellets remaining)\n",
                    asset_relative_path.c_str(),
                    maze->dots_remaining());
    }
    return maze;
}

}  // namespace world
