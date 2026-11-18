#include "Presets.h"
#include "Parameters.h"

#include <algorithm>

namespace openclapper::presets {
namespace {
using namespace params;
using namespace core;

struct Material {
    NoiseType noise;
    ClickType click;
    DistortionType dist;
    float bodyLevel, resonance, noiseLevel, clickHz, clickLevel, clickDecayMs, pitchSweep;
};

constexpr Material palm{NoiseType::Pink,   ClickType::Crack,  DistortionType::Tape,     0.92f, 2.0f, 1.00f, 2400, 0.58f, 2.2f, 7};
constexpr Material chalk{NoiseType::White, ClickType::Crack,  DistortionType::Tape,     0.88f, 2.2f, 1.00f, 3400, 0.60f, 1.9f, 5};
constexpr Material rim{NoiseType::Velvet,  ClickType::Crack,  DistortionType::Tube,     0.78f, 2.4f, 0.98f, 4100, 0.62f, 1.7f, 8};
constexpr Material paper{NoiseType::LoFi,  ClickType::Thump,  DistortionType::HardClip, 0.92f, 1.9f, 0.98f, 2100, 0.56f, 2.5f, 6};
constexpr Material silk{NoiseType::White,  ClickType::Thump,  DistortionType::Tape,     0.95f, 1.8f, 1.00f, 2000, 0.54f, 2.6f, 8};

struct Clap {
    const char* name;
    const Material& material;
    int bursts;
    float spacingMs, burstDecayMs, jitter, pan, width, bodyMs, noiseMs, lowCutHz, highCutHz, roomMix, gateMs;
    float roomSize, roomMs, impact, preLevel, noiseHoldMs, noiseCurve, drive, driveMix, doubler, outDb;
};

constexpr Clap bank[]{
    // Uplifting: huge layered stadium & euphoric hands-in-the-air trance claps
    {"Uplifting / Downbeat",       chalk, 4, 10.5f, 12.0f, .18f, .52f, 1.24f, 110, 320, 210, 14500, .32f, 290, .80f,  860, .68f, .92f, 18.0f, -.10f, .34f, .40f, .36f, -0.4f},
    {"Uplifting / First Light",    silk,  5, 11.5f, 13.5f, .24f, .86f, 1.44f, 145, 420, 190, 15200, .38f, 380, .86f, 1120, .74f, .95f, 24.0f, +.05f, .32f, .38f, .46f, -0.4f},
    {"Uplifting / Balcony",        silk,  4, 12.8f, 14.5f, .28f, .90f, 1.48f, 165, 460, 200, 14200, .42f, 420, .90f, 1240, .76f, .94f, 26.0f, +.12f, .30f, .36f, .48f, -0.4f},
    {"Uplifting / Sunlit Palm",    palm,  4, 11.2f, 13.0f, .22f, .78f, 1.32f, 125, 360, 180, 13600, .34f, 320, .82f,  920, .78f, .96f, 20.0f, +.02f, .36f, .42f, .40f, -0.4f},
    {"Uplifting / Wide Ribbon",    silk,  5, 11.0f, 13.5f, .26f, .98f, 1.56f, 135, 390, 220, 15400, .36f, 350, .85f, 1050, .70f, .95f, 22.0f, +.08f, .32f, .38f, .50f, -0.4f},
    {"Uplifting / Euphoria",       chalk, 5, 11.4f, 13.0f, .22f, .92f, 1.50f, 150, 480, 220, 15600, .40f, 410, .88f, 1180, .80f, .96f, 26.0f, +.14f, .38f, .44f, .48f, -0.3f},
    {"Uplifting / High Gallery",   silk,  4, 13.2f, 15.0f, .30f, .84f, 1.44f, 175, 520, 210, 13800, .44f, 460, .92f, 1360, .74f, .94f, 28.0f, +.18f, .30f, .38f, .46f, -0.4f},
    {"Uplifting / Silver Edge",    rim,   4,  9.8f, 11.0f, .20f, .84f, 1.38f,  95, 290, 280, 15800, .30f, 260, .76f,  780, .62f, .92f, 14.0f, -.12f, .38f, .42f, .38f, -0.3f},
    {"Uplifting / Morning Sun",    palm,  4, 11.8f, 13.5f, .24f, .70f, 1.28f, 140, 410, 175, 12800, .38f, 360, .84f, 1080, .82f, .95f, 22.0f, +.06f, .34f, .40f, .42f, -0.4f},
    {"Uplifting / Pure 138",       chalk, 4, 10.6f, 11.8f, .19f, .80f, 1.34f, 115, 340, 230, 14800, .32f, 300, .78f,  860, .72f, .94f, 18.0f, -.04f, .36f, .42f, .40f, -0.3f},
    {"Uplifting / Opposite Walls", chalk, 3, 14.5f, 13.5f, .20f, 1.0f, 1.60f, 130, 380, 240, 13800, .36f, 340, .86f,  980, .70f, .95f, 20.0f, +.04f, .34f, .38f, .50f, -0.4f},
    {"Uplifting / Silk Fans",      silk,  5, 11.5f, 13.2f, .26f, .98f, 1.58f, 140, 420, 200, 13000, .36f, 370, .84f, 1060, .74f, .94f, 22.0f, +.10f, .31f, .38f, .48f, -0.4f},
    {"Uplifting / Skyward",        chalk, 5, 11.8f, 13.8f, .23f, .94f, 1.52f, 160, 500, 230, 15600, .42f, 440, .90f, 1280, .82f, .96f, 28.0f, +.16f, .36f, .42f, .48f, -0.3f},
    {"Uplifting / Glass Ceiling",  rim,   4, 10.2f, 11.5f, .16f, .78f, 1.34f, 105, 310, 260, 15000, .35f, 310, .80f,  920, .64f, .92f, 16.0f, -.08f, .36f, .40f, .40f, -0.4f},
    {"Uplifting / Starlight",      palm,  4, 11.4f, 13.0f, .25f, .90f, 1.44f, 135, 390, 200, 14200, .38f, 360, .85f, 1040, .76f, .95f, 22.0f, +.08f, .35f, .40f, .44f, -0.4f},
    {"Uplifting / Afterglow",      silk,  4, 12.5f, 14.2f, .26f, .76f, 1.38f, 180, 540, 185, 13200, .44f, 480, .92f, 1400, .80f, .94f, 30.0f, +.20f, .32f, .38f, .46f, -0.4f},

    // Rolling: heavy punchy pocket claps with chest weight and distinct flam articulation
    {"Rolling / Two Hands",        palm,  3, 12.2f, 11.5f, .20f, .62f, 1.12f,  95, 240, 200, 11800, .24f, 200, .66f,  580, .68f, .94f, 14.0f, -.14f, .36f, .42f, .28f, -0.5f},
    {"Rolling / Sixteenth",        chalk, 2,  8.8f,  8.2f, .10f, .34f,  .96f,  70, 175, 280, 12800, .16f, 145, .54f,  420, .54f, .90f,  9.0f, -.26f, .34f, .40f, .18f, -0.5f},
    {"Rolling / Pocket Drag",      paper, 3, 13.2f, 12.5f, .28f, .52f, 1.04f, 115, 280, 210, 10400, .26f, 230, .70f,  660, .74f, .94f, 16.0f, -.10f, .40f, .46f, .24f, -0.5f},
    {"Rolling / Offbeat",          chalk, 3,  9.8f,  9.8f, .16f, .48f, 1.08f,  85, 220, 260, 12400, .20f, 180, .60f,  520, .62f, .92f, 12.0f, -.18f, .35f, .42f, .24f, -0.5f},
    {"Rolling / Brush Cut",        paper, 3, 10.4f,  9.5f, .20f, .52f, 1.00f,  80, 205, 250, 10800, .18f, 170, .58f,  480, .64f, .92f, 12.0f, -.18f, .38f, .44f, .22f, -0.5f},
    {"Rolling / Slipstream",       silk,  3, 11.2f, 11.0f, .18f, .60f, 1.18f, 100, 255, 220, 11800, .26f, 210, .68f,  620, .60f, .92f, 14.0f, -.12f, .32f, .38f, .28f, -0.5f},
    {"Rolling / Soft Shoe",        palm,  3, 10.0f,  9.8f, .17f, .40f,  .94f,  90, 225, 200, 10600, .22f, 185, .62f,  540, .66f, .90f, 13.0f, -.14f, .34f, .40f, .22f, -0.5f},
    {"Rolling / Clock Hands",      chalk, 3,  8.6f,  8.5f, .08f, .48f,  .98f,  75, 190, 290, 12200, .18f, 160, .56f,  460, .58f, .90f, 10.0f, -.22f, .36f, .42f, .20f, -0.5f},

    // Progressive: warm, heavy chest slam, sculpted wide arena room
    {"Progressive / Cupped Palm",  palm,  3, 12.0f, 12.8f, .26f, .40f, 1.06f, 135, 310, 165, 10200, .28f, 260, .74f,  780, .78f, .94f, 18.0f, -.04f, .34f, .40f, .28f, -0.5f},
    {"Progressive / Soft Heel",    silk,  3, 12.4f, 13.2f, .24f, .46f, 1.08f, 150, 350, 175, 10600, .32f, 300, .78f,  860, .72f, .94f, 20.0f, +.04f, .30f, .38f, .30f, -0.5f},
    {"Progressive / Archway",      silk,  4, 11.8f, 13.0f, .26f, .76f, 1.38f, 165, 420, 190, 12200, .40f, 380, .86f, 1100, .68f, .94f, 24.0f, +.12f, .32f, .38f, .42f, -0.4f},
    {"Progressive / Shoulder Room",palm,  4, 11.4f, 12.6f, .30f, .84f, 1.32f, 155, 380, 170, 11000, .34f, 320, .80f,  940, .76f, .95f, 20.0f, +.06f, .35f, .42f, .38f, -0.4f},
    {"Progressive / Felt Pad",     silk,  2, 10.8f, 11.2f, .20f, .38f,  .96f, 120, 270, 180,  9800, .26f, 230, .70f,  720, .68f, .90f, 16.0f, -.06f, .30f, .36f, .22f, -0.5f},
    {"Progressive / Booth Air",    palm,  3, 11.5f, 12.2f, .22f, .50f, 1.16f, 140, 340, 185, 11400, .36f, 310, .82f,  920, .72f, .92f, 20.0f, +.04f, .32f, .38f, .32f, -0.5f},
    {"Progressive / Side Street",  paper, 4, 11.6f, 12.6f, .30f, .88f, 1.40f, 150, 390, 220, 10600, .32f, 340, .80f,  980, .78f, .94f, 22.0f, +.08f, .38f, .44f, .36f, -0.4f},
    {"Progressive / Low Roof",     palm,  4, 12.0f, 13.0f, .24f, .60f, 1.22f, 185, 450, 155, 10000, .38f, 400, .88f, 1160, .82f, .95f, 24.0f, +.14f, .34f, .40f, .34f, -0.4f},

    // Psy: biting stereo-cutting whips backed by solid chest core
    {"Psy / Fingertips",           rim,   2,  8.2f,  7.8f, .10f, .32f,  .94f,  65, 160, 340, 14200, .18f, 140, .58f,  460, .48f, .90f, 10.0f, -.28f, .38f, .44f, .20f, -0.4f},
    {"Psy / Tripwire",             rim,   3,  9.8f,  8.8f, .14f, .62f, 1.14f,  78, 195, 310, 13800, .22f, 175, .64f,  560, .55f, .92f, 12.0f, -.22f, .40f, .46f, .28f, -0.4f},
    {"Psy / Pinwheel",             rim,   3,  9.4f,  8.5f, .16f, .82f, 1.36f,  75, 185, 330, 14400, .22f, 170, .62f,  540, .52f, .92f, 11.0f, -.24f, .38f, .44f, .34f, -0.4f},
    {"Psy / Ratchet",              rim,   4,  7.8f,  7.5f, .09f, .56f, 1.12f,  65, 155, 360, 14800, .18f, 145, .56f,  480, .54f, .92f, 10.0f, -.28f, .42f, .46f, .28f, -0.3f},
    {"Psy / Split Second",         rim,   4,  9.4f,  9.2f, .14f, .84f, 1.28f,  85, 220, 290, 14200, .25f, 200, .68f,  640, .58f, .94f, 14.0f, -.18f, .40f, .45f, .34f, -0.4f},
    {"Psy / Broken Pair",          chalk, 3, 11.2f, 10.5f, .36f, .86f, 1.32f,  95, 250, 260, 13200, .26f, 220, .70f,  700, .64f, .94f, 15.0f, -.14f, .40f, .44f, .32f, -0.4f},
    {"Psy / Frosted Tile",         chalk, 3,  9.4f,  9.6f, .14f, .70f, 1.38f,  98, 260, 280, 13800, .34f, 240, .76f,  820, .60f, .93f, 16.0f, -.12f, .38f, .44f, .36f, -0.4f},
    {"Psy / Five Across",          chalk, 5,  9.6f,  9.8f, .20f, .94f, 1.36f, 105, 265, 260, 13400, .28f, 230, .74f,  740, .68f, .95f, 16.0f, -.14f, .40f, .46f, .38f, -0.3f},

    // Tech: slamming, saturated, industrial club & warehouse smashes
    {"Tech / Dry Chalk",           chalk, 3,  8.8f,  8.8f, .12f, .34f,  .90f,  88, 210, 270, 11800, .18f, 180, .62f,  520, .72f, .92f, 14.0f, -.18f, .44f, .50f, .18f, -0.3f},
    {"Tech / Concrete",            paper, 3, 11.2f, 12.0f, .22f, .64f, 1.24f, 130, 340, 195,  9800, .38f, 320, .82f,  960, .86f, .96f, 22.0f, +.04f, .48f, .54f, .32f, -0.3f},
    {"Tech / Front Row",           chalk, 4, 10.2f, 11.0f, .18f, .54f, 1.18f, 115, 290, 220, 14000, .28f, 250, .74f,  760, .78f, .95f, 18.0f, -.08f, .42f, .48f, .28f, -0.3f},
    {"Tech / Mono Print",          palm,  3,  9.6f,  9.8f, .12f,    0,  .00f, 100, 240, 200, 11200, .18f, 190, .60f,  540, .84f, .94f, 15.0f, -.14f, .44f, .50f, .00f, -0.3f},
    {"Tech / Short Plate",         chalk, 3, 10.6f, 11.5f, .14f, .56f, 1.28f, 120, 320, 230, 12600, .36f, 290, .80f,  880, .74f, .94f, 18.0f, -.04f, .40f, .46f, .30f, -0.4f},
    {"Tech / Four Corners",        palm,  4, 10.8f, 11.0f, .22f, .88f, 1.26f, 125, 310, 195, 11800, .28f, 260, .76f,  780, .82f, .95f, 18.0f, -.06f, .44f, .50f, .34f, -0.3f},
    {"Tech / Loose Knuckle",       paper, 3, 12.2f, 12.2f, .32f, .76f, 1.22f, 135, 340, 210,  9800, .28f, 280, .78f,  840, .84f, .95f, 20.0f, -.02f, .46f, .52f, .30f, -0.4f},
    {"Tech / Corner Pocket",       paper, 4, 10.4f, 11.2f, .24f, .72f, 1.24f, 145, 370, 225, 10800, .34f, 330, .82f,  960, .88f, .94f, 22.0f, +.04f, .46f, .52f, .30f, -0.3f},

    // Classic: fat 808/909 analog stacks & gated 80s stadium claps
    {"Classic / Paper Fold",       paper, 4, 10.6f, 11.4f, .20f, .50f, 1.10f, 115, 290, 200, 10400, .26f, 240, .72f,  700, .74f, .94f, 16.0f, -.08f, .40f, .46f, .26f, -0.4f},
    {"Classic / Wood Block",       paper, 2,  9.4f,  9.5f, .16f, .36f,  .96f,  95, 230, 210,  9600, .24f, 200, .66f,  620, .68f, .90f, 14.0f, -.14f, .38f, .44f, .18f, -0.4f},
    {"Classic / Lean Back",        silk,  3, 13.0f, 12.6f, .28f, .72f, 1.24f, 135, 360, 185, 11000, .34f, 320, .80f,  940, .70f, .95f, 20.0f, +.04f, .34f, .40f, .34f, -0.4f},
    {"Classic / Palm Circle",      palm,  5, 10.8f, 12.4f, .32f, .92f, 1.44f, 150, 420, 175, 12000, .38f, 390, .86f, 1140, .80f, .96f, 24.0f, +.10f, .36f, .42f, .42f, -0.3f},
    {"Classic / Staggered Line",   paper, 4, 11.2f, 11.8f, .28f, .96f, 1.38f, 130, 340, 210, 10800, .32f, 310, .80f,  900, .76f, .94f, 20.0f, +.02f, .38f, .44f, .36f, -0.4f},
    {"Classic / Upper Deck",       chalk, 5, 11.0f, 11.6f, .24f, .96f, 1.54f, 165, 480, 210, 14400, .42f, 440, .90f, 1300, .78f, .96f, 26.0f, +.16f, .36f, .42f, .44f, -0.3f},
    {"Classic / Ghost Palm",       palm,  2,  9.0f,  8.8f, .18f, .30f,  .90f,  90, 220, 205, 10200, .20f, 190, .62f,  560, .58f, .90f, 12.0f, -.16f, .34f, .40f, .18f, -0.5f},
    {"Classic / Last Accent",      palm,  4, 11.2f, 12.2f, .20f, .62f, 1.22f, 140, 380, 190, 11600, .34f, 340, .82f,  980, .76f, .94f, 20.0f, +.06f, .36f, .42f, .34f, -0.4f}
};

FactoryPreset makePreset(const Clap& clap) {
    const auto& m = clap.material;
    auto values = std::vector<std::pair<juce::String, float>>{
        {kPreBursts, static_cast<float>(clap.bursts)}, {kPreSpread, clap.spacingMs}, {kPreDecay, clap.burstDecayMs},
        {kPreJitter, clap.jitter}, {kFlamSpread, clap.pan}, {kPreLevel, clap.preLevel},
        {kToneQ, m.resonance}, {kToneDecay, clap.bodyMs},
        {kTonePitchEnv, m.pitchSweep}, {kTonePitchDecay, 14}, {kToneHPF, 120},
        {kToneLPF, std::clamp(clap.highCutHz * 1.25f, 9500.0f, 19000.0f)}, {kToneLevel, m.bodyLevel},
        {kImpactPunch, clap.impact},
        {kNoiseType, static_cast<float>(m.noise)}, {kNoiseAttack, 0.2f}, {kNoiseHold, clap.noiseHoldMs}, {kNoiseDecay, clap.noiseMs},
        {kNoiseCurve, clap.noiseCurve}, {kNoiseHPF, clap.lowCutHz}, {kNoiseLPF, clap.highCutHz}, {kNoiseStereoWidth, clap.width}, {kNoiseLevel, m.noiseLevel},
        {kClickType, static_cast<float>(m.click)}, {kClickFreq, m.clickHz}, {kClickLevel, m.clickLevel}, {kClickDecay, m.clickDecayMs},
        {kDistType, static_cast<float>(m.dist)}, {kDistDrive, clap.drive}, {kDistTone, 0.1f}, {kDistMix, clap.driveMix},
        {kCompThresh, -15}, {kCompRatio, 3.4f}, {kCompAttack, 6}, {kCompRelease, 48}, {kCompMakeup, 3.6f}, {kCompMix, 0.85f},
        {kLimiterEnabled, 1}, {kLimiterCeiling, -0.5f}, {kLimiterRelease, 24}, {kReverbSize, clap.roomSize},
        {kReverbDecay, clap.roomMs}, {kReverbDamp, 0.34f}, {kReverbGate, clap.gateMs}, {kReverbMix, clap.roomMix},
        {kSlapbackDivision, static_cast<float>(SlapbackDivision::Off)}, {kSlapbackMix, 0}, {kSlapbackFeedback, 0},
        {kClapDoubler, clap.doubler}, {kPitchHumanize, 8.0f},
        {kReverseMode, static_cast<float>(ReverseSyncMode::Off)}, {kReverseMix, 0},
        {kEqLowFreq, 80.0f}, {kEqLowDb, 0.0f}, {kEqMidFreq, 1000.0f}, {kEqMidDb, 0.0f},
        {kEqHighFreq, 7500.0f}, {kEqHighDb, 0.0f}, {kStereoWidth, 1.08f}, {kOutputGain, clap.outDb}
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
