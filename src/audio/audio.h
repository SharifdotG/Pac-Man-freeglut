#pragma once

#include "audio/sfx_ids.h"

namespace audio {

// Boot the miniaudio engine and synthesize the 8-SFX + 1-BGM bank in
// memory. Returns false if the engine fails to start; callers can keep
// running silently in that case (the play_*/start_bgm calls become no-ops).
bool init();
void shutdown();

void play(SfxId id);

void start_bgm();
void stop_bgm();
bool bgm_is_playing();

// Volume controls — all in [0, 1]. Master multiplies into both SFX and
// BGM. Set/get are async-safe (no locking required because miniaudio's
// per-sound volume API is thread-safe).
void set_master_volume(float v01);
void set_sfx_volume(float v01);
void set_bgm_volume(float v01);
float master_volume();
float sfx_volume();
float bgm_volume();

}  // namespace audio
