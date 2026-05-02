#pragma once

#include <cstdint>

namespace util {

// Cardinal directions on the maze grid. Used by Pac, the four ghosts, and the
// input layer. Header-only — no util/direction.cpp.
enum class Direction : std::uint8_t {
    None,
    Up,
    Down,
    Left,
    Right,
};

inline int dx(Direction d) {
    switch (d) {
        case Direction::Left:
            return -1;
        case Direction::Right:
            return 1;
        default:
            return 0;
    }
}

inline int dy(Direction d) {
    switch (d) {
        case Direction::Up:
            return -1;  // top-left origin: "up" is -y
        case Direction::Down:
            return 1;
        default:
            return 0;
    }
}

inline Direction opposite(Direction d) {
    switch (d) {
        case Direction::Up:
            return Direction::Down;
        case Direction::Down:
            return Direction::Up;
        case Direction::Left:
            return Direction::Right;
        case Direction::Right:
            return Direction::Left;
        default:
            return Direction::None;
    }
}

inline bool is_opposite(Direction a, Direction b) {
    return a != Direction::None && b != Direction::None && a == opposite(b);
}

}  // namespace util
