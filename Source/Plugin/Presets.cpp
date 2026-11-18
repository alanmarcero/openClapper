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
    {"Uplifting / Downbeat", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.35f, 0.62f, 0.42f, 1.2f, 28.0f, 6.0f, 16.0f, 220.0f, 11039.833449f, 0.4f, 0.9f, 0.0f, 145.0f, -0.85f, 963.410299f, 9199.861208f, 1.05f, 0.0f, 5200.0f, 0.48f, 0.55f, 0.22f, 0.0f, 0.28f, 0.0f, 0.55f, 420.0f, 90.0f, 0.5f, 0.0f, 5.0f, 1.08f, -0.24f, -12.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.45f, -1.5f, 1.6f, 2800.0f, 1.2f},
    {"Uplifting / First Light", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.55f, 0.74f, 0.3f, 0.8f, 70.0f, 1.0f, 16.0f, 140.0f, 2576.010658f, 0.12f, 0.78f, 2.0f, 280.0f, -0.28f, 433.318707f, 2146.675548f, 1.25f, 0.0f, 1800.0f, 0.28f, 1.4f, 0.08f, 0.0f, 0.12f, 0.16f, 0.7f, 700.0f, 230.0f, 0.7f, 0.0f, 5.0f, 1.2f, 8.49f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.5f, -0.8f, 1800.0f, -0.4f},
    {"Uplifting / Balcony", NoiseType::White, ClickType::Needle, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.9f, 0.7f, 0.28f, 0.9f, 48.0f, 3.0f, 16.0f, 220.0f, 12170.980698f, 0.18f, 0.8f, 4.0f, 420.0f, -0.12f, 1482.401879f, 10142.483915f, 1.55f, 0.4f, 7000.0f, 0.34f, 0.35f, 0.12f, 0.0f, 0.18f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 1.35f, 1.36f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -2.5f, 0.2f, 2500.0f, 2.2f},
    {"Uplifting / Sunlit Palm", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 5, 18.0f, 7.0f, 0.22f, 0.65f, 0.8f, 0.48f, 1.0f, 55.0f, 2.0f, 16.0f, 160.0f, 2200.0f, 0.34f, 0.86f, 0.0f, 150.0f, -0.55f, 332.001075f, 1800.206049f, 1.45f, 0.0f, 1600.0f, 0.36f, 1.6f, 0.16f, 0.0f, 0.24f, 0.0f, 0.4f, 320.0f, 140.0f, 0.66f, 0.2f, 5.0f, 1.0f, 2.97f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 1.0f, -1.2f, 3200.0f, -1.5f},
    {"Uplifting / Wide Ribbon", NoiseType::White, ClickType::Needle, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 1.0f, 0.55f, 0.22f, 0.7f, 22.0f, 8.0f, 16.0f, 220.0f, 2867.44958f, 0.05f, 0.74f, 8.0f, 360.0f, 0.0f, 321.726431f, 2389.541317f, 1.8f, 3.5f, 9000.0f, 0.46f, 0.3f, 0.05f, 0.0f, 0.08f, 0.18f, 0.4f, 320.0f, 340.0f, 0.68f, 0.0f, 12.0f, 1.55f, 3.44f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.1f, -3.5f, 1.2f, 4500.0f, 2.8f},
    {"Uplifting / Euphoria", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.7f, 0.62f, 0.36f, 1.15f, 40.0f, 5.0f, 16.0f, 220.0f, 17572.577314f, 0.3f, 0.92f, 0.0f, 145.0f, -0.85f, 1958.162782f, 14643.814429f, 1.3f, 0.0f, 5600.0f, 0.5f, 0.5f, 0.2f, 0.0f, 0.26f, 0.0f, 0.75f, 800.0f, 90.0f, 0.48f, 0.0f, 5.0f, 1.22f, -2.66f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.5f, -1.5f, 2.0f, 3000.0f, 1.8f},
    {"Uplifting / High Gallery", NoiseType::Pink, ClickType::Crack, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.85f, 0.74f, 0.34f, 0.95f, 60.0f, 1.5f, 16.0f, 180.0f, 18000.0f, 0.26f, 0.8f, 2.0f, 280.0f, -0.28f, 2167.853933f, 18000.0f, 1.4f, 0.0f, 3000.0f, 0.32f, 0.9f, 0.12f, 0.0f, 0.18f, 0.16f, 0.9f, 1100.0f, 230.0f, 0.6f, 0.0f, 5.0f, 1.25f, 2.37f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.8f, -0.4f, 2500.0f, 0.2f},
    {"Uplifting / Silver Edge", NoiseType::Velvet, ClickType::Needle, DistortionType::Tube, 4, 12.0f, 8.0f, 0.16f, 0.3f, 0.7f, 0.2f, 1.4f, 18.0f, 10.0f, 16.0f, 220.0f, 13245.219516f, 0.0f, 0.7f, 4.0f, 420.0f, -0.12f, 2100.764004f, 11037.68293f, 0.9f, 0.4f, 11000.0f, 0.55f, 0.25f, 0.1f, 0.4f, 0.15f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 2.0f, 0.95f, 5.42f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.15f, -4.0f, 2.4f, 5000.0f, 3.0f},
    {"Uplifting / Morning Sun", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 5, 18.0f, 7.0f, 0.22f, 0.75f, 0.8f, 0.5f, 0.9f, 64.0f, -2.0f, 16.0f, 130.0f, 7461.667269f, 0.38f, 0.88f, 0.0f, 150.0f, -0.55f, 1172.062464f, 6218.056057f, 1.5f, 0.0f, 1500.0f, 0.4f, 1.8f, 0.18f, 0.0f, 0.3f, 0.0f, 0.6f, 320.0f, 200.0f, 0.64f, 0.2f, 5.0f, 1.0f, 1.38f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 1.4f, -1.6f, 2400.0f, -1.0f},
    {"Uplifting / Pure 138", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 0.25f, 0.55f, 0.4f, 1.3f, 30.0f, 4.0f, 16.0f, 220.0f, 3194.854518f, 0.36f, 0.9f, 8.0f, 360.0f, 0.0f, 455.929876f, 2662.378765f, 0.95f, 3.5f, 6000.0f, 0.52f, 0.45f, 0.24f, 0.0f, 0.32f, 0.18f, 0.4f, 320.0f, 340.0f, 0.68f, 0.0f, 5.0f, 0.98f, 2.63f, -14.0f, 2.6f, 6.0f, 90.0f, 0.0f, 0.55f, -1.5f, 1.8f, 2200.0f, 0.8f},
    {"Uplifting / Opposite Walls", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 1.0f, 0.62f, 0.26f, 1.0f, 26.0f, 5.0f, 16.0f, 220.0f, 5597.654233f, 0.16f, 0.76f, 0.0f, 145.0f, -0.85f, 633.682186f, 4664.711861f, 1.7f, 0.0f, 4500.0f, 0.44f, 0.5f, 0.12f, 0.0f, 0.18f, 0.0f, 0.4f, 320.0f, 90.0f, 0.62f, 0.0f, 14.0f, 1.6f, 2.93f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.2f, -2.0f, 0.4f, 2500.0f, 1.4f},
    {"Uplifting / Silk Fans", NoiseType::White, ClickType::Thump, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.75f, 0.74f, 0.24f, 0.75f, 80.0f, 0.5f, 16.0f, 220.0f, 7687.851812f, 0.08f, 0.72f, 2.0f, 280.0f, -0.28f, 701.54046f, 6406.543176f, 1.35f, 0.0f, 2000.0f, 0.22f, 1.2f, 0.06f, 0.0f, 0.1f, 0.16f, 0.8f, 1000.0f, 230.0f, 0.72f, 0.0f, 5.0f, 1.0f, 6.59f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.2f, -1.0f, 1600.0f, -0.6f},
    {"Uplifting / Skyward", NoiseType::White, ClickType::Needle, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.8f, 0.7f, 0.32f, 1.25f, 34.0f, 7.0f, 16.0f, 220.0f, 3165.504097f, 0.24f, 0.88f, 4.0f, 420.0f, -0.12f, 272.897789f, 2637.92008f, 1.4f, 0.4f, 8500.0f, 0.5f, 0.35f, 0.16f, 0.35f, 0.22f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 1.3f, 4.14f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -2.2f, 1.0f, 4000.0f, 2.6f},
    {"Uplifting / Glass Ceiling", NoiseType::Velvet, ClickType::Needle, DistortionType::Tube, 5, 18.0f, 7.0f, 0.22f, 0.45f, 0.8f, 0.62f, 3.4f, 24.0f, 12.0f, 10.0f, 400.0f, 2361.39275f, 0.0f, 0.48f, 0.0f, 150.0f, -0.55f, 379.291809f, 1967.827291f, 1.2f, 0.0f, 10500.0f, 0.4f, 0.28f, 0.0f, 0.0f, 0.0f, 0.0f, 0.35f, 320.0f, 80.0f, 0.3f, 0.2f, 0.0f, 0.9f, 5.1f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.0f, -3.0f, 1.5f, 5500.0f, 2.4f},
    {"Uplifting / Starlight", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 0.65f, 0.55f, 0.3f, 1.5f, 32.0f, 4.0f, 16.0f, 220.0f, 11317.067308f, 0.14f, 0.66f, 8.0f, 360.0f, 0.0f, 2280.975546f, 9430.889423f, 1.25f, 3.5f, 6400.0f, 0.38f, 0.45f, 0.12f, 0.0f, 0.18f, 0.18f, 0.5f, 320.0f, 340.0f, 0.68f, 0.0f, 5.0f, 1.0f, 6.91f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.8f, 0.6f, 2500.0f, 1.6f},
    {"Uplifting / Afterglow", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.6f, 0.62f, 0.4f, 0.85f, 90.0f, -1.0f, 16.0f, 150.0f, 6191.597663f, 0.3f, 0.82f, 0.0f, 145.0f, -0.85f, 723.944833f, 5159.664719f, 1.2f, 0.0f, 1700.0f, 0.3f, 1.5f, 0.12f, 0.0f, 0.18f, 0.0f, 0.88f, 1200.0f, 90.0f, 0.68f, 0.0f, 5.0f, 1.18f, 3.03f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 1.2f, -0.6f, 2500.0f, -0.2f},
    {"Rolling / Two Hands", NoiseType::Pink, ClickType::Crack, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.55f, 0.74f, 0.36f, 1.0f, 30.0f, 2.0f, 16.0f, 220.0f, 9000.461942f, 0.28f, 0.8f, 2.0f, 280.0f, -0.28f, 994.177738f, 7500.384952f, 0.85f, 0.0f, 3400.0f, 0.42f, 0.7f, 0.14f, 0.0f, 0.2f, 0.16f, 0.4f, 320.0f, 230.0f, 0.62f, 0.0f, 5.0f, 0.9f, 4.59f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -0.4f, 0.8f, 2500.0f, 0.6f},
    {"Rolling / Sixteenth", NoiseType::White, ClickType::Needle, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.15f, 0.7f, 0.22f, 1.2f, 16.0f, 8.0f, 16.0f, 220.0f, 3749.08272f, 0.1f, 0.78f, 4.0f, 420.0f, -0.12f, 386.733085f, 3124.2356f, 0.7f, 0.4f, 8000.0f, 0.5f, 0.28f, 0.1f, 0.0f, 0.14f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 0.8f, 8.4f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.15f, -2.5f, 1.0f, 4000.0f, 1.8f},
    {"Rolling / Pocket Drag", NoiseType::LoFi, ClickType::Thump, DistortionType::HardClip, 3, 18.0f, 7.0f, 0.22f, 0.6f, 0.8f, 0.32f, 0.8f, 40.0f, -3.0f, 16.0f, 220.0f, 3120.0f, 0.2f, 0.74f, 0.0f, 220.0f, -0.35f, 280.0f, 2600.0f, 1.15f, 0.0f, 1400.0f, 0.34f, 1.3f, 0.28f, -0.25f, 0.34f, 0.0f, 0.4f, 320.0f, 120.0f, 0.62f, 0.2f, 16.0f, 1.0f, 3.91f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.6f, -0.8f, 2000.0f, -2.2f},
    {"Rolling / Offbeat", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 0.4f, 0.55f, 0.3f, 1.1f, 24.0f, 3.0f, 16.0f, 220.0f, 3835.464083f, 0.22f, 0.82f, 8.0f, 360.0f, 0.0f, 595.067182f, 3196.220069f, 0.95f, 3.5f, 5000.0f, 0.4f, 0.5f, 0.12f, 0.0f, 0.18f, 0.18f, 0.4f, 320.0f, 340.0f, 0.68f, 0.0f, 5.0f, 1.0f, 7.71f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 1.2f, 2600.0f, 0.6f},
    {"Rolling / Brush Cut", NoiseType::Velvet, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.3f, 0.62f, 0.26f, 1.3f, 20.0f, 4.0f, 16.0f, 220.0f, 17362.022832f, 0.08f, 0.68f, 0.0f, 145.0f, -0.85f, 1453.878432f, 14468.35236f, 0.8f, 0.0f, 4200.0f, 0.36f, 0.4f, 0.08f, 0.0f, 0.12f, 0.0f, 0.4f, 320.0f, 90.0f, 0.62f, 0.0f, 5.0f, 1.0f, 5.55f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.0f, 0.4f, 2500.0f, -0.8f},
    {"Rolling / Slipstream", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.5f, 0.74f, 0.34f, 1.0f, 36.0f, 2.0f, 16.0f, 220.0f, 10070.443681f, 0.18f, 0.8f, 2.0f, 280.0f, -0.28f, 1408.884348f, 8392.0364f, 1.15f, 0.0f, 4600.0f, 0.38f, 0.6f, 0.12f, 0.0f, 0.18f, 0.16f, 0.4f, 320.0f, 230.0f, 0.55f, 0.0f, 5.0f, 1.0f, 2.96f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.4f, 2500.0f, 0.4f},
    {"Rolling / Soft Shoe", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.25f, 0.7f, 0.44f, 0.75f, 48.0f, -1.0f, 16.0f, 140.0f, 2200.0f, 0.3f, 0.76f, 4.0f, 420.0f, -0.12f, 315.535972f, 1431.547504f, 0.7f, 0.4f, 1300.0f, 0.32f, 1.6f, 0.1f, 0.0f, 0.16f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 1.0f, 7.28f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 1.0f, -1.0f, 3000.0f, -1.8f},
    {"Rolling / Clock Hands", NoiseType::White, ClickType::Needle, DistortionType::Tape, 5, 18.0f, 7.0f, 0.22f, 0.45f, 0.8f, 0.28f, 1.4f, 18.0f, 5.0f, 16.0f, 220.0f, 8879.189704f, 0.14f, 0.86f, 0.0f, 150.0f, -0.55f, 1529.743816f, 7399.324753f, 1.05f, 0.0f, 7500.0f, 0.48f, 0.32f, 0.18f, 0.0f, 0.22f, 0.0f, 0.4f, 320.0f, 120.0f, 0.62f, 0.2f, 0.0f, 0.75f, 4.73f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.4f, -1.5f, 1.4f, 3500.0f, 1.0f},
    {"Progressive / Cupped Palm", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 0.15f, 0.55f, 0.55f, 0.7f, 50.0f, -2.0f, 16.0f, 110.0f, 9900.294171f, 0.42f, 0.7f, 8.0f, 360.0f, 0.0f, 843.290699f, 8250.245142f, 0.55f, 3.5f, 1200.0f, 0.38f, 1.8f, 0.12f, 0.0f, 0.2f, 0.18f, 0.4f, 320.0f, 340.0f, 0.68f, 0.0f, 5.0f, 0.65f, 5.59f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 1.6f, -1.4f, 2800.0f, -3.0f},
    {"Progressive / Soft Heel", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.3f, 0.62f, 0.4f, 0.8f, 60.0f, 0.0f, 16.0f, 220.0f, 5552.441228f, 0.2f, 0.78f, 0.0f, 145.0f, -0.85f, 470.490437f, 4627.034356f, 0.9f, 0.0f, 1600.0f, 0.26f, 1.4f, 0.12f, 0.0f, 0.18f, 0.0f, 0.55f, 320.0f, 90.0f, 0.7f, 0.0f, 5.0f, 1.0f, 5.3f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.8f, -0.6f, 2500.0f, -1.2f},
    {"Progressive / Archway", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.6f, 0.74f, 0.36f, 1.05f, 46.0f, 2.0f, 16.0f, 220.0f, 10466.982721f, 0.18f, 0.82f, 2.0f, 280.0f, -0.28f, 1996.579717f, 8722.485601f, 1.2f, 0.0f, 4000.0f, 0.36f, 0.7f, 0.12f, 0.0f, 0.18f, 0.16f, 0.8f, 900.0f, 230.0f, 0.52f, 0.0f, 5.0f, 1.15f, 1.6f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.6f, 2500.0f, 0.8f},
    {"Progressive / Shoulder Room", NoiseType::Pink, ClickType::Crack, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.7f, 0.7f, 0.42f, 0.95f, 55.0f, 1.0f, 16.0f, 220.0f, 5908.063648f, 0.32f, 0.8f, 4.0f, 420.0f, -0.12f, 1113.935758f, 4923.386373f, 1.15f, 0.4f, 2800.0f, 0.34f, 0.9f, 0.12f, 0.0f, 0.18f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 1.0f, 9.53f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.6f, 0.2f, 2500.0f, 0.6f},
    {"Progressive / Felt Pad", NoiseType::LoFi, ClickType::Thump, DistortionType::Tape, 5, 18.0f, 7.0f, 0.22f, 0.45f, 0.8f, 0.36f, 0.65f, 40.0f, -4.0f, 16.0f, 220.0f, 2589.051383f, 0.16f, 0.64f, 0.0f, 150.0f, -0.55f, 281.613044f, 2157.542819f, 0.9f, 0.0f, 1100.0f, 0.3f, 1.5f, 0.14f, -0.4f, 0.22f, 0.0f, 0.35f, 320.0f, 140.0f, 0.8f, 0.2f, 5.0f, 0.6f, 6.44f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.4f, -1.8f, 2500.0f, -3.2f},
    {"Progressive / Booth Air", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 0.45f, 0.55f, 0.32f, 1.1f, 38.0f, 3.0f, 16.0f, 220.0f, 11753.045236f, 0.16f, 0.8f, 8.0f, 360.0f, 0.0f, 1195.057002f, 9794.204364f, 1.2f, 3.5f, 5200.0f, 0.4f, 0.55f, 0.12f, 0.0f, 0.18f, 0.18f, 0.6f, 320.0f, 340.0f, 0.68f, 0.0f, 5.0f, 1.0f, 3.35f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.8f, 3200.0f, 1.2f},
    {"Progressive / Side Street", NoiseType::LoFi, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.8f, 0.62f, 0.3f, 0.9f, 34.0f, 1.0f, 16.0f, 220.0f, 18000.0f, 0.14f, 0.76f, 0.0f, 145.0f, -0.85f, 1660.97107f, 16140.36651f, 1.25f, 0.0f, 2600.0f, 0.36f, 0.8f, 0.2f, 0.0f, 0.28f, 0.0f, 0.5f, 320.0f, 90.0f, 0.6f, 0.0f, 18.0f, 1.0f, 1.14f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, -0.4f, 2500.0f, -0.6f},
    {"Progressive / Low Roof", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.4f, 0.74f, 0.48f, 0.8f, 75.0f, -2.0f, 16.0f, 120.0f, 3296.462247f, 0.4f, 0.84f, 2.0f, 280.0f, -0.28f, 646.114313f, 2747.051872f, 0.95f, 0.0f, 1400.0f, 0.36f, 1.7f, 0.12f, 0.0f, 0.18f, 0.16f, 0.45f, 700.0f, 230.0f, 0.74f, 0.0f, 5.0f, 1.0f, 6.81f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 1.8f, -1.2f, 2200.0f, -2.0f},
    {"Psy / Fingertips", NoiseType::Velvet, ClickType::Needle, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.25f, 0.7f, 0.18f, 1.3f, 16.0f, 9.0f, 16.0f, 220.0f, 6392.073822f, 0.0f, 0.72f, 4.0f, 420.0f, -0.12f, 1272.606448f, 5326.728185f, 0.85f, 0.4f, 10000.0f, 0.52f, 0.22f, 0.06f, 0.0f, 0.1f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 0.85f, 9.01f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.1f, -3.5f, 1.6f, 4800.0f, 2.4f},
    {"Psy / Tripwire", NoiseType::White, ClickType::Needle, DistortionType::Tape, 5, 18.0f, 7.0f, 0.22f, 0.8f, 0.8f, 0.24f, 1.4f, 18.0f, 7.0f, 16.0f, 220.0f, 2908.638752f, 0.08f, 0.8f, 0.0f, 150.0f, -0.55f, 399.08378f, 2423.865627f, 1.55f, 0.0f, 8800.0f, 0.48f, 0.3f, 0.12f, 0.0f, 0.18f, 0.0f, 0.4f, 320.0f, 70.0f, 0.35f, 0.2f, 5.0f, 1.0f, 7.54f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -2.8f, 1.2f, 4200.0f, 2.0f},
    {"Psy / Pinwheel", NoiseType::White, ClickType::Crack, DistortionType::Tape, 4, 13.0f, 11.0f, 0.2f, 1.0f, 0.55f, 0.2f, 1.2f, 18.0f, 6.0f, 16.0f, 220.0f, 5040.0f, 0.05f, 0.76f, 6.0f, 400.0f, -0.1f, 500.0f, 4200.0f, 1.6f, 3.5f, 7000.0f, 0.42f, 0.35f, 0.12f, 0.0f, 0.18f, 0.3f, 0.4f, 320.0f, 400.0f, 0.68f, 0.0f, 5.0f, 1.45f, 4.97f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -2.2f, 0.4f, 2500.0f, 1.6f},
    {"Psy / Ratchet", NoiseType::White, ClickType::Needle, DistortionType::HardClip, 5, 5.5f, 4.0f, 0.04f, 0.4f, 0.62f, 0.16f, 1.5f, 14.0f, 4.0f, 16.0f, 220.0f, 18000.0f, 0.06f, 0.62f, 0.0f, 160.0f, -0.8f, 2200.0f, 16000.0f, 0.9f, 0.0f, 9500.0f, 0.58f, 0.22f, 0.12f, 0.3f, 0.16f, 0.0f, 0.4f, 320.0f, 90.0f, 0.62f, 0.0f, 0.0f, 0.85f, -1.11f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.3f, -3.0f, 0.8f, 3800.0f, 1.4f},
    {"Psy / Split Second", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.7f, 0.74f, 0.26f, 1.25f, 22.0f, 5.0f, 16.0f, 220.0f, 2837.935841f, 0.12f, 0.8f, 2.0f, 280.0f, -0.28f, 238.872438f, 2364.946534f, 1.25f, 0.0f, 6200.0f, 0.44f, 0.4f, 0.12f, 0.0f, 0.18f, 0.16f, 0.4f, 320.0f, 230.0f, 0.4f, 0.0f, 5.0f, 1.0f, 5.78f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.6f, 2500.0f, 1.2f},
    {"Psy / Broken Pair", NoiseType::LoFi, ClickType::Crack, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.85f, 0.7f, 0.22f, 0.9f, 22.0f, 2.0f, 16.0f, 220.0f, 2200.0f, 0.1f, 0.7f, 4.0f, 420.0f, -0.12f, 234.276205f, 1642.296766f, 1.3f, 0.4f, 3600.0f, 0.34f, 0.55f, 0.22f, 0.0f, 0.3f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 20.0f, 1.0f, 7.78f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.2f, 2500.0f, -0.4f},
    {"Psy / Frosted Tile", NoiseType::Velvet, ClickType::Needle, DistortionType::Tube, 5, 18.0f, 7.0f, 0.22f, 0.75f, 0.8f, 0.28f, 1.8f, 20.0f, 8.0f, 16.0f, 220.0f, 3044.348742f, 0.04f, 0.68f, 0.0f, 150.0f, -0.55f, 565.555705f, 2536.957285f, 1.6f, 0.0f, 9800.0f, 0.4f, 0.26f, 0.12f, 0.0f, 0.18f, 0.0f, 0.3f, 240.0f, 90.0f, 0.25f, 0.2f, 5.0f, 1.0f, 9.21f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -2.6f, 1.4f, 5200.0f, 2.2f},
    {"Psy / Five Across", NoiseType::White, ClickType::Crack, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 0.95f, 0.55f, 0.24f, 1.15f, 24.0f, 4.0f, 16.0f, 220.0f, 15032.846837f, 0.14f, 0.82f, 8.0f, 360.0f, 0.0f, 1381.775479f, 12527.372364f, 1.45f, 3.5f, 5400.0f, 0.46f, 0.42f, 0.12f, 0.0f, 0.18f, 0.18f, 0.4f, 320.0f, 340.0f, 0.68f, 0.0f, 5.0f, 1.35f, -0.14f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.8f, 3000.0f, 1.0f},
    {"Tech / Dry Chalk", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.2f, 0.62f, 0.4f, 1.2f, 26.0f, 4.0f, 16.0f, 220.0f, 6901.843155f, 0.34f, 0.9f, 0.0f, 145.0f, -0.85f, 614.071228f, 5751.535962f, 0.7f, 0.0f, 5000.0f, 0.5f, 0.5f, 0.26f, 0.0f, 0.34f, 0.0f, 0.4f, 320.0f, 90.0f, 0.62f, 0.0f, 5.0f, 0.8f, 6.41f, -10.0f, 2.8f, 6.0f, 90.0f, 0.0f, 0.5f, -1.0f, 1.6f, 2400.0f, 0.4f},
    {"Tech / Concrete", NoiseType::LoFi, ClickType::Thump, DistortionType::HardClip, 3, 10.0f, 6.5f, 0.1f, 0.35f, 0.74f, 0.46f, 0.85f, 48.0f, -2.0f, 16.0f, 160.0f, 3376.74772f, 0.48f, 0.8f, 2.0f, 280.0f, -0.28f, 338.51456f, 2813.956433f, 1.0f, 0.0f, 1500.0f, 0.4f, 1.4f, 0.34f, -0.15f, 0.4f, 0.16f, 0.55f, 600.0f, 230.0f, 0.7f, 0.0f, 5.0f, 1.0f, 3.39f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.4f, -0.6f, 2000.0f, -1.6f},
    {"Tech / Front Row", NoiseType::White, ClickType::Needle, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.4f, 0.7f, 0.3f, 1.3f, 24.0f, 6.0f, 16.0f, 220.0f, 6841.171417f, 0.2f, 0.86f, 4.0f, 420.0f, -0.12f, 827.064628f, 5700.97618f, 1.05f, 0.4f, 7800.0f, 0.52f, 0.32f, 0.18f, 0.25f, 0.24f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 1.0f, 6.13f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.8f, 1.4f, 3600.0f, 2.0f},
    {"Tech / Mono Print", NoiseType::Pink, ClickType::Crack, DistortionType::Tape, 5, 18.0f, 7.0f, 0.22f, 0.25f, 0.8f, 0.44f, 1.05f, 28.0f, 2.0f, 16.0f, 220.0f, 8557.745464f, 0.36f, 0.88f, 0.0f, 150.0f, -0.55f, 801.468942f, 7131.454553f, 0.4f, 0.0f, 3600.0f, 0.46f, 0.6f, 0.2f, 0.0f, 0.3f, 0.0f, 0.4f, 320.0f, 120.0f, 0.62f, 0.2f, 0.0f, 0.0f, 3.98f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.45f, -0.5f, 1.0f, 2500.0f, 0.6f},
    {"Tech / Short Plate", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 6.0f, 11.0f, 0.2f, 0.45f, 0.55f, 0.28f, 1.2f, 22.0f, 4.0f, 16.0f, 220.0f, 8640.0f, 0.12f, 0.78f, 0.0f, 200.0f, -0.7f, 2400.0f, 7200.0f, 1.2f, 3.5f, 6400.0f, 0.4f, 0.4f, 0.12f, 0.0f, 0.18f, 0.0f, 0.4f, 280.0f, 90.0f, 0.68f, 0.0f, 5.0f, 1.0f, 7.53f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -2.0f, 0.4f, 2500.0f, 1.8f},
    {"Tech / Four Corners", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.9f, 0.62f, 0.34f, 1.15f, 26.0f, 3.0f, 16.0f, 220.0f, 2803.857888f, 0.3f, 0.84f, 0.0f, 145.0f, -0.85f, 495.041278f, 2336.54824f, 1.3f, 0.0f, 4800.0f, 0.44f, 0.5f, 0.22f, 0.0f, 0.28f, 0.0f, 0.4f, 320.0f, 90.0f, 0.62f, 0.0f, 5.0f, 1.2f, 9.33f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 1.2f, 2500.0f, 0.6f},
    {"Tech / Loose Knuckle", NoiseType::LoFi, ClickType::Thump, DistortionType::HardClip, 3, 10.0f, 6.5f, 0.1f, 0.5f, 0.74f, 0.38f, 0.8f, 36.0f, -3.0f, 16.0f, 220.0f, 3871.268422f, 0.26f, 0.74f, 2.0f, 280.0f, -0.28f, 479.720926f, 3226.057019f, 0.9f, 0.0f, 1300.0f, 0.38f, 1.5f, 0.36f, -0.2f, 0.42f, 0.16f, 0.4f, 320.0f, 230.0f, 0.62f, 0.0f, 18.0f, 1.0f, 3.1f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.2f, -0.8f, 2500.0f, -2.4f},
    {"Tech / Corner Pocket", NoiseType::LoFi, ClickType::Crack, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.65f, 0.7f, 0.36f, 0.95f, 32.0f, 1.0f, 16.0f, 220.0f, 2200.0f, 0.28f, 0.8f, 4.0f, 420.0f, -0.12f, 267.646861f, 1813.564377f, 1.05f, 0.4f, 3000.0f, 0.4f, 0.75f, 0.24f, 0.0f, 0.32f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 5.0f, 1.0f, 5.5f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.2f, 2500.0f, -0.4f},
    {"Classic / Paper Fold", NoiseType::LoFi, ClickType::Crack, DistortionType::Tape, 4, 18.0f, 7.0f, 0.22f, 0.65f, 0.8f, 0.4f, 1.3f, 30.0f, 3.0f, 16.0f, 220.0f, 4080.0f, 0.22f, 0.82f, 0.0f, 280.0f, -0.2f, 400.0f, 3400.0f, 1.35f, 0.0f, 2800.0f, 0.4f, 0.7f, 0.16f, 0.0f, 0.24f, 0.22f, 0.4f, 320.0f, 260.0f, 0.6f, 0.2f, 5.0f, 1.0f, 3.83f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.8f, 1800.0f, -0.8f},
    {"Classic / Wood Block", NoiseType::White, ClickType::Needle, DistortionType::Tape, 1, 4.0f, 3.0f, 0.0f, 0.0f, 0.2f, 0.95f, 8.5f, 46.0f, 5.0f, 22.0f, 280.0f, 6000.0f, 0.2f, 0.55f, 0.0f, 150.0f, -0.55f, 360.481392f, 1551.060511f, 0.3f, 0.0f, 2200.0f, 0.55f, 0.45f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 320.0f, 120.0f, 0.62f, 0.0f, 0.0f, 0.55f, 10.03f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.0f, -2.0f, 2.2f, 900.0f, -1.5f},
    {"Classic / Lean Back", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.45f, 0.62f, 0.4f, 0.85f, 58.0f, -1.0f, 16.0f, 220.0f, 9652.948388f, 0.24f, 0.78f, 0.0f, 145.0f, -0.85f, 1747.642834f, 8044.123657f, 1.05f, 0.0f, 1600.0f, 0.3f, 1.4f, 0.12f, 0.0f, 0.18f, 0.0f, 0.6f, 320.0f, 90.0f, 0.66f, 0.0f, 5.0f, 1.0f, 8.1f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.8f, 0.4f, 2500.0f, -0.6f},
    {"Classic / Palm Circle", NoiseType::Pink, ClickType::Crack, DistortionType::Tape, 3, 10.0f, 6.5f, 0.1f, 0.75f, 0.74f, 0.42f, 1.0f, 42.0f, 2.0f, 16.0f, 220.0f, 4190.792863f, 0.3f, 0.86f, 2.0f, 280.0f, -0.28f, 679.82945f, 3492.327386f, 1.3f, 0.0f, 3200.0f, 0.4f, 0.75f, 0.14f, 0.0f, 0.22f, 0.16f, 0.65f, 320.0f, 230.0f, 0.55f, 0.0f, 5.0f, 1.15f, 4.72f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, 0.2f, 0.6f, 2500.0f, 0.6f},
    {"Classic / Staggered Line", NoiseType::LoFi, ClickType::Crack, DistortionType::Tape, 4, 12.0f, 8.0f, 0.16f, 0.85f, 0.7f, 0.28f, 0.95f, 28.0f, 1.0f, 16.0f, 220.0f, 7550.172764f, 0.12f, 0.76f, 4.0f, 420.0f, -0.12f, 944.872962f, 6291.810637f, 1.3f, 0.4f, 3000.0f, 0.34f, 0.65f, 0.18f, 0.0f, 0.26f, 0.26f, 0.75f, 900.0f, 380.0f, 0.55f, 0.12f, 16.0f, 1.0f, 2.77f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.5f, 0.4f, 2500.0f, -0.6f},
    {"Classic / Upper Deck", NoiseType::White, ClickType::Crack, DistortionType::Tape, 5, 18.0f, 7.0f, 0.22f, 1.0f, 0.8f, 0.34f, 1.1f, 40.0f, 4.0f, 16.0f, 220.0f, 3558.63004f, 0.26f, 0.88f, 0.0f, 150.0f, -0.55f, 738.147811f, 2965.525033f, 1.8f, 0.0f, 5000.0f, 0.46f, 0.5f, 0.16f, 0.0f, 0.22f, 0.0f, 0.85f, 1000.0f, 420.0f, 0.5f, 0.2f, 5.0f, 1.28f, 4.79f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.25f, -1.2f, 1.0f, 2800.0f, 1.4f},
    {"Classic / Ghost Palm", NoiseType::Pink, ClickType::Thump, DistortionType::Tape, 3, 14.0f, 11.0f, 0.2f, 0.2f, 0.55f, 0.22f, 0.7f, 26.0f, 0.0f, 16.0f, 220.0f, 2200.0f, 0.06f, 0.62f, 8.0f, 360.0f, 0.0f, 411.828906f, 1675.575243f, 0.55f, 3.5f, 1800.0f, 0.18f, 0.9f, 0.04f, 0.0f, 0.08f, 0.18f, 0.4f, 320.0f, 340.0f, 0.68f, 0.0f, 5.0f, 0.7f, 12.0f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.1f, -1.5f, -0.8f, 2500.0f, -1.4f},
    {"Classic / Last Accent", NoiseType::White, ClickType::Crack, DistortionType::Tape, 2, 7.0f, 4.0f, 0.04f, 0.5f, 0.62f, 0.4f, 1.15f, 34.0f, 3.0f, 16.0f, 220.0f, 2455.978284f, 0.28f, 0.86f, 0.0f, 145.0f, -0.85f, 227.025913f, 2046.64857f, 1.1f, 0.0f, 4600.0f, 0.44f, 0.55f, 0.18f, 0.0f, 0.26f, 0.0f, 0.55f, 320.0f, 90.0f, 0.52f, 0.0f, 5.0f, 1.05f, 9.75f, -14.0f, 2.0f, 6.0f, 90.0f, 0.0f, 0.4f, -0.8f, 1.0f, 2400.0f, 0.6f},
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
