#pragma once

namespace render {
namespace camera {

// Tiny screen-shake module. Caller starts a shake with magnitude (max
// pixel offset) and duration; tick() drives a decaying random offset
// that the renderer applies via glTranslate. Magnitude attenuates
// linearly from full → 0 over the lifetime.
//
// Used only for the death-beat in P10. Add more triggers in P11 polish
// (eat-ghost / eat-fruit could get a tiny hit-stop).

void start_shake(float magnitude_px, float duration_sec);
void update(float dt);

float offset_x();
float offset_y();

void clear();

}  // namespace camera
}  // namespace render
