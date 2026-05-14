#include "gameplay/combo.h"

#include <cstring>
#include <vector>

#include "render/text.h"

namespace gameplay {

namespace {

constexpr float kPopupLifetime = 0.60f;
constexpr float kPopupDriftPx = 32.0f;  // travelled-distance over the lifetime

struct Popup {
    char text[16] = {0};
    float x = 0;
    float y = 0;  // y at spawn time
    float lifetime = 0.0f;
    render::Color color{1, 1, 1, 1};
};

std::vector<Popup> g_popups;
bool g_reserved = false;

}  // namespace

void popup_emit(const char* text, float x, float y, render::Color color) {
    if (!g_reserved) {
        g_popups.reserve(64);
        g_reserved = true;
    }
    Popup p;
    std::strncpy(p.text, text, sizeof(p.text) - 1);
    p.text[sizeof(p.text) - 1] = 0;
    p.x = x;
    p.y = y;
    p.lifetime = kPopupLifetime;
    p.color = color;
    g_popups.push_back(p);
}

void popups_update(float dt) {
    if (g_popups.empty())
        return;
    auto write_it = g_popups.begin();
    for (auto read_it = g_popups.begin(); read_it != g_popups.end(); ++read_it) {
        read_it->lifetime -= dt;
        if (read_it->lifetime > 0.0f) {
            if (write_it != read_it)
                *write_it = *read_it;
            ++write_it;
        }
    }
    g_popups.erase(write_it, g_popups.end());
}

void popups_render() {
    for (const auto& p : g_popups) {
        const float t = p.lifetime / kPopupLifetime;     // 1 → 0 over lifetime
        const float drift = (1.0f - t) * kPopupDriftPx;  // moves UP (smaller y) over time
        const float draw_y = p.y - drift;
        render::Color c = p.color;
        c.a *= t;  // fade out as it ages
        render::set_color(c);
        render::draw_text_centered(p.x, draw_y, p.text);
    }
}

void popups_clear() {
    g_popups.clear();
}

}  // namespace gameplay
