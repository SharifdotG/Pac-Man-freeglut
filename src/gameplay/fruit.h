#pragma once

namespace gameplay {

// Bonus-score fruit that briefly appears once or twice per level. Pac
// catches it by overlapping in continuous coordinates (same trick as the
// pac↔ghost test) — it is NOT placed on the maze grid as a tile, so
// where it spawns doesn't have to be a Dot/Empty cell.
//
// Lifecycle: inactive → spawned (by Game when a dot threshold is crossed)
// → either eaten by Pac (score) or expires after `kFruitLifetime`. Only
// one fruit can be active at a time.
enum class FruitKind : unsigned char {
    Cherry,      //  100 pts (level 1)
    Strawberry,  //  300 pts (level 2)
    Orange,      //  500 pts (level 3)
    Apple,       //  700 pts (level 4)
    Melon,       // 1000 pts (level 5+)
};

struct Fruit {
    bool active = false;
    int col = 0;
    int row = 0;
    FruitKind kind = FruitKind::Cherry;
    float lifetime_remaining = 0.0f;  // seconds; ≤ 0 → expire
};

void fruit_spawn(Fruit& f, FruitKind kind, int col, int row);
void fruit_update(Fruit& f, float dt);
void fruit_render(const Fruit& f);

int fruit_points(FruitKind k);
FruitKind fruit_kind_for_level(int level);  // 1-based level number

}  // namespace gameplay
