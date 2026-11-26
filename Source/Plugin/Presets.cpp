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
    float masterWidth, outDb;
    float compThresh, compRatio, compAttack, compRelease, compMakeup, compMix;
    float eqLowDb, eqMidDb, eqMidHz, eqHighDb;
    float noiseTone;
};

constexpr Clap bank[]{
    {"Uplifting / Downbeat", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 3.5f, 0.04f, 0.15f, 0.45f, 0.55f, 0.8f, 90.0f, 1.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.78f, 0.0f, 180.0f, -0.62f, 90.0f, 10200.0f, 0.85f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.55f, 420.0f, 80.0f, 0.5f, 0.95f, 5.03f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 4.9f, 990.0f, 0.6f, 0.22f},
    {"Uplifting / First Light", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 2, 0.75f, 4.1f, 0.07f, 0.27f, 0.53f, 0.59f, 0.92f, 106.0f, 2.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.82f, 0.0f, 376.0f, -0.55f, 120.0f, 11200.0f, 0.97f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.7f, 700.0f, 80.0f, 0.7f, 1.03f, 3.4f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 4.3f, 1309.0f, 1.1f, 0.28f},
    {"Uplifting / Balcony", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 4.7f, 0.1f, 0.39f, 0.61f, 0.63f, 1.04f, 122.0f, 3.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.86f, 0.0f, 572.0f, -0.48f, 150.0f, 11200.0f, 1.09f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.11f, 0.95f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 6.2f, 921.5f, 1.4f, 0.30f},
    {"Uplifting / Sunlit Palm", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 1, 1.45f, 5.3f, 0.13f, 0.51f, 0.69f, 0.67f, 1.16f, 138.0f, 1.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.78f, 0.0f, 292.0f, -0.41f, 110.0f, 10600.0f, 1.21f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.66f, 1.19f, -1.09f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.3f, 1608.0f, 0.9f, 0.25f},
    {"Uplifting / Wide Ribbon", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 3.5f, 0.04f, 0.63f, 0.45f, 0.71f, 0.8f, 154.0f, 2.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.82f, 0.0f, 488.0f, -0.34f, 90.0f, 10200.0f, 1.33f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.95f, 2.78f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 6.1f, 1144.0f, 0.6f, 0.26f},
    {"Uplifting / Euphoria", NoiseType::White, ClickType::Needle, DistortionType::Tape, 3, 0.4f, 4.1f, 0.07f, 0.75f, 0.53f, 0.55f, 0.92f, 90.0f, 3.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.86f, 0.0f, 208.0f, -0.27f, 120.0f, 11200.0f, 0.85f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 800.0f, 80.0f, 0.48f, 1.03f, 2.8f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 4.0f, 1505.0f, 1.1f, 0.34f},
    {"Uplifting / High Gallery", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 4.7f, 0.1f, 0.15f, 0.61f, 0.59f, 1.04f, 106.0f, 1.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.78f, 0.0f, 404.0f, -0.2f, 150.0f, 11200.0f, 0.97f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.9f, 1100.0f, 80.0f, 0.6f, 1.11f, 1.61f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.9f, 1054.5f, 1.4f, 0.32f},
    {"Uplifting / Silver Edge", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 5.3f, 0.13f, 0.27f, 0.69f, 0.63f, 1.16f, 122.0f, 2.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.82f, 0.0f, 600.0f, -0.62f, 110.0f, 10600.0f, 1.09f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.19f, 1.28f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.0f, 1440.0f, 0.9f, 0.24f},
    {"Uplifting / Morning Sun", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 3.5f, 0.04f, 0.39f, 0.45f, 0.67f, 0.8f, 138.0f, 3.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.86f, 0.0f, 320.0f, -0.55f, 90.0f, 10200.0f, 1.21f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.6f, 320.0f, 80.0f, 0.64f, 0.95f, 1.75f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 5.8f, 1028.5f, 0.6f, 0.22f},
    {"Uplifting / Pure 138", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 1, 1.8f, 4.1f, 0.07f, 0.51f, 0.53f, 0.71f, 0.92f, 154.0f, 1.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.78f, 0.0f, 516.0f, -0.48f, 120.0f, 11200.0f, 1.33f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 1.03f, -0.34f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.2f, 1358.0f, 1.1f, 0.28f},
    {"Uplifting / Opposite Walls", NoiseType::White, ClickType::Needle, DistortionType::Tape, 2, 0.4f, 4.7f, 0.1f, 0.63f, 0.61f, 0.55f, 1.04f, 90.0f, 2.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.82f, 0.0f, 236.0f, -0.41f, 150.0f, 11200.0f, 0.85f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 1.11f, 3.11f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.6f, 954.75f, 1.4f, 0.26f},
    {"Uplifting / Silk Fans", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 3, 0.75f, 5.3f, 0.13f, 0.75f, 0.69f, 0.59f, 1.16f, 106.0f, 3.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.86f, 0.0f, 432.0f, -0.34f, 110.0f, 10600.0f, 0.97f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.8f, 1000.0f, 80.0f, 0.72f, 1.19f, 0.34f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 4.7f, 1664.0f, 0.9f, 0.35f},
    {"Uplifting / Skyward", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 1, 1.1f, 3.5f, 0.04f, 0.15f, 0.45f, 0.63f, 0.8f, 122.0f, 1.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.78f, 0.0f, 628.0f, -0.27f, 90.0f, 10200.0f, 1.09f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.95f, 2.34f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.5f, 1182.5f, 0.6f, 0.30f},
    {"Uplifting / Glass Ceiling", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.45f, 4.1f, 0.07f, 0.27f, 0.53f, 0.67f, 0.92f, 138.0f, 2.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.82f, 0.0f, 348.0f, -0.2f, 120.0f, 11200.0f, 1.21f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.35f, 320.0f, 80.0f, 0.3f, 1.03f, 1.02f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 4.9f, 1554.0f, 1.1f, 0.36f},
    {"Uplifting / Starlight", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.8f, 4.7f, 0.1f, 0.39f, 0.61f, 0.71f, 1.04f, 154.0f, 3.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.86f, 0.0f, 544.0f, -0.62f, 150.0f, 11200.0f, 1.33f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.5f, 320.0f, 80.0f, 0.68f, 1.11f, -0.9f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 6.8f, 855.0f, 1.4f, 0.31f},
    {"Uplifting / Afterglow", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 5.3f, 0.13f, 0.51f, 0.69f, 0.55f, 1.16f, 90.0f, 1.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.78f, 0.0f, 264.0f, -0.55f, 110.0f, 10600.0f, 0.85f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.88f, 1200.0f, 80.0f, 0.68f, 1.19f, 2.68f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 4.4f, 1496.0f, 0.9f, 0.27f},
    {"Rolling / Two Hands", NoiseType::White, ClickType::Needle, DistortionType::Tape, 2, 0.75f, 3.5f, 0.04f, 0.63f, 0.45f, 0.59f, 0.8f, 106.0f, 2.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.82f, 0.0f, 460.0f, -0.48f, 90.0f, 10200.0f, 0.97f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.95f, 4.02f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.2f, 1067.0f, 0.6f, 0.16f},
    {"Rolling / Sixteenth", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 4.1f, 0.07f, 0.75f, 0.53f, 0.63f, 0.92f, 122.0f, 3.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.86f, 0.0f, 180.0f, -0.41f, 120.0f, 11200.0f, 1.09f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.03f, 0.81f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 4.6f, 1407.0f, 1.1f, 0.18f},
    {"Rolling / Pocket Hit", NoiseType::White, ClickType::Crack, DistortionType::Tape, 1, 1.45f, 4.7f, 0.1f, 0.15f, 0.61f, 0.67f, 1.04f, 138.0f, 1.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.78f, 0.0f, 376.0f, -0.34f, 150.0f, 11200.0f, 1.21f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 1.11f, 1.1f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 6.5f, 988.0f, 1.4f, 0.15f},
    {"Rolling / Offbeat", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 5.3f, 0.13f, 0.27f, 0.69f, 0.71f, 1.16f, 154.0f, 2.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.82f, 0.0f, 572.0f, -0.27f, 110.0f, 10600.0f, 1.33f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 1.19f, -0.07f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.6f, 1720.0f, 0.9f, 0.20f},
    {"Rolling / Brush Cut", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 3, 0.4f, 3.5f, 0.04f, 0.39f, 0.45f, 0.55f, 0.8f, 90.0f, 3.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.86f, 0.0f, 292.0f, -0.2f, 90.0f, 10200.0f, 0.85f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.95f, 2.72f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 4.9f, 1221.0f, 0.6f, 0.14f},
    {"Rolling / Slipstream", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 4.1f, 0.07f, 0.51f, 0.53f, 0.59f, 0.92f, 106.0f, 1.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.78f, 0.0f, 488.0f, -0.62f, 120.0f, 11200.0f, 0.97f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.55f, 1.03f, 2.74f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 4.3f, 1260.0f, 1.1f, 0.19f},
    {"Rolling / Quick Step", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 4.7f, 0.1f, 0.63f, 0.61f, 0.63f, 1.04f, 122.0f, 2.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.82f, 0.0f, 208.0f, -0.55f, 150.0f, 11200.0f, 1.09f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.11f, 2.59f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 6.2f, 888.25f, 1.4f, 0.17f},
    {"Rolling / Clock Hands", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 5.3f, 0.13f, 0.75f, 0.69f, 0.67f, 1.16f, 138.0f, 3.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.86f, 0.0f, 404.0f, -0.48f, 110.0f, 10600.0f, 1.21f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 1.19f, -0.03f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 5.3f, 1552.0f, 0.9f, 0.21f},
    {"Progressive / Open Air", NoiseType::White, ClickType::Crack, DistortionType::Tape, 1, 1.8f, 3.5f, 0.04f, 0.15f, 0.45f, 0.71f, 0.8f, 154.0f, 1.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.78f, 0.0f, 600.0f, -0.41f, 90.0f, 10200.0f, 1.33f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 0.95f, 2.1f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 6.1f, 1105.5f, 0.6f, 0.28f},
    {"Progressive / Clear Heel", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 2, 0.4f, 4.1f, 0.07f, 0.27f, 0.53f, 0.55f, 0.92f, 90.0f, 2.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.82f, 0.0f, 320.0f, -0.34f, 120.0f, 11200.0f, 0.85f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.55f, 320.0f, 80.0f, 0.7f, 1.03f, 3.09f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 4.0f, 1456.0f, 1.1f, 0.24f},
    {"Progressive / Archway", NoiseType::White, ClickType::Needle, DistortionType::Tape, 3, 0.75f, 4.7f, 0.1f, 0.39f, 0.61f, 0.59f, 1.04f, 106.0f, 3.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.86f, 0.0f, 516.0f, -0.27f, 150.0f, 11200.0f, 0.97f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.8f, 900.0f, 80.0f, 0.52f, 1.11f, 1.57f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 5.9f, 1021.25f, 1.4f, 0.32f},
    {"Progressive / Shoulder Room", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 1, 1.1f, 5.3f, 0.13f, 0.51f, 0.69f, 0.63f, 1.16f, 122.0f, 1.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.78f, 0.0f, 236.0f, -0.2f, 110.0f, 10600.0f, 1.09f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.19f, -0.59f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.0f, 1776.0f, 0.9f, 0.30f},
    {"Progressive / Air Pad", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.45f, 3.5f, 0.04f, 0.63f, 0.45f, 0.67f, 0.8f, 138.0f, 2.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.82f, 0.0f, 432.0f, -0.62f, 90.0f, 10200.0f, 1.21f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.35f, 320.0f, 80.0f, 0.8f, 0.95f, 3.63f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.8f, 990.0f, 0.6f, 0.34f},
    {"Progressive / Booth Air", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.8f, 4.1f, 0.07f, 0.75f, 0.53f, 0.71f, 0.92f, 154.0f, 3.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.86f, 0.0f, 628.0f, -0.55f, 120.0f, 11200.0f, 1.33f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.6f, 320.0f, 80.0f, 0.68f, 1.03f, 0.11f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 5.2f, 1309.0f, 1.1f, 0.29f},
    {"Progressive / Side Street", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 4.7f, 0.1f, 0.15f, 0.61f, 0.55f, 1.04f, 90.0f, 1.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.78f, 0.0f, 348.0f, -0.48f, 150.0f, 11200.0f, 0.85f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.5f, 320.0f, 80.0f, 0.6f, 1.11f, 2.55f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.6f, 921.5f, 1.4f, 0.23f},
    {"Progressive / High Line", NoiseType::White, ClickType::Needle, DistortionType::Tape, 2, 0.75f, 5.3f, 0.13f, 0.27f, 0.69f, 0.59f, 1.16f, 106.0f, 2.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.82f, 0.0f, 544.0f, -0.41f, 110.0f, 10600.0f, 0.97f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.45f, 700.0f, 80.0f, 0.74f, 1.19f, 1.94f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 4.7f, 1608.0f, 0.9f, 0.26f},
    {"Psy / Fingertips", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 3.5f, 0.04f, 0.39f, 0.45f, 0.63f, 0.8f, 122.0f, 3.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.86f, 0.0f, 264.0f, -0.34f, 90.0f, 10200.0f, 1.09f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.95f, 2.27f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 5.5f, 1144.0f, 0.6f, 0.20f},
    {"Psy / Tripwire", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 1, 1.45f, 4.1f, 0.07f, 0.51f, 0.53f, 0.67f, 0.92f, 138.0f, 1.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.78f, 0.0f, 460.0f, -0.27f, 120.0f, 11200.0f, 1.21f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.35f, 1.03f, 0.12f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 4.9f, 1505.0f, 1.1f, 0.25f},
    {"Psy / Pinwheel", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 4.7f, 0.1f, 0.63f, 0.61f, 0.71f, 1.04f, 154.0f, 2.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.82f, 0.0f, 180.0f, -0.2f, 150.0f, 11200.0f, 1.33f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 1.11f, 0.43f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 6.8f, 1054.5f, 1.4f, 0.28f},
    {"Psy / Ratchet", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 3, 0.4f, 5.3f, 0.13f, 0.75f, 0.69f, 0.55f, 1.16f, 90.0f, 3.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.86f, 0.0f, 376.0f, -0.62f, 110.0f, 10600.0f, 0.85f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 1.19f, 1.35f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 4.4f, 1440.0f, 0.9f, 0.18f},
    {"Psy / Split Second", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 3.5f, 0.04f, 0.15f, 0.45f, 0.59f, 0.8f, 106.0f, 1.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.78f, 0.0f, 572.0f, -0.55f, 90.0f, 10200.0f, 0.97f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.4f, 0.95f, 3.19f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.2f, 1028.5f, 0.6f, 0.22f},
    {"Psy / Broken Pair", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 4.1f, 0.07f, 0.27f, 0.53f, 0.63f, 0.92f, 122.0f, 2.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.82f, 0.0f, 292.0f, -0.48f, 120.0f, 11200.0f, 1.09f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.03f, 1.88f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 4.6f, 1358.0f, 1.1f, 0.24f},
    {"Psy / Frosted Tile", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 4.7f, 0.1f, 0.39f, 0.61f, 0.67f, 1.04f, 138.0f, 3.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.86f, 0.0f, 488.0f, -0.41f, 150.0f, 11200.0f, 1.21f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.3f, 240.0f, 80.0f, 0.25f, 1.11f, -0.3f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 6.5f, 954.75f, 1.4f, 0.32f},
    {"Psy / Five Across", NoiseType::White, ClickType::Crack, DistortionType::Tape, 1, 1.8f, 5.3f, 0.13f, 0.51f, 0.69f, 0.71f, 1.16f, 154.0f, 1.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.78f, 0.0f, 208.0f, -0.34f, 110.0f, 10600.0f, 1.33f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 1.19f, -0.52f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.6f, 1664.0f, 0.9f, 0.26f},
    {"Tech / Dry Chalk", NoiseType::White, ClickType::Needle, DistortionType::Tape, 2, 0.4f, 3.5f, 0.04f, 0.63f, 0.45f, 0.55f, 0.8f, 90.0f, 2.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.82f, 0.0f, 404.0f, -0.27f, 90.0f, 10200.0f, 0.85f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 0.95f, 4.54f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 4.9f, 1182.5f, 0.6f, 0.10f},
    {"Tech / Daylight", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 3, 0.75f, 4.1f, 0.07f, 0.75f, 0.53f, 0.59f, 0.92f, 106.0f, 3.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.86f, 0.0f, 600.0f, -0.2f, 120.0f, 11200.0f, 0.97f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.55f, 600.0f, 80.0f, 0.7f, 1.03f, 1.29f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 4.3f, 1554.0f, 1.1f, 0.14f},
    {"Tech / Front Row", NoiseType::White, ClickType::Crack, DistortionType::Tape, 1, 1.1f, 4.7f, 0.1f, 0.15f, 0.61f, 0.63f, 1.04f, 122.0f, 1.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.78f, 0.0f, 320.0f, -0.62f, 150.0f, 11200.0f, 1.09f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.11f, 1.92f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 6.2f, 855.0f, 1.4f, 0.16f},
    {"Tech / Mono Print", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.45f, 5.3f, 0.13f, 0.27f, 0.69f, 0.67f, 1.16f, 138.0f, 2.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.82f, 0.0f, 516.0f, -0.55f, 110.0f, 10600.0f, 1.21f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 1.19f, 1.17f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.3f, 1496.0f, 0.9f, 0.12f},
    {"Tech / Short Plate", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 1.8f, 3.5f, 0.04f, 0.39f, 0.45f, 0.71f, 0.8f, 154.0f, 3.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.86f, 0.0f, 306.0f, -0.48f, 90.0f, 10200.0f, 1.33f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.4f, 280.0f, 80.0f, 0.68f, 0.95f, 0.61f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 6.1f, 1067.0f, 0.6f, 0.15f},
    {"Tech / Four Corners", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 0.4f, 4.1f, 0.07f, 0.51f, 0.53f, 0.55f, 0.92f, 90.0f, 1.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.78f, 0.0f, 502.0f, -0.41f, 120.0f, 11200.0f, 0.85f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 1.03f, 3.4f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 4.0f, 1407.0f, 1.1f, 0.13f},
    {"Tech / Loose Knuckle", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 2, 0.75f, 4.7f, 0.1f, 0.63f, 0.61f, 0.59f, 1.04f, 106.0f, 2.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.82f, 0.0f, 698.0f, -0.34f, 150.0f, 11200.0f, 0.97f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.62f, 1.11f, 3.38f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.9f, 988.0f, 1.4f, 0.18f},
    {"Tech / Corner Pocket", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.1f, 5.3f, 0.13f, 0.75f, 0.69f, 0.63f, 1.16f, 122.0f, 3.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.86f, 0.0f, 418.0f, -0.27f, 110.0f, 10600.0f, 1.09f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 1.19f, 0.61f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 5.0f, 1720.0f, 0.9f, 0.16f},
    {"Classic / Paper Fold", NoiseType::White, ClickType::Crack, DistortionType::Tape, 1, 1.45f, 3.5f, 0.04f, 0.15f, 0.45f, 0.67f, 0.8f, 138.0f, 1.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.78f, 0.0f, 614.0f, -0.2f, 90.0f, 10200.0f, 1.21f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.6f, 0.95f, 2.61f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 5.8f, 1221.0f, 0.6f, 0.19f},
    {"Classic / Wood Block", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.8f, 4.1f, 0.07f, 0.27f, 0.53f, 0.71f, 0.92f, 154.0f, 2.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.82f, 0.0f, 334.0f, -0.62f, 120.0f, 11200.0f, 1.33f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 1.03f, 1.8f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.2f, 1260.0f, 1.1f, 0.26f},
    {"Classic / Lean Back", NoiseType::White, ClickType::Needle, DistortionType::Tape, 3, 0.4f, 4.7f, 0.1f, 0.39f, 0.61f, 0.55f, 1.04f, 90.0f, 3.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.86f, 0.0f, 530.0f, -0.55f, 150.0f, 11200.0f, 0.85f, 0.0f, 4300.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.6f, 320.0f, 80.0f, 0.66f, 1.11f, 2.32f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 5.6f, 888.25f, 1.4f, 0.20f},
    {"Classic / Palm Circle", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 1, 0.75f, 5.3f, 0.13f, 0.51f, 0.69f, 0.59f, 1.16f, 106.0f, 1.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.78f, 0.0f, 250.0f, -0.48f, 110.0f, 10600.0f, 0.97f, 0.0f, 4750.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.65f, 320.0f, 80.0f, 0.55f, 1.19f, 0.49f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 4.7f, 1552.0f, 0.9f, 0.24f},
    {"Classic / Staggered Line", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 1.1f, 3.5f, 0.04f, 0.63f, 0.45f, 0.63f, 0.8f, 122.0f, 2.5f, 14.0f, 140.0f, 16500.0f, 0.32f, 0.82f, 0.0f, 446.0f, -0.41f, 90.0f, 10200.0f, 1.09f, 0.0f, 5200.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.75f, 900.0f, 80.0f, 0.55f, 0.95f, 3.94f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 5.5f, 1105.5f, 0.6f, 0.21f},
    {"Classic / Upper Deck", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 1.45f, 4.1f, 0.07f, 0.75f, 0.53f, 0.67f, 0.92f, 138.0f, 3.5f, 14.0f, 160.0f, 16500.0f, 0.37f, 0.86f, 0.0f, 642.0f, -0.34f, 120.0f, 11200.0f, 1.21f, 0.0f, 5650.0f, 0.46f, 0.51f, 0.16f, 0.35f, 0.12f, 0.0f, 0.85f, 1000.0f, 80.0f, 0.5f, 1.03f, 0.63f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 9.4f, 4.9f, 1456.0f, 1.1f, 0.25f},
    {"Classic / Silver Palm", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 1, 1.8f, 4.7f, 0.1f, 0.15f, 0.61f, 0.71f, 1.04f, 154.0f, 1.5f, 14.0f, 180.0f, 16500.0f, 0.42f, 0.78f, 0.0f, 362.0f, -0.27f, 150.0f, 11200.0f, 1.33f, 0.0f, 3400.0f, 0.36f, 0.35f, 0.08f, 0.35f, 0.12f, 0.0f, 0.4f, 320.0f, 80.0f, 0.68f, 1.11f, -0.48f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.0f, 6.8f, 1021.25f, 1.4f, 0.28f},
    {"Classic / Last Accent", NoiseType::White, ClickType::Needle, DistortionType::Tape, 2, 0.4f, 5.3f, 0.13f, 0.27f, 0.69f, 0.55f, 1.16f, 90.0f, 2.5f, 14.0f, 200.0f, 16500.0f, 0.47f, 0.82f, 0.0f, 558.0f, -0.2f, 110.0f, 10600.0f, 0.85f, 0.0f, 3850.0f, 0.41f, 0.43f, 0.12f, 0.35f, 0.12f, 0.0f, 0.55f, 320.0f, 80.0f, 0.52f, 1.19f, 2.44f, -18.0f, 1.6f, 4.0f, 80.0f, 0.0f, 0.15f, 8.7f, 4.4f, 1776.0f, 0.9f, 0.22f},
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
        {kNoiseHPF, clap.lowCutHz}, {kNoiseLPF, clap.highCutHz}, {kNoiseResonance, clap.noiseTone},
        {kNoiseStereoWidth, clap.width}, {kNoiseLevel, clap.noiseLevel},
        {kClickType, static_cast<float>(clap.click)}, {kClickFreq, clap.clickHz},
        {kClickLevel, clap.clickLevel}, {kClickDecay, clap.clickDecayMs},
        {kDistType, static_cast<float>(clap.dist)}, {kDistDrive, clap.drive},
        {kDistTone, clap.driveTone}, {kDistMix, clap.driveMix},
        {kCompThresh, clap.compThresh}, {kCompRatio, clap.compRatio}, {kCompAttack, clap.compAttack},
        {kCompRelease, clap.compRelease}, {kCompMakeup, clap.compMakeup}, {kCompMix, clap.compMix},
        {kLimiterEnabled, 1}, {kLimiterCeiling, -1.0f}, {kLimiterRelease, 48},
        {kReverbSize, clap.roomSize}, {kReverbDecay, clap.roomMs}, {kReverbDamp, clap.damp},
        {kReverbGate, clap.gateMs}, {kReverbMix, clap.roomMix},
        {kEchoMix, 0.0f},
        {kReverseMode, static_cast<float>(ReverseSyncMode::Off)}, {kReverseMix, 0},
        {kEqLowFreq, params::eqNoteToHz(40)}, {kEqLowDb, clap.eqLowDb}, {kEqMidFreq, clap.eqMidHz}, {kEqMidDb, clap.eqMidDb},
        {kEqHighFreq, 8000.0f}, {kEqHighDb, clap.eqHighDb}, {kStereoWidth, clap.masterWidth}, {kOutputGain, clap.outDb}
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
