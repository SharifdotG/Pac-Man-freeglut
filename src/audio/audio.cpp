// audio.cpp — pulls in miniaudio (single-header) here, exactly once.
//
// Sound assets are NOT shipped as files. Instead, this TU synthesizes
// chiptune-style WAVs in memory at startup and feeds the byte buffers to
// miniaudio decoders. Result: no audio asset pipeline, no licensing
// considerations, full control over the timbre. Trade-off: the SFX sound
// like a 1980 arcade rather than a polished modern game — appropriate for
// a Pac-Man clone.

#include "audio/audio.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Silence conversion / shadow / pedantic warnings from miniaudio's internals —
// they're not our code and there's no way to fix them upstream from here.
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace audio {

namespace {

// =============================================================================
// Synthesis helpers
// =============================================================================

constexpr int kSampleRate = 22050;
constexpr float kPi = 3.14159265358979323846f;

class Synth {
  public:
    Synth() { m_samples.reserve(static_cast<std::size_t>(kSampleRate)); }

    // Continuous sine. `fade_out` applies a linear amplitude ramp to zero.
    void add_sine(float freq, float duration_sec, float amp = 0.45f,
                  bool fade_out = true) {
        const int n =
            static_cast<int>(static_cast<float>(kSampleRate) * duration_sec);
        const std::size_t start = m_samples.size();
        m_samples.resize(start + static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) {
            const float t =
                static_cast<float>(i) / static_cast<float>(kSampleRate);
            const float env =
                fade_out
                    ? (1.0f - static_cast<float>(i) / static_cast<float>(n))
                    : 1.0f;
            const float s = amp * env * std::sin(2.0f * kPi * freq * t);
            m_samples[start + static_cast<std::size_t>(i)] = pcm_clip(s);
        }
    }

    // Square wave — punchier than sine, classic chiptune timbre.
    void add_square(float freq, float duration_sec, float amp = 0.30f,
                    bool fade_out = true) {
        const int n =
            static_cast<int>(static_cast<float>(kSampleRate) * duration_sec);
        const std::size_t start = m_samples.size();
        m_samples.resize(start + static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) {
            const float t =
                static_cast<float>(i) / static_cast<float>(kSampleRate);
            const float env =
                fade_out
                    ? (1.0f - static_cast<float>(i) / static_cast<float>(n))
                    : 1.0f;
            const float phase = std::fmod(freq * t, 1.0f);
            const float s = amp * env * (phase < 0.5f ? 1.0f : -1.0f);
            m_samples[start + static_cast<std::size_t>(i)] = pcm_clip(s);
        }
    }

    // Linear-frequency sweep (good for "death" descender, "level start" rise).
    void add_sweep(float f0, float f1, float duration_sec, float amp = 0.45f) {
        const int n =
            static_cast<int>(static_cast<float>(kSampleRate) * duration_sec);
        const std::size_t start = m_samples.size();
        m_samples.resize(start + static_cast<std::size_t>(n));
        // Phase accumulator avoids the "nyquist popping" you get from
        // freq-as-function-of-t inside sin(2π·f·t).
        float phase = 0.0f;
        for (int i = 0; i < n; ++i) {
            const float u = static_cast<float>(i) / static_cast<float>(n);
            const float freq = f0 + (f1 - f0) * u;
            phase += 2.0f * kPi * freq / static_cast<float>(kSampleRate);
            const float env = 1.0f - 0.6f * u; // gentle fade
            const float s = amp * env * std::sin(phase);
            m_samples[start + static_cast<std::size_t>(i)] = pcm_clip(s);
        }
    }

    void add_silence(float duration_sec) {
        const int n =
            static_cast<int>(static_cast<float>(kSampleRate) * duration_sec);
        m_samples.insert(m_samples.end(), static_cast<std::size_t>(n), 0);
    }

    // Frequency-modulated square — used for the BGM siren wobble.
    void add_wobble_square(float center_hz, float amp_hz, float wobble_hz,
                           float duration_sec, float amp = 0.22f) {
        const int n =
            static_cast<int>(static_cast<float>(kSampleRate) * duration_sec);
        const std::size_t start = m_samples.size();
        m_samples.resize(start + static_cast<std::size_t>(n));
        float phase = 0.0f;
        for (int i = 0; i < n; ++i) {
            const float t =
                static_cast<float>(i) / static_cast<float>(kSampleRate);
            const float freq =
                center_hz + amp_hz * std::sin(2.0f * kPi * wobble_hz * t);
            phase += 2.0f * kPi * freq / static_cast<float>(kSampleRate);
            const float wave = std::sin(phase) >= 0.0f ? 1.0f : -1.0f;
            const float s = amp * wave;
            m_samples[start + static_cast<std::size_t>(i)] = pcm_clip(s);
        }
    }

    // Wrap accumulated samples in a minimal RIFF/WAVE/PCM header.
    std::vector<std::uint8_t> to_wav() const {
        std::vector<std::uint8_t> out;
        out.reserve(44 + m_samples.size() * 2);
        const std::uint32_t data_size =
            static_cast<std::uint32_t>(m_samples.size() * 2);
        const std::uint32_t byte_rate =
            static_cast<std::uint32_t>(kSampleRate) * 2u;
        write_str(out, "RIFF");
        write_u32(out, 36u + data_size);
        write_str(out, "WAVE");
        write_str(out, "fmt ");
        write_u32(out, 16u); // PCM fmt chunk size
        write_u16(out, 1u);  // PCM format
        write_u16(out, 1u);  // mono
        write_u32(out, static_cast<std::uint32_t>(kSampleRate));
        write_u32(out, byte_rate);
        write_u16(out, 2u);  // block align (mono * 16-bit)
        write_u16(out, 16u); // bits per sample
        write_str(out, "data");
        write_u32(out, data_size);
        for (std::int16_t s : m_samples) {
            out.push_back(static_cast<std::uint8_t>(s & 0xFF));
            out.push_back(static_cast<std::uint8_t>((s >> 8) & 0xFF));
        }
        return out;
    }

  private:
    static std::int16_t pcm_clip(float s) {
        if (s > 1.0f)
            s = 1.0f;
        if (s < -1.0f)
            s = -1.0f;
        return static_cast<std::int16_t>(s * 32000.0f);
    }
    static void write_u16(std::vector<std::uint8_t> &out, std::uint16_t v) {
        out.push_back(static_cast<std::uint8_t>(v & 0xFF));
        out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
    }
    static void write_u32(std::vector<std::uint8_t> &out, std::uint32_t v) {
        out.push_back(static_cast<std::uint8_t>(v & 0xFF));
        out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
        out.push_back(static_cast<std::uint8_t>((v >> 16) & 0xFF));
        out.push_back(static_cast<std::uint8_t>((v >> 24) & 0xFF));
    }
    static void write_str(std::vector<std::uint8_t> &out, const char *s) {
        for (int i = 0; s[i] != 0; ++i) {
            out.push_back(static_cast<std::uint8_t>(s[i]));
        }
    }

    std::vector<std::int16_t> m_samples;
};

// =============================================================================
// Per-SFX patches — the four-bar arcade sounds
// =============================================================================

std::vector<std::uint8_t> make_sfx(SfxId id) {
    Synth s;
    switch (id) {
    case SfxId::Chomp:
        // Tiny mid-pitch pop.
        s.add_square(560.0f, 0.060f, 0.32f, true);
        break;

    case SfxId::PowerPellet:
        // Up-and-down trill.
        s.add_sine(540.0f, 0.05f, 0.40f, false);
        s.add_sine(820.0f, 0.05f, 0.40f, false);
        s.add_sine(540.0f, 0.05f, 0.40f, false);
        s.add_sine(820.0f, 0.08f, 0.40f, true);
        break;

    case SfxId::EatGhost:
        // Quick rising sweep with bright tail.
        s.add_sweep(220.0f, 880.0f, 0.30f, 0.40f);
        break;

    case SfxId::EatFruit:
        // Two-note arpeggio: bright + brighter.
        s.add_sine(880.0f, 0.10f, 0.45f, false);
        s.add_sine(1320.0f, 0.18f, 0.45f, true);
        break;

    case SfxId::Death:
        // Long descending sweep — the "you screwed up" cue.
        s.add_sweep(880.0f, 110.0f, 1.20f, 0.50f);
        break;

    case SfxId::ExtraLife:
        // Three ascending notes — major triad.
        s.add_sine(523.0f, 0.10f, 0.45f, false); // C5
        s.add_sine(659.0f, 0.10f, 0.45f, false); // E5
        s.add_sine(784.0f, 0.20f, 0.45f, true);  // G5
        break;

    case SfxId::LevelStart:
        // Quick four-note jingle — "ready!"
        s.add_sine(523.0f, 0.10f, 0.40f, false); // C5
        s.add_sine(659.0f, 0.10f, 0.40f, false); // E5
        s.add_sine(784.0f, 0.10f, 0.40f, false); // G5
        s.add_sine(1047.0f, 0.18f, 0.40f, true); // C6
        break;

    case SfxId::MenuClick:
        // Short blip.
        s.add_square(440.0f, 0.030f, 0.30f, true);
        break;

    case SfxId::Count:
        break;
    }
    return s.to_wav();
}

std::vector<std::uint8_t> make_bgm() {
    // 4-second wobbling square at ~140Hz wobbling ±40Hz at 2 Hz —
    // approximates the arcade siren.
    Synth s;
    s.add_wobble_square(140.0f, 40.0f, 2.0f, 4.0f, 0.18f);
    return s.to_wav();
}

// =============================================================================
// miniaudio state
// =============================================================================

struct SoundSlot {
    std::vector<std::uint8_t> wav_bytes;
    ma_decoder decoder{};
    ma_sound sound{};
    bool ready = false;
};

ma_engine g_engine{};
bool g_engine_ready = false;

std::array<SoundSlot, static_cast<std::size_t>(SfxId::Count)> g_sfx;
SoundSlot g_bgm;

float g_master_vol = 0.7f;
float g_sfx_vol = 0.8f;
float g_bgm_vol = 0.4f;

void apply_volumes() {
    if (!g_engine_ready)
        return;
    for (auto &slot : g_sfx) {
        if (slot.ready) {
            ma_sound_set_volume(&slot.sound, g_master_vol * g_sfx_vol);
        }
    }
    if (g_bgm.ready) {
        ma_sound_set_volume(&g_bgm.sound, g_master_vol * g_bgm_vol);
    }
}

bool init_slot(SoundSlot &slot, std::vector<std::uint8_t> wav, bool looping) {
    slot.wav_bytes = std::move(wav);
    if (slot.wav_bytes.empty())
        return false;

    ma_decoder_config dec_cfg = ma_decoder_config_init_default();
    if (ma_decoder_init_memory(slot.wav_bytes.data(), slot.wav_bytes.size(),
                               &dec_cfg, &slot.decoder) != MA_SUCCESS) {
        return false;
    }
    // MA_SOUND_FLAG_DECODE pre-decodes upfront; trade RAM for zero-jitter
    // playback. Our SFX are tiny, so the cost is negligible.
    if (ma_sound_init_from_data_source(&g_engine, &slot.decoder,
                                       MA_SOUND_FLAG_DECODE, nullptr,
                                       &slot.sound) != MA_SUCCESS) {
        ma_decoder_uninit(&slot.decoder);
        return false;
    }
    if (looping) {
        ma_sound_set_looping(&slot.sound, MA_TRUE);
    }
    slot.ready = true;
    return true;
}

} // namespace

// =============================================================================
// Public API
// =============================================================================

bool init() {
    if (g_engine_ready)
        return true;

    if (ma_engine_init(nullptr, &g_engine) != MA_SUCCESS) {
        std::printf(
            "[pacman:WARN] miniaudio engine init failed — running silent.\n");
        return false;
    }
    g_engine_ready = true;

    int loaded = 0;
    for (int i = 0; i < static_cast<int>(SfxId::Count); ++i) {
        if (init_slot(g_sfx[static_cast<std::size_t>(i)],
                      make_sfx(static_cast<SfxId>(i)),
                      /*looping=*/false)) {
            ++loaded;
        }
    }
    init_slot(g_bgm, make_bgm(), /*looping=*/true);
    apply_volumes();

    std::printf("[pacman] audio: engine up, %d/%d SFX loaded, BGM %s\n", loaded,
                static_cast<int>(SfxId::Count),
                g_bgm.ready ? "loaded" : "NOT loaded");
    return true;
}

void shutdown() {
    if (!g_engine_ready)
        return;
    for (auto &slot : g_sfx) {
        if (slot.ready) {
            ma_sound_uninit(&slot.sound);
            ma_decoder_uninit(&slot.decoder);
            slot.ready = false;
        }
    }
    if (g_bgm.ready) {
        ma_sound_uninit(&g_bgm.sound);
        ma_decoder_uninit(&g_bgm.decoder);
        g_bgm.ready = false;
    }
    ma_engine_uninit(&g_engine);
    g_engine_ready = false;
}

void play(SfxId id) {
    if (!g_engine_ready)
        return;
    const int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(SfxId::Count))
        return;
    SoundSlot &slot = g_sfx[static_cast<std::size_t>(idx)];
    if (!slot.ready)
        return;
    // Restart from the top — overlapping SFX of the same kind interrupt
    // each other, which is fine for our use cases (rapid chomp etc.).
    ma_sound_seek_to_pcm_frame(&slot.sound, 0);
    ma_sound_start(&slot.sound);
}

void start_bgm() {
    if (!g_engine_ready || !g_bgm.ready)
        return;
    if (ma_sound_is_playing(&g_bgm.sound))
        return;
    ma_sound_seek_to_pcm_frame(&g_bgm.sound, 0);
    ma_sound_start(&g_bgm.sound);
}

void stop_bgm() {
    if (!g_engine_ready || !g_bgm.ready)
        return;
    ma_sound_stop(&g_bgm.sound);
}

bool bgm_is_playing() {
    if (!g_engine_ready || !g_bgm.ready)
        return false;
    return ma_sound_is_playing(&g_bgm.sound) == MA_TRUE;
}

void set_master_volume(float v01) {
    if (v01 < 0.0f)
        v01 = 0.0f;
    if (v01 > 1.0f)
        v01 = 1.0f;
    g_master_vol = v01;
    apply_volumes();
}

void set_sfx_volume(float v01) {
    if (v01 < 0.0f)
        v01 = 0.0f;
    if (v01 > 1.0f)
        v01 = 1.0f;
    g_sfx_vol = v01;
    apply_volumes();
}

void set_bgm_volume(float v01) {
    if (v01 < 0.0f)
        v01 = 0.0f;
    if (v01 > 1.0f)
        v01 = 1.0f;
    g_bgm_vol = v01;
    apply_volumes();
}

float master_volume() { return g_master_vol; }
float sfx_volume() { return g_sfx_vol; }
float bgm_volume() { return g_bgm_vol; }

} // namespace audio
