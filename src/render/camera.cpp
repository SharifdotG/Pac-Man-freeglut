#include "render/camera.h"

#include <cstdlib>

namespace render {
namespace camera {

namespace {
float g_magnitude = 0.0f;
float g_total = 0.0f;      // initial duration — used to compute attenuation ratio
float g_remaining = 0.0f;  // counts down to 0
float g_ox = 0.0f;
float g_oy = 0.0f;

float frand_centered(float half) {
    const float u = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return (u * 2.0f - 1.0f) * half;
}
}  // namespace

void start_shake(float magnitude_px, float duration_sec) {
    g_magnitude = magnitude_px;
    g_total = duration_sec > 0.0f ? duration_sec : 0.001f;
    g_remaining = duration_sec;
}

void update(float dt) {
    if (g_remaining <= 0.0f) {
        g_ox = 0.0f;
        g_oy = 0.0f;
        return;
    }
    g_remaining -= dt;
    if (g_remaining <= 0.0f) {
        g_remaining = 0.0f;
        g_ox = 0.0f;
        g_oy = 0.0f;
        return;
    }
    const float ratio = g_remaining / g_total;  // 1 → 0 over lifetime
    const float current = g_magnitude * ratio;
    g_ox = frand_centered(current);
    g_oy = frand_centered(current);
}

float offset_x() {
    return g_ox;
}
float offset_y() {
    return g_oy;
}

void clear() {
    g_magnitude = 0.0f;
    g_total = 0.0f;
    g_remaining = 0.0f;
    g_ox = 0.0f;
    g_oy = 0.0f;
}

}  // namespace camera
}  // namespace render
