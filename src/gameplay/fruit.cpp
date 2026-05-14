#include "gameplay/fruit.h"

#include "render/primitives.h"
#include "world/tile.h"

namespace gameplay {

namespace {
// 9 seconds matches the arcade fruit-on-screen window long enough for the
// player to see and react, short enough to feel like a real reward.
constexpr float kFruitLifetime = 9.0f;
}  // namespace

void fruit_spawn(Fruit& f, FruitKind kind, int col, int row) {
    f.active = true;
    f.col = col;
    f.row = row;
    f.kind = kind;
    f.lifetime_remaining = kFruitLifetime;
}

void fruit_update(Fruit& f, float dt) {
    if (!f.active)
        return;
    f.lifetime_remaining -= dt;
    if (f.lifetime_remaining <= 0.0f) {
        f.active = false;
    }
}

void fruit_render(const Fruit& f) {
    if (!f.active)
        return;

    const float cx = static_cast<float>(world::tile_center_px(f.col));
    const float cy = static_cast<float>(world::tile_center_px(f.row));

    render::Color body{1, 1, 1, 1};
    switch (f.kind) {
        case FruitKind::Cherry:
            body = {0.95f, 0.10f, 0.15f, 1.0f};
            break;  // red
        case FruitKind::Strawberry:
            body = {0.95f, 0.30f, 0.40f, 1.0f};
            break;  // pink-red
        case FruitKind::Orange:
            body = {1.00f, 0.55f, 0.05f, 1.0f};
            break;  // orange
        case FruitKind::Apple:
            body = {0.85f, 0.15f, 0.20f, 1.0f};
            break;  // deep red
        case FruitKind::Melon:
            body = {0.55f, 0.95f, 0.40f, 1.0f};
            break;  // green
    }

    // Body — a fat dot with a slight downward bias so the stem reads.
    render::set_color(body);
    render::draw_filled_circle(cx, cy + 1.5f, 8.0f, 18);

    // Stem — small green rectangle on top.
    render::set_color({0.20f, 0.70f, 0.20f, 1.0f});
    render::draw_quad(cx - 1.0f, cy - 9.0f, 2.0f, 5.0f);
}

int fruit_points(FruitKind k) {
    switch (k) {
        case FruitKind::Cherry:
            return 100;
        case FruitKind::Strawberry:
            return 300;
        case FruitKind::Orange:
            return 500;
        case FruitKind::Apple:
            return 700;
        case FruitKind::Melon:
            return 1000;
    }
    return 0;
}

FruitKind fruit_kind_for_level(int level) {
    switch (level) {
        case 1:
            return FruitKind::Cherry;
        case 2:
            return FruitKind::Strawberry;
        case 3:
            return FruitKind::Orange;
        case 4:
            return FruitKind::Apple;
        default:
            return FruitKind::Melon;
    }
}

}  // namespace gameplay
