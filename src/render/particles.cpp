#include "render/particles.h"

#include <cmath>
#include <cstdlib>
#include <vector>

namespace render {
namespace particles {

namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

struct Particle {
    float x = 0, y = 0;
    float vx = 0, vy = 0;
    float lifetime = 0.0f;  // counts down to 0
    float max_lifetime = 1.0f;
    float size = 3.0f;
    Color color{1, 1, 1, 1};
    float gravity = 0.0f;  // px/s^2 added to vy each tick
};

std::vector<Particle> g_particles;
bool g_reserved = false;

void ensure_reserved() {
    if (g_reserved)
        return;
    g_particles.reserve(256);
    g_reserved = true;
}

float frand() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

// Uniformly random float in [-half, +half].
float frand_centered(float half) {
    return (frand() * 2.0f - 1.0f) * half;
}

}  // namespace

void emit_sparkle(float x, float y, Color color) {
    ensure_reserved();
    constexpr int kCount = 4;
    for (int i = 0; i < kCount; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        const float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(kCount);
        const float speed = 30.0f + frand() * 20.0f;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.max_lifetime = 0.20f + frand() * 0.10f;
        p.lifetime = p.max_lifetime;
        p.size = 2.5f;
        p.color = color;
        g_particles.push_back(p);
    }
}

void emit_burst(float x, float y, int count, Color color) {
    ensure_reserved();
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        const float angle = kTwoPi * frand();
        const float speed = 40.0f + frand() * 80.0f;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.max_lifetime = 0.40f + frand() * 0.25f;
        p.lifetime = p.max_lifetime;
        p.size = 2.5f + frand() * 2.0f;
        p.color = color;
        p.gravity = 60.0f;
        g_particles.push_back(p);
    }
}

void emit_ring(float x, float y, Color color) {
    ensure_reserved();
    constexpr int kCount = 16;
    for (int i = 0; i < kCount; ++i) {
        Particle p;
        p.x = x;
        p.y = y;
        const float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(kCount);
        const float speed = 90.0f;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed;
        p.max_lifetime = 0.45f;
        p.lifetime = p.max_lifetime;
        p.size = 3.0f;
        p.color = color;
        g_particles.push_back(p);
    }
}

void emit_dissolve(float x, float y, Color color) {
    ensure_reserved();
    constexpr int kCount = 30;
    for (int i = 0; i < kCount; ++i) {
        Particle p;
        p.x = x + frand_centered(6.0f);
        p.y = y + frand_centered(6.0f);
        const float angle = kTwoPi * frand();
        const float speed = 10.0f + frand() * 30.0f;
        p.vx = std::cos(angle) * speed;
        p.vy = std::sin(angle) * speed - 20.0f;  // slight upward bias
        p.max_lifetime = 0.7f + frand() * 0.4f;
        p.lifetime = p.max_lifetime;
        p.size = 3.0f;
        p.color = color;
        p.gravity = 30.0f;
        g_particles.push_back(p);
    }
}

void update(float dt) {
    if (g_particles.empty())
        return;
    auto write_it = g_particles.begin();
    for (auto read_it = g_particles.begin(); read_it != g_particles.end(); ++read_it) {
        read_it->vy += read_it->gravity * dt;
        read_it->x += read_it->vx * dt;
        read_it->y += read_it->vy * dt;
        read_it->lifetime -= dt;
        if (read_it->lifetime > 0.0f) {
            if (write_it != read_it)
                *write_it = *read_it;
            ++write_it;
        }
    }
    g_particles.erase(write_it, g_particles.end());
}

void render() {
    for (const auto& p : g_particles) {
        const float alpha = p.lifetime / p.max_lifetime;
        Color c = p.color;
        c.a *= alpha;
        set_color(c);
        const float h = p.size * 0.5f;
        draw_quad(p.x - h, p.y - h, p.size, p.size);
    }
}

void clear() {
    g_particles.clear();
}

}  // namespace particles
}  // namespace render
