#include "Presets.h"
#include "Parameters.h"

#include <algorithm>

namespace openclapper::presets {
namespace {
using namespace params;
using namespace core;

struct Clap {
    const char* name;
    NoiseType noise;
    ClickType click;
    DistortionType dist;
    int bursts;
    float spacingMs, burstDecayMs, jitter, pan, preLevel;
    float toneLevel, resonance, bodyMs, pitchSweep, pitchMs, toneHpf, toneLpf, impact;
    float noiseLevel, noiseHoldMs, noiseMs, noiseCurve, lowCutHz, highCutHz, width;
    float noiseAttackMs;
    float clickHz, clickLevel, clickDecayMs;
    float drive, driveTone, driveMix;
    float roomMix, roomSize, roomMs, gateMs, damp;
    float doubler, humanize, masterWidth, outDb;
    float compThresh, compRatio, compAttack, compRelease, compMakeup, compMix;
    float eqLowDb, eqMidDb, eqMidHz, eqHighDb;
};

constexpr Clap bank[]{
    {"Uplifting / Downbeat", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 3.5f, 0.04f, 0.15f, 0.45f, 0.22f, 0.7f, 22.0f, 2.0f, 12.0f, 500.0f, 16500.0f, 0.06f, 0.78f, 0.0f, 250.0f, -0.75f, 420.0f, 14000.0f, 0.85f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.55f, 420.0f, 80.0f, 0.5f, 0.12f, 4.0f, 0.95f, 6.64f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 1700.0f, 1.4f},
    {"Uplifting / First Light", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 0.75f, 4.1f, 0.07f, 0.27f, 0.53f, 0.26f, 0.85f, 28.0f, 3.0f, 12.0f, 580.0f, 16500.0f, 0.1f, 0.82f, 0.0f, 370.0f, -0.67f, 650.0f, 12800.0f, 0.97f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.7f, 700.0f, 80.0f, 0.7f, 0.0f, 4.0f, 1.03f, 4.21f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2000.0f, 2.0f},
    {"Uplifting / Balcony", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 4.7f, 0.1f, 0.39f, 0.61f, 0.3f, 1.0f, 34.0f, 4.0f, 12.0f, 660.0f, 16500.0f, 0.14f, 0.86f, 0.0f, 490.0f, -0.59f, 1500.0f, 17500.0f, 1.09f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.11f, -1.05f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2300.0f, 3.8f},
    {"Uplifting / Sunlit Palm", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 1.45f, 5.3f, 0.13f, 0.51f, 0.69f, 0.34f, 1.15f, 40.0f, 5.0f, 12.0f, 740.0f, 16500.0f, 0.18f, 0.78f, 0.0f, 298.0f, -0.51f, 850.0f, 15500.0f, 1.21f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.66f, 0.0f, 4.0f, 1.19f, -0.34f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2600.0f, 2.6f},
    {"Uplifting / Wide Ribbon", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 3.5f, 0.04f, 0.63f, 0.45f, 0.38f, 0.7f, 46.0f, 2.0f, 12.0f, 820.0f, 16500.0f, 0.06f, 0.82f, 0.0f, 418.0f, -0.75f, 420.0f, 14000.0f, 1.33f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.12f, 4.0f, 0.95f, 2.37f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2900.0f, 1.4f},
    {"Uplifting / Euphoria", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 0.4f, 4.1f, 0.07f, 0.75f, 0.53f, 0.22f, 0.85f, 22.0f, 3.0f, 12.0f, 500.0f, 16500.0f, 0.1f, 0.86f, 0.0f, 538.0f, -0.67f, 650.0f, 12800.0f, 0.85f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 800.0f, 80.0f, 0.48f, 0.0f, 4.0f, 1.03f, 1.69f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3200.0f, 2.0f},
    {"Uplifting / High Gallery", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 4.7f, 0.1f, 0.15f, 0.61f, 0.26f, 1.0f, 28.0f, 4.0f, 12.0f, 580.0f, 16500.0f, 0.14f, 0.78f, 0.0f, 346.0f, -0.59f, 1500.0f, 17500.0f, 0.97f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.9f, 1100.0f, 80.0f, 0.6f, 0.0f, 4.0f, 1.11f, 1.25f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3500.0f, 3.8f},
    {"Uplifting / Silver Edge", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 5.3f, 0.13f, 0.27f, 0.69f, 0.3f, 1.15f, 34.0f, 5.0f, 12.0f, 660.0f, 16500.0f, 0.18f, 0.82f, 0.0f, 466.0f, -0.51f, 850.0f, 15500.0f, 1.09f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.19f, -0.38f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3800.0f, 2.6f},
    {"Uplifting / Morning Sun", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 3.5f, 0.04f, 0.39f, 0.45f, 0.34f, 0.7f, 40.0f, 2.0f, 12.0f, 740.0f, 16500.0f, 0.06f, 0.86f, 0.0f, 274.0f, -0.75f, 420.0f, 14000.0f, 1.21f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.6f, 320.0f, 80.0f, 0.64f, 0.12f, 4.0f, 0.95f, 0.8f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4100.0f, 1.4f},
    {"Uplifting / Pure 138", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 1.8f, 4.1f, 0.07f, 0.51f, 0.53f, 0.38f, 0.85f, 46.0f, 3.0f, 12.0f, 820.0f, 16500.0f, 0.1f, 0.78f, 0.0f, 394.0f, -0.67f, 650.0f, 12800.0f, 1.33f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.03f, -0.59f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4400.0f, 2.0f},
    {"Uplifting / Opposite Walls", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 0.4f, 4.7f, 0.1f, 0.63f, 0.61f, 0.22f, 1.0f, 22.0f, 4.0f, 12.0f, 500.0f, 16500.0f, 0.14f, 0.82f, 0.0f, 514.0f, -0.59f, 1500.0f, 17500.0f, 0.85f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.0f, 4.0f, 1.11f, -1.7f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4700.0f, 3.8f},
    {"Uplifting / Silk Fans", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 0.75f, 5.3f, 0.13f, 0.75f, 0.69f, 0.26f, 1.15f, 28.0f, 5.0f, 12.0f, 580.0f, 16500.0f, 0.18f, 0.86f, 0.0f, 322.0f, -0.51f, 850.0f, 15500.0f, 0.97f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.8f, 1000.0f, 80.0f, 0.72f, 0.0f, 4.0f, 1.19f, -2.46f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5000.0f, 2.6f},
    {"Uplifting / Skyward", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 1.1f, 3.5f, 0.04f, 0.15f, 0.45f, 0.3f, 0.7f, 34.0f, 2.0f, 12.0f, 660.0f, 16500.0f, 0.06f, 0.78f, 0.0f, 442.0f, -0.75f, 420.0f, 14000.0f, 1.09f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.12f, 4.0f, 0.95f, 2.4f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5300.0f, 1.4f},
    {"Uplifting / Glass Ceiling", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.45f, 4.1f, 0.07f, 0.27f, 0.53f, 0.34f, 0.85f, 40.0f, 3.0f, 12.0f, 740.0f, 16500.0f, 0.1f, 0.82f, 0.0f, 250.0f, -0.67f, 650.0f, 12800.0f, 1.21f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.35f, 320.0f, 80.0f, 0.3f, 0.0f, 4.0f, 1.03f, 1.31f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5600.0f, 2.0f},
    {"Uplifting / Starlight", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.8f, 4.7f, 0.1f, 0.39f, 0.61f, 0.38f, 1.0f, 46.0f, 4.0f, 12.0f, 820.0f, 16500.0f, 0.14f, 0.86f, 0.0f, 370.0f, -0.59f, 1500.0f, 17500.0f, 1.33f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.5f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.11f, -1.94f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 1700.0f, 3.8f},
    {"Uplifting / Afterglow", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 5.3f, 0.13f, 0.51f, 0.69f, 0.22f, 1.15f, 22.0f, 5.0f, 12.0f, 500.0f, 16500.0f, 0.18f, 0.78f, 0.0f, 490.0f, -0.51f, 850.0f, 15500.0f, 0.85f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.88f, 1200.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.19f, 1.26f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2000.0f, 2.6f},
    {"Rolling / Two Hands", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 0.75f, 3.5f, 0.04f, 0.63f, 0.45f, 0.26f, 0.7f, 28.0f, 2.0f, 12.0f, 580.0f, 16500.0f, 0.06f, 0.82f, 0.0f, 298.0f, -0.75f, 420.0f, 14000.0f, 0.97f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.12f, 4.0f, 0.95f, 4.78f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2300.0f, 1.4f},
    {"Rolling / Sixteenth", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 4.1f, 0.07f, 0.75f, 0.53f, 0.3f, 0.85f, 34.0f, 3.0f, 12.0f, 660.0f, 16500.0f, 0.1f, 0.86f, 0.0f, 418.0f, -0.67f, 650.0f, 12800.0f, 1.09f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.03f, 0.87f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2600.0f, 2.0f},
    {"Rolling / Pocket Hit", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 1.45f, 4.7f, 0.1f, 0.15f, 0.61f, 0.34f, 1.0f, 40.0f, 4.0f, 12.0f, 740.0f, 16500.0f, 0.14f, 0.78f, 0.0f, 538.0f, -0.59f, 1500.0f, 17500.0f, 1.21f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.0f, 4.0f, 1.11f, 0.67f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2900.0f, 3.8f},
    {"Rolling / Offbeat", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 5.3f, 0.13f, 0.27f, 0.69f, 0.38f, 1.15f, 46.0f, 5.0f, 12.0f, 820.0f, 16500.0f, 0.18f, 0.82f, 0.0f, 346.0f, -0.51f, 850.0f, 15500.0f, 1.33f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.19f, -1.09f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3200.0f, 2.6f},
    {"Rolling / Brush Cut", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 0.4f, 3.5f, 0.04f, 0.39f, 0.45f, 0.22f, 0.7f, 22.0f, 2.0f, 12.0f, 500.0f, 16500.0f, 0.06f, 0.86f, 0.0f, 466.0f, -0.75f, 420.0f, 14000.0f, 0.85f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.12f, 4.0f, 0.95f, 3.25f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3500.0f, 1.4f},
    {"Rolling / Slipstream", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 4.1f, 0.07f, 0.51f, 0.53f, 0.26f, 0.85f, 28.0f, 3.0f, 12.0f, 580.0f, 16500.0f, 0.1f, 0.78f, 0.0f, 274.0f, -0.67f, 650.0f, 12800.0f, 0.97f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.03f, 1.58f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3800.0f, 2.0f},
    {"Rolling / Quick Step", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 4.7f, 0.1f, 0.63f, 0.61f, 0.3f, 1.0f, 34.0f, 4.0f, 12.0f, 660.0f, 16500.0f, 0.14f, 0.82f, 0.0f, 394.0f, -0.59f, 1500.0f, 17500.0f, 1.09f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.11f, -2.13f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4100.0f, 3.8f},
    {"Rolling / Clock Hands", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 5.3f, 0.13f, 0.75f, 0.69f, 0.34f, 1.15f, 40.0f, 5.0f, 12.0f, 740.0f, 16500.0f, 0.18f, 0.86f, 0.0f, 514.0f, -0.51f, 850.0f, 15500.0f, 1.21f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.0f, 4.0f, 1.19f, -3.46f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4400.0f, 2.6f},
    {"Progressive / Open Air", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 1.8f, 3.5f, 0.04f, 0.15f, 0.45f, 0.38f, 0.7f, 46.0f, 2.0f, 12.0f, 820.0f, 16500.0f, 0.06f, 0.78f, 0.0f, 322.0f, -0.75f, 420.0f, 14000.0f, 1.33f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.12f, 4.0f, 0.95f, 1.37f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4700.0f, 1.4f},
    {"Progressive / Clear Heel", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 0.4f, 4.1f, 0.07f, 0.27f, 0.53f, 0.22f, 0.85f, 22.0f, 3.0f, 12.0f, 500.0f, 16500.0f, 0.1f, 0.82f, 0.0f, 442.0f, -0.67f, 650.0f, 12800.0f, 0.85f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.55f, 320.0f, 80.0f, 0.7f, 0.0f, 4.0f, 1.03f, 2.74f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5000.0f, 2.0f},
    {"Progressive / Archway", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 0.75f, 4.7f, 0.1f, 0.39f, 0.61f, 0.26f, 1.0f, 28.0f, 4.0f, 12.0f, 580.0f, 16500.0f, 0.14f, 0.86f, 0.0f, 250.0f, -0.59f, 1500.0f, 17500.0f, 0.97f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.8f, 900.0f, 80.0f, 0.52f, 0.0f, 4.0f, 1.11f, -2.78f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5300.0f, 3.8f},
    {"Progressive / Shoulder Room", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 1.1f, 5.3f, 0.13f, 0.51f, 0.69f, 0.3f, 1.15f, 34.0f, 5.0f, 12.0f, 660.0f, 16500.0f, 0.18f, 0.78f, 0.0f, 370.0f, -0.51f, 850.0f, 15500.0f, 1.09f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.19f, -2.74f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5600.0f, 2.6f},
    {"Progressive / Air Pad", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.45f, 3.5f, 0.04f, 0.63f, 0.45f, 0.34f, 0.7f, 40.0f, 2.0f, 12.0f, 740.0f, 16500.0f, 0.06f, 0.82f, 0.0f, 490.0f, -0.75f, 420.0f, 14000.0f, 1.21f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.35f, 320.0f, 80.0f, 0.8f, 0.12f, 4.0f, 0.95f, 3.07f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 1700.0f, 1.4f},
    {"Progressive / Booth Air", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.8f, 4.1f, 0.07f, 0.75f, 0.53f, 0.38f, 0.85f, 46.0f, 3.0f, 12.0f, 820.0f, 16500.0f, 0.1f, 0.86f, 0.0f, 298.0f, -0.67f, 650.0f, 12800.0f, 1.33f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.6f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.03f, 0.37f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2000.0f, 2.0f},
    {"Progressive / Side Street", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 4.7f, 0.1f, 0.15f, 0.61f, 0.22f, 1.0f, 22.0f, 4.0f, 12.0f, 500.0f, 16500.0f, 0.14f, 0.78f, 0.0f, 418.0f, -0.59f, 1500.0f, 17500.0f, 0.85f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.5f, 320.0f, 80.0f, 0.6f, 0.0f, 4.0f, 1.11f, 2.22f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2300.0f, 3.8f},
    {"Progressive / High Line", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 0.75f, 5.3f, 0.13f, 0.27f, 0.69f, 0.26f, 1.15f, 28.0f, 5.0f, 12.0f, 580.0f, 16500.0f, 0.18f, 0.82f, 0.0f, 538.0f, -0.51f, 850.0f, 15500.0f, 0.97f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.45f, 700.0f, 80.0f, 0.74f, 0.0f, 4.0f, 1.19f, 1.29f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2600.0f, 2.6f},
    {"Psy / Fingertips", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 3.5f, 0.04f, 0.39f, 0.45f, 0.3f, 0.7f, 34.0f, 2.0f, 12.0f, 660.0f, 16500.0f, 0.06f, 0.86f, 0.0f, 346.0f, -0.75f, 420.0f, 14000.0f, 1.09f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.12f, 4.0f, 0.95f, 2.6f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2900.0f, 1.4f},
    {"Psy / Tripwire", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 1.45f, 4.1f, 0.07f, 0.51f, 0.53f, 0.34f, 0.85f, 40.0f, 3.0f, 12.0f, 740.0f, 16500.0f, 0.1f, 0.78f, 0.0f, 466.0f, -0.67f, 650.0f, 12800.0f, 1.21f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.35f, 0.0f, 4.0f, 1.03f, 0.6f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3200.0f, 2.0f},
    {"Psy / Pinwheel", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 4.7f, 0.1f, 0.63f, 0.61f, 0.38f, 1.0f, 46.0f, 4.0f, 12.0f, 820.0f, 16500.0f, 0.14f, 0.82f, 0.0f, 274.0f, -0.59f, 1500.0f, 17500.0f, 1.33f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.11f, -2.44f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3500.0f, 3.8f},
    {"Psy / Ratchet", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 0.4f, 5.3f, 0.13f, 0.75f, 0.69f, 0.22f, 1.15f, 22.0f, 5.0f, 12.0f, 500.0f, 16500.0f, 0.18f, 0.86f, 0.0f, 394.0f, -0.51f, 850.0f, 15500.0f, 0.85f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.0f, 4.0f, 1.19f, -0.76f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3800.0f, 2.6f},
    {"Psy / Split Second", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 3.5f, 0.04f, 0.15f, 0.45f, 0.26f, 0.7f, 28.0f, 2.0f, 12.0f, 580.0f, 16500.0f, 0.06f, 0.78f, 0.0f, 514.0f, -0.75f, 420.0f, 14000.0f, 0.97f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.4f, 0.12f, 4.0f, 0.95f, 3.89f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4100.0f, 1.4f},
    {"Psy / Broken Pair", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 4.1f, 0.07f, 0.27f, 0.53f, 0.3f, 0.85f, 34.0f, 3.0f, 12.0f, 660.0f, 16500.0f, 0.1f, 0.82f, 0.0f, 322.0f, -0.67f, 650.0f, 12800.0f, 1.09f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.03f, 2.53f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4400.0f, 2.0f},
    {"Psy / Frosted Tile", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 4.7f, 0.1f, 0.39f, 0.61f, 0.34f, 1.0f, 40.0f, 4.0f, 12.0f, 740.0f, 16500.0f, 0.14f, 0.86f, 0.0f, 442.0f, -0.59f, 1500.0f, 17500.0f, 1.21f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.3f, 240.0f, 80.0f, 0.25f, 0.0f, 4.0f, 1.11f, -3.27f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4700.0f, 3.8f},
    {"Psy / Five Across", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 1.8f, 5.3f, 0.13f, 0.51f, 0.69f, 0.38f, 1.15f, 46.0f, 5.0f, 12.0f, 820.0f, 16500.0f, 0.18f, 0.78f, 0.0f, 250.0f, -0.51f, 850.0f, 15500.0f, 1.33f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.19f, -3.85f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5000.0f, 2.6f},
    {"Tech / Dry Chalk", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 0.4f, 3.5f, 0.04f, 0.63f, 0.45f, 0.22f, 0.7f, 22.0f, 2.0f, 12.0f, 500.0f, 16500.0f, 0.06f, 0.82f, 0.0f, 370.0f, -0.75f, 420.0f, 14000.0f, 0.85f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.12f, 4.0f, 0.95f, 3.34f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5300.0f, 1.4f},
    {"Tech / Daylight", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 0.75f, 4.1f, 0.07f, 0.75f, 0.53f, 0.26f, 0.85f, 28.0f, 3.0f, 12.0f, 580.0f, 16500.0f, 0.1f, 0.86f, 0.0f, 490.0f, -0.67f, 650.0f, 12800.0f, 0.97f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.55f, 600.0f, 80.0f, 0.7f, 0.0f, 4.0f, 1.03f, -1.16f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5600.0f, 2.0f},
    {"Tech / Front Row", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 1.1f, 4.7f, 0.1f, 0.15f, 0.61f, 0.3f, 1.0f, 34.0f, 4.0f, 12.0f, 660.0f, 16500.0f, 0.14f, 0.78f, 0.0f, 298.0f, -0.59f, 1500.0f, 17500.0f, 1.09f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.11f, 0.87f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 1700.0f, 3.8f},
    {"Tech / Mono Print", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.45f, 5.3f, 0.13f, 0.27f, 0.69f, 0.34f, 1.15f, 40.0f, 5.0f, 12.0f, 740.0f, 16500.0f, 0.18f, 0.82f, 0.0f, 418.0f, -0.51f, 850.0f, 15500.0f, 1.21f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.0f, 4.0f, 1.19f, 1.05f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2000.0f, 2.6f},
    {"Tech / Short Plate", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.8f, 3.5f, 0.04f, 0.39f, 0.45f, 0.38f, 0.7f, 46.0f, 2.0f, 12.0f, 820.0f, 16500.0f, 0.06f, 0.86f, 0.0f, 578.0f, -0.75f, 420.0f, 14000.0f, 1.33f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 280.0f, 80.0f, 0.68f, 0.12f, 4.0f, 0.95f, 1.64f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2300.0f, 1.4f},
    {"Tech / Four Corners", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 4.1f, 0.07f, 0.51f, 0.53f, 0.22f, 0.85f, 22.0f, 3.0f, 12.0f, 500.0f, 16500.0f, 0.1f, 0.78f, 0.0f, 386.0f, -0.67f, 650.0f, 12800.0f, 0.85f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.0f, 4.0f, 1.03f, 2.08f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2600.0f, 2.0f},
    {"Tech / Loose Knuckle", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 0.75f, 4.7f, 0.1f, 0.63f, 0.61f, 0.26f, 1.0f, 28.0f, 4.0f, 12.0f, 580.0f, 16500.0f, 0.14f, 0.82f, 0.0f, 506.0f, -0.59f, 1500.0f, 17500.0f, 0.97f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.0f, 4.0f, 1.11f, -0.68f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 2900.0f, 3.8f},
    {"Tech / Corner Pocket", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 5.3f, 0.13f, 0.75f, 0.69f, 0.3f, 1.15f, 34.0f, 5.0f, 12.0f, 660.0f, 16500.0f, 0.18f, 0.86f, 0.0f, 314.0f, -0.51f, 850.0f, 15500.0f, 1.09f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.19f, -1.23f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3200.0f, 2.6f},
    {"Classic / Paper Fold", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 1.45f, 3.5f, 0.04f, 0.15f, 0.45f, 0.34f, 0.7f, 40.0f, 2.0f, 12.0f, 740.0f, 16500.0f, 0.06f, 0.78f, 0.0f, 434.0f, -0.75f, 420.0f, 14000.0f, 1.21f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.6f, 0.12f, 4.0f, 0.95f, 3.14f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3500.0f, 1.4f},
    {"Classic / Wood Block", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 4.1f, 0.07f, 0.27f, 0.53f, 0.38f, 0.85f, 46.0f, 3.0f, 12.0f, 820.0f, 16500.0f, 0.1f, 0.82f, 0.0f, 554.0f, -0.67f, 650.0f, 12800.0f, 1.33f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.03f, 1.35f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 3800.0f, 2.0f},
    {"Classic / Lean Back", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 0.4f, 4.7f, 0.1f, 0.39f, 0.61f, 0.22f, 1.0f, 22.0f, 4.0f, 12.0f, 500.0f, 16500.0f, 0.14f, 0.86f, 0.0f, 362.0f, -0.59f, 1500.0f, 17500.0f, 0.85f, 0.0f, 5600.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.6f, 320.0f, 80.0f, 0.66f, 0.0f, 4.0f, 1.11f, -1.41f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4100.0f, 3.8f},
    {"Classic / Palm Circle", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 5.3f, 0.13f, 0.51f, 0.69f, 0.26f, 1.15f, 28.0f, 5.0f, 12.0f, 580.0f, 16500.0f, 0.18f, 0.78f, 0.0f, 482.0f, -0.51f, 850.0f, 15500.0f, 0.97f, 0.0f, 6300.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.65f, 320.0f, 80.0f, 0.55f, 0.0f, 4.0f, 1.19f, -1.49f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4400.0f, 2.6f},
    {"Classic / Staggered Line", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 3.5f, 0.04f, 0.63f, 0.45f, 0.3f, 0.7f, 34.0f, 2.0f, 12.0f, 660.0f, 16500.0f, 0.06f, 0.82f, 0.0f, 290.0f, -0.75f, 420.0f, 14000.0f, 1.09f, 0.0f, 7000.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.12f, 4.0f, 0.95f, 2.32f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 4700.0f, 1.4f},
    {"Classic / Upper Deck", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 4.1f, 0.07f, 0.75f, 0.53f, 0.34f, 0.85f, 40.0f, 3.0f, 12.0f, 740.0f, 16500.0f, 0.1f, 0.86f, 0.0f, 410.0f, -0.67f, 650.0f, 12800.0f, 1.21f, 0.0f, 7700.0f, 0.74f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.85f, 1000.0f, 80.0f, 0.5f, 0.0f, 4.0f, 1.03f, -1.44f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5000.0f, 2.0f},
    {"Classic / Silver Palm", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 1.8f, 4.7f, 0.1f, 0.15f, 0.61f, 0.38f, 1.0f, 46.0f, 4.0f, 12.0f, 820.0f, 16500.0f, 0.14f, 0.78f, 0.0f, 530.0f, -0.59f, 1500.0f, 17500.0f, 1.33f, 0.0f, 4200.0f, 0.62f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.0f, 4.0f, 1.11f, -2.0f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5300.0f, 3.8f},
    {"Classic / Last Accent", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 0.4f, 5.3f, 0.13f, 0.27f, 0.69f, 0.22f, 1.15f, 22.0f, 5.0f, 12.0f, 500.0f, 16500.0f, 0.18f, 0.82f, 0.0f, 338.0f, -0.51f, 850.0f, 15500.0f, 0.85f, 0.0f, 4900.0f, 0.68f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.55f, 320.0f, 80.0f, 0.52f, 0.0f, 4.0f, 1.19f, 1.16f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, -2.2f, 6.5f, 5600.0f, 2.6f},
};

FactoryPreset makePreset(const Clap& clap) {
    auto values = std::vector<std::pair<juce::String, float>>{
        {kPreBursts, static_cast<float>(clap.bursts)}, {kPreSpread, clap.spacingMs}, {kPreDecay, clap.burstDecayMs},
        {kPreJitter, clap.jitter}, {kFlamSpread, clap.pan}, {kPreLevel, clap.preLevel},
        {kToneQ, clap.resonance}, {kToneDecay, clap.bodyMs}, {kTonePitchEnv, clap.pitchSweep},
        {kTonePitchDecay, clap.pitchMs}, {kToneHPF, clap.toneHpf}, {kToneLPF, clap.toneLpf},
        {kToneLevel, clap.toneLevel}, {kImpactPunch, clap.impact},
        {kNoiseType, static_cast<float>(clap.noise)}, {kNoiseAttack, clap.noiseAttackMs},
        {kNoiseHold, clap.noiseHoldMs}, {kNoiseDecay, clap.noiseMs}, {kNoiseCurve, clap.noiseCurve},
        {kNoiseHPF, clap.lowCutHz}, {kNoiseLPF, clap.highCutHz}, {kNoiseStereoWidth, clap.width},
        {kNoiseLevel, clap.noiseLevel},
        {kClickType, static_cast<float>(clap.click)}, {kClickFreq, clap.clickHz},
        {kClickLevel, clap.clickLevel}, {kClickDecay, clap.clickDecayMs},
        {kDistType, static_cast<float>(clap.dist)}, {kDistDrive, clap.drive},
        {kDistTone, clap.driveTone}, {kDistMix, clap.driveMix},
        {kCompThresh, clap.compThresh}, {kCompRatio, clap.compRatio}, {kCompAttack, clap.compAttack},
        {kCompRelease, clap.compRelease}, {kCompMakeup, clap.compMakeup}, {kCompMix, clap.compMix},
        {kLimiterEnabled, 1}, {kLimiterCeiling, -1.0f}, {kLimiterRelease, 48},
        {kReverbSize, clap.roomSize}, {kReverbDecay, clap.roomMs}, {kReverbDamp, clap.damp},
        {kReverbGate, clap.gateMs}, {kReverbMix, clap.roomMix},
        {kSlapbackDivision, static_cast<float>(SlapbackDivision::Off)}, {kSlapbackMix, 0}, {kSlapbackFeedback, 0},
        {kClapDoubler, clap.doubler}, {kPitchHumanize, clap.humanize},
        {kReverseMode, static_cast<float>(ReverseSyncMode::Off)}, {kReverseMix, 0},
        {kEqLowFreq, 110.0f}, {kEqLowDb, clap.eqLowDb}, {kEqMidFreq, clap.eqMidHz}, {kEqMidDb, clap.eqMidDb},
        {kEqHighFreq, 10000.0f}, {kEqHighDb, clap.eqHighDb}, {kStereoWidth, clap.masterWidth}, {kOutputGain, clap.outDb}
    };
    std::ranges::for_each(kPerformanceParameters, [&](const auto& parameter) {
        values.emplace_back(parameter.id, parameter.initial);
    });
    values.emplace_back("roll_division", 0);
    size_t layerIndex = 0;
    std::ranges::for_each(kLayerNames, [&](const auto&) {
        values.emplace_back(kLayerMute[layerIndex], 0);
        values.emplace_back(kLayerSolo[layerIndex], 0);
        if (layerIndex < kLayerVelocity.size()) values.emplace_back(kLayerVelocity[layerIndex], 1);
        ++layerIndex;
    });
    return {clap.name, std::move(values)};
}
} // namespace

const std::vector<FactoryPreset>& factoryPresets() {
    static const auto presets = [] {
        std::vector<FactoryPreset> result;
        result.reserve(std::size(bank));
        std::ranges::for_each(bank, [&](const auto& clap) {
            result.push_back(makePreset(clap));
        });
        return result;
    }();
    return presets;
}

void apply(const FactoryPreset& preset, juce::AudioProcessorValueTreeState& state,
           const juce::StringArray& lockedIds) {
    std::ranges::for_each(preset.values, [&](const auto& entry) {
        const auto& [paramId, value] = entry;
        if (lockedIds.contains(paramId)) return;
        if (auto* param = state.getParameter(paramId)) {
            param->setValueNotifyingHost(param->convertTo0to1(value));
        }
    });
}

} // namespace openclapper::presets
