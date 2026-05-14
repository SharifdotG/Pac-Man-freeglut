#include "gameplay/perks.h"

namespace gameplay {

const char* perk_short_name(PerkKind k) {
    switch (k) {
        case PerkKind::Freeze:
            return "FREEZE";
        case PerkKind::Speed:
            return "SPEED";
        case PerkKind::Invisibility:
            return "INVIS";
    }
    return "?";
}

render::Color perk_color(PerkKind k) {
    switch (k) {
        case PerkKind::Freeze:
            return {0.50f, 0.85f, 1.00f, 1.0f};  // ice blue
        case PerkKind::Speed:
            return {1.00f, 0.85f, 0.20f, 1.0f};  // gold
        case PerkKind::Invisibility:
            return {0.65f, 0.65f, 0.85f, 1.0f};  // pale grey
    }
    return {1, 1, 1, 1};
}

render::Color perk_screen_tint(PerkKind k) {
    switch (k) {
        case PerkKind::Freeze:
            return {0.30f, 0.55f, 0.85f, 0.18f};
        case PerkKind::Speed:
            return {1.00f, 0.70f, 0.10f, 0.14f};
        case PerkKind::Invisibility:
            return {0.55f, 0.55f, 0.70f, 0.20f};
    }
    return {0, 0, 0, 0};
}

float perk_duration(PerkKind k) {
    switch (k) {
        case PerkKind::Freeze:
            return kFreezeDuration;
        case PerkKind::Speed:
            return kSpeedDuration;
        case PerkKind::Invisibility:
            return kInvisibilityDuration;
    }
    return 0.0f;
}

void perk_draw_icon(float cx, float cy, float radius, PerkKind k) {
    // Each perk is drawn as a coloured filled disc with a small white
    // glyph hint on top — easy to recognise at HUD-icon size.
    const render::Color body = perk_color(k);
    render::set_color(body);
    render::draw_filled_circle(cx, cy, radius, 18);

    // Inner white glyph
    render::set_color({1.0f, 1.0f, 1.0f, 0.95f});
    const float r2 = radius * 0.55f;
    switch (k) {
        case PerkKind::Freeze: {
            // Snowflake-ish: a plus + an X
            const float s = r2 * 0.8f;
            render::draw_quad(cx - 1.0f, cy - s, 2.0f, s * 2.0f);  // vertical
            render::draw_quad(cx - s, cy - 1.0f, s * 2.0f, 2.0f);  // horizontal
            render::draw_quad(cx - s * 0.7f, cy - s * 0.7f, 1.5f, s * 1.4f);
            render::draw_quad(cx + s * 0.7f - 1.5f, cy - s * 0.7f, 1.5f, s * 1.4f);
            break;
        }
        case PerkKind::Speed: {
            // Forward chevron »
            for (int i = 0; i < 2; ++i) {
                const float ox = static_cast<float>(i) * r2 * 0.5f - r2 * 0.25f;
                render::draw_quad(cx + ox - 1.0f, cy - r2 * 0.6f, 2.0f, r2 * 0.5f);
                render::draw_quad(cx + ox, cy - r2 * 0.1f, r2 * 0.5f, 2.0f);
                render::draw_quad(cx + ox + r2 * 0.5f - 1.0f, cy - r2 * 0.1f, 2.0f, r2 * 0.5f);
                render::draw_quad(cx + ox, cy + r2 * 0.4f, r2 * 0.5f, 2.0f);
            }
            break;
        }
        case PerkKind::Invisibility: {
            // Eye outline — circle with a slash
            render::draw_filled_circle(cx, cy, r2 * 0.65f, 14);
            render::set_color({0.20f, 0.20f, 0.30f, 1.0f});
            render::draw_filled_circle(cx, cy, r2 * 0.30f, 12);
            // Slash through it
            render::set_color({1.0f, 1.0f, 1.0f, 0.95f});
            render::draw_quad(cx - r2, cy - 1.5f, r2 * 2.0f, 1.5f);
            break;
        }
    }
}

}  // namespace gameplay
