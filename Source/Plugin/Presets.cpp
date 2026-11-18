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

constexpr Material palm{NoiseType::Pink,   ClickType::Crack,  DistortionType::Tape,     0.90f, 2.1f, 1.00f, 2500, 0.54f, 1.8f, 6};
constexpr Material chalk{NoiseType::White, ClickType::Crack,  DistortionType::Tape,     0.86f, 2.3f, 0.98f, 3500, 0.56f, 1.6f, 4};
constexpr Material rim{NoiseType::Velvet,  ClickType::Crack,  DistortionType::Tube,     0.72f, 2.5f, 0.96f, 4100, 0.58f, 1.4f, 8};
constexpr Material paper{NoiseType::LoFi,  ClickType::Thump,  DistortionType::HardClip, 0.88f, 2.0f, 0.96f, 2200, 0.52f, 2.0f, 5};
constexpr Material silk{NoiseType::White,  ClickType::Thump,  DistortionType::Tape,     0.94f, 1.8f, 1.00f, 2100, 0.50f, 2.2f, 7};

struct Clap {
    const char* name;
    const Material& material;
    int bursts;
    float spacingMs, burstDecayMs, jitter, pan, width, bodyMs, noiseMs, lowCutHz, highCutHz, roomMix, gateMs;
    float preLevel, noiseHoldMs, noiseCurve, drive, driveMix, doubler, outDb;
};

constexpr Clap bank[]{
    // Uplifting: huge layered stadium & euphoric hands-in-the-air trance claps
    {"Uplifting / Downbeat",       chalk, 4,  8.5f, 7.5f, .16f, .46f, 1.12f, 42, 120, 260, 12800, .14f, 102, .90f, 5.0f, -.36f, .28f, .36f, .24f, -0.8f},
    {"Uplifting / First Light",    silk,  5,  8.2f, 8.2f, .22f, .82f, 1.38f, 44, 124, 240, 13800, .24f, 118, .94f, 5.5f, -.36f, .26f, .34f, .38f, -0.8f},
    {"Uplifting / Balcony",        silk,  4,  9.2f, 8.6f, .26f, .86f, 1.40f, 44, 126, 250, 13200, .22f, 116, .92f, 5.5f, -.36f, .25f, .32f, .36f, -0.8f},
    {"Uplifting / Sunlit Palm",    palm,  4,  8.8f, 8.2f, .20f, .74f, 1.26f, 44, 124, 220, 12400, .20f, 114, .94f, 5.0f, -.36f, .30f, .38f, .32f, -0.8f},
    {"Uplifting / Wide Ribbon",    silk,  5,  8.2f, 8.4f, .24f, .96f, 1.52f, 42, 124, 280, 14200, .24f, 118, .94f, 5.5f, -.38f, .26f, .35f, .42f, -0.8f},
    {"Uplifting / Euphoria",       chalk, 5,  8.0f, 7.8f, .20f, .88f, 1.44f, 42, 124, 290, 14400, .26f, 120, .95f, 5.5f, -.38f, .32f, .40f, .40f, -0.6f},
    {"Uplifting / High Gallery",   silk,  4,  9.2f, 8.8f, .28f, .78f, 1.36f, 44, 126, 270, 12800, .26f, 120, .92f, 5.5f, -.36f, .24f, .34f, .38f, -0.8f},
    {"Uplifting / Silver Edge",    rim,   4,  7.6f, 6.5f, .18f, .82f, 1.34f, 40, 116, 360, 14600, .18f, 108, .90f, 4.5f, -.38f, .32f, .38f, .30f, -0.6f},
    {"Uplifting / Morning Sun",    palm,  4,  9.0f, 8.4f, .22f, .64f, 1.20f, 44, 124, 210, 11800, .22f, 115, .94f, 5.0f, -.36f, .28f, .36f, .34f, -0.8f},
    {"Uplifting / Pure 138",       chalk, 4,  8.2f, 7.4f, .17f, .76f, 1.28f, 42, 122, 300, 13600, .20f, 112, .92f, 5.0f, -.36f, .31f, .38f, .32f, -0.6f},
    {"Uplifting / Opposite Walls", chalk, 3, 10.5f, 7.8f, .18f, 1.0f, 1.54f, 42, 124, 320, 12600, .19f, 115, .94f, 5.0f, -.36f, .28f, .35f, .42f, -0.8f},
    {"Uplifting / Silk Fans",      silk,  5,  8.2f, 8.2f, .24f, .98f, 1.56f, 42, 124, 260, 11800, .21f, 116, .93f, 5.0f, -.38f, .25f, .34f, .42f, -0.8f},
    {"Uplifting / Skyward",        chalk, 5,  8.0f, 7.6f, .21f, .92f, 1.46f, 42, 124, 310, 14400, .25f, 118, .95f, 5.0f, -.38f, .30f, .38f, .40f, -0.6f},
    {"Uplifting / Glass Ceiling",  rim,   4,  8.0f, 6.8f, .14f, .74f, 1.28f, 40, 118, 340, 14200, .24f, 116, .90f, 4.5f, -.38f, .30f, .36f, .34f, -0.8f},
    {"Uplifting / Starlight",      palm,  4,  8.6f, 8.2f, .23f, .88f, 1.38f, 44, 124, 260, 13200, .24f, 116, .94f, 5.0f, -.36f, .29f, .36f, .36f, -0.8f},
    {"Uplifting / Afterglow",      silk,  4,  8.8f, 8.4f, .24f, .70f, 1.30f, 44, 126, 240, 12200, .26f, 120, .92f, 5.5f, -.36f, .26f, .34f, .36f, -0.8f},

    // Rolling: punchy pocket claps with real slap weight and distinct flam articulation
    {"Rolling / Two Hands",        palm,  3, 10.2f, 8.0f, .18f, .58f,  .96f, 42, 114, 250, 10400, .12f,  95, .92f, 4.5f, -.38f, .32f, .38f, .18f, -0.9f},
    {"Rolling / Sixteenth",        chalk, 2,  7.5f, 5.8f, .08f, .28f,  .82f, 36,  92, 340, 11800, .06f,  70, .86f, 3.5f, -.44f, .30f, .36f, .10f, -0.9f},
    {"Rolling / Pocket Drag",      paper, 3, 10.5f, 8.4f, .26f, .46f,  .88f, 44, 118, 270,  9200, .14f,  98, .92f, 5.0f, -.36f, .36f, .42f, .16f, -0.9f},
    {"Rolling / Offbeat",          chalk, 3,  8.2f, 6.8f, .14f, .42f,  .92f, 40, 106, 320, 11400, .09f,  80, .89f, 4.0f, -.40f, .31f, .38f, .16f, -0.9f},
    {"Rolling / Brush Cut",        paper, 3,  8.5f, 6.6f, .18f, .48f,  .86f, 38, 102, 310,  9600, .08f,  75, .90f, 4.0f, -.40f, .34f, .40f, .14f, -0.9f},
    {"Rolling / Slipstream",       silk,  3,  8.8f, 7.4f, .16f, .54f, 1.02f, 42, 112, 280, 10800, .13f,  90, .90f, 4.5f, -.38f, .28f, .35f, .20f, -0.9f},
    {"Rolling / Soft Shoe",        palm,  3,  8.0f, 6.8f, .15f, .34f,  .78f, 40, 105, 250,  9400, .10f,  80, .88f, 4.0f, -.38f, .29f, .36f, .14f, -0.9f},
    {"Rolling / Clock Hands",      chalk, 3,  7.4f, 6.0f, .06f, .44f,  .86f, 36,  96, 350, 11200, .08f,  75, .88f, 3.5f, -.42f, .32f, .38f, .12f, -0.9f},

    // Progressive: warm, heavy chest, sculpted room width
    {"Progressive / Cupped Palm",  palm,  3,  9.2f, 8.4f, .24f, .32f,  .84f, 45, 118, 195,  9200, .14f,  95, .92f, 5.5f, -.36f, .30f, .36f, .18f, -0.9f},
    {"Progressive / Soft Heel",    silk,  3,  9.4f, 8.6f, .22f, .38f,  .86f, 45, 122, 210,  9400, .16f, 100, .92f, 5.5f, -.35f, .26f, .34f, .20f, -0.9f},
    {"Progressive / Archway",      silk,  4,  8.6f, 8.2f, .24f, .68f, 1.28f, 44, 124, 230, 11200, .25f, 118, .93f, 5.5f, -.36f, .28f, .35f, .34f, -0.8f},
    {"Progressive / Shoulder Room",palm,  4,  8.8f, 8.0f, .28f, .78f, 1.22f, 45, 122, 200,  9800, .18f, 108, .94f, 5.0f, -.36f, .31f, .38f, .30f, -0.8f},
    {"Progressive / Felt Pad",     silk,  2,  8.5f, 7.6f, .18f, .30f,  .74f, 42, 112, 220,  9000, .12f,  85, .88f, 4.5f, -.36f, .26f, .34f, .14f, -0.9f},
    {"Progressive / Booth Air",    palm,  3,  8.8f, 7.8f, .20f, .40f,  .96f, 44, 118, 215, 10200, .22f, 110, .90f, 5.0f, -.36f, .28f, .36f, .24f, -0.9f},
    {"Progressive / Side Street",  paper, 4,  8.6f, 7.8f, .28f, .84f, 1.32f, 44, 120, 280,  9200, .16f, 106, .93f, 5.0f, -.36f, .34f, .40f, .28f, -0.8f},
    {"Progressive / Low Roof",     palm,  4,  8.8f, 8.0f, .22f, .52f, 1.04f, 45, 124, 190,  8900, .24f, 114, .94f, 5.0f, -.36f, .30f, .38f, .26f, -0.8f},

    // Psy: biting, stereo-cutting transients backed by tight impactful body
    {"Psy / Fingertips",           rim,   2,  6.8f, 5.5f, .08f, .24f,  .76f, 36,  94, 420, 13200, .08f,  75, .88f, 3.5f, -.44f, .34f, .40f, .14f, -0.8f},
    {"Psy / Tripwire",             rim,   3,  8.4f, 6.0f, .12f, .54f,  .98f, 38, 102, 380, 12800, .10f,  85, .90f, 4.0f, -.42f, .36f, .42f, .20f, -0.8f},
    {"Psy / Pinwheel",             rim,   3,  7.8f, 5.8f, .14f, .76f, 1.24f, 36,  94, 410, 13400, .10f,  80, .90f, 3.8f, -.44f, .35f, .42f, .26f, -0.8f},
    {"Psy / Ratchet",              rim,   4,  6.6f, 5.4f, .07f, .48f,  .96f, 35,  88, 440, 13800, .07f,  70, .90f, 3.5f, -.46f, .38f, .44f, .20f, -0.6f},
    {"Psy / Split Second",         rim,   4,  7.8f, 6.2f, .12f, .78f, 1.14f, 38, 106, 360, 13200, .12f,  90, .92f, 4.0f, -.40f, .36f, .42f, .26f, -0.8f},
    {"Psy / Broken Pair",          chalk, 3,  9.2f, 6.8f, .34f, .80f, 1.16f, 40, 112, 330, 12200, .12f,  95, .92f, 4.5f, -.38f, .36f, .42f, .24f, -0.8f},
    {"Psy / Frosted Tile",         chalk, 3,  7.6f, 6.4f, .12f, .64f, 1.26f, 40, 112, 350, 12800, .22f, 112, .91f, 4.5f, -.38f, .34f, .40f, .28f, -0.8f},
    {"Psy / Five Across",          chalk, 5,  7.6f, 6.2f, .18f, .92f, 1.22f, 42, 112, 320, 12400, .14f,  98, .94f, 4.5f, -.38f, .35f, .42f, .30f, -0.6f},

    // Tech: slamming, saturated, industrial club claps
    {"Tech / Dry Chalk",           chalk, 3,  7.2f, 6.0f, .10f, .24f,  .68f, 40, 108, 340, 10600, .06f,  75, .90f, 4.5f, -.40f, .40f, .46f, .10f, -0.6f},
    {"Tech / Concrete",            paper, 3,  8.8f, 8.0f, .20f, .56f, 1.04f, 44, 120, 250,  8600, .22f, 108, .94f, 5.5f, -.36f, .42f, .48f, .22f, -0.6f},
    {"Tech / Front Row",           chalk, 4,  8.2f, 7.2f, .16f, .46f,  .98f, 42, 118, 280, 13000, .14f,  96, .93f, 5.0f, -.36f, .36f, .42f, .20f, -0.6f},
    {"Tech / Mono Print",          palm,  3,  8.0f, 6.8f, .10f,    0,     0, 42, 112, 260, 10200, .06f,  75, .92f, 4.5f, -.38f, .38f, .44f,    0, -0.6f},
    {"Tech / Short Plate",         chalk, 3,  8.4f, 7.6f, .12f, .48f, 1.14f, 42, 118, 290, 11600, .22f, 110, .92f, 5.0f, -.36f, .36f, .42f, .22f, -0.8f},
    {"Tech / Four Corners",        palm,  4,  8.6f, 7.2f, .20f, .84f, 1.12f, 44, 118, 240, 11000, .14f,  98, .94f, 5.0f, -.36f, .38f, .44f, .26f, -0.6f},
    {"Tech / Loose Knuckle",       paper, 3,  9.8f, 8.0f, .30f, .70f, 1.04f, 44, 118, 270,  8800, .14f,  95, .94f, 5.0f, -.36f, .40f, .46f, .22f, -0.8f},
    {"Tech / Corner Pocket",       paper, 4,  8.2f, 7.4f, .22f, .64f, 1.08f, 44, 118, 290,  9600, .20f, 106, .92f, 5.0f, -.36f, .38f, .44f, .22f, -0.6f},

    // Classic: fat 808/909 analog stacks & wide studio crowd claps
    {"Classic / Paper Fold",       paper, 4,  8.4f, 7.6f, .18f, .42f,  .88f, 44, 116, 250,  9200, .10f,  85, .92f, 5.0f, -.36f, .36f, .42f, .18f, -0.8f},
    {"Classic / Wood Block",       paper, 2,  8.0f, 6.8f, .14f, .28f,  .74f, 40, 104, 260,  8400, .12f,  85, .88f, 4.5f, -.38f, .34f, .40f, .12f, -0.8f},
    {"Classic / Lean Back",        silk,  3, 10.4f, 8.2f, .26f, .64f, 1.04f, 44, 120, 220,  9800, .16f, 102, .94f, 5.0f, -.36f, .30f, .36f, .26f, -0.8f},
    {"Classic / Palm Circle",      palm,  5,  8.2f, 8.0f, .30f, .88f, 1.32f, 44, 124, 210, 10800, .20f, 114, .96f, 5.5f, -.36f, .32f, .38f, .34f, -0.6f},
    {"Classic / Staggered Line",   paper, 4,  8.6f, 7.6f, .26f, .94f, 1.26f, 44, 118, 260,  9600, .16f, 106, .93f, 5.0f, -.36f, .34f, .40f, .30f, -0.8f},
    {"Classic / Upper Deck",       chalk, 5,  8.2f, 7.2f, .22f, .94f, 1.46f, 44, 124, 280, 13400, .22f, 116, .95f, 5.0f, -.36f, .30f, .38f, .38f, -0.6f},
    {"Classic / Ghost Palm",       palm,  2,  7.6f, 6.2f, .16f, .22f,  .68f, 38,  98, 260,  9000, .08f,  75, .88f, 4.0f, -.38f, .30f, .36f, .12f, -0.9f},
    {"Classic / Last Accent",      palm,  4,  8.6f, 8.0f, .18f, .54f, 1.04f, 44, 120, 230, 10400, .16f, 104, .93f, 5.0f, -.36f, .32f, .38f, .26f, -0.8f}
};

FactoryPreset makePreset(const Clap& clap) {
    const auto& m = clap.material;
    auto values = std::vector<std::pair<juce::String, float>>{
        {kPreBursts, static_cast<float>(clap.bursts)}, {kPreSpread, clap.spacingMs}, {kPreDecay, clap.burstDecayMs},
        {kPreJitter, clap.jitter}, {kFlamSpread, clap.pan}, {kPreLevel, clap.preLevel},
        {kToneQ, m.resonance}, {kToneDecay, clap.bodyMs},
        {kTonePitchEnv, m.pitchSweep}, {kTonePitchDecay, 12}, {kToneHPF, 160},
        {kToneLPF, std::clamp(clap.highCutHz * 1.25f, 9500.0f, 18000.0f)}, {kToneLevel, m.bodyLevel},
        {kNoiseType, static_cast<float>(m.noise)}, {kNoiseAttack, 0.2f}, {kNoiseHold, clap.noiseHoldMs}, {kNoiseDecay, clap.noiseMs},
        {kNoiseCurve, clap.noiseCurve}, {kNoiseHPF, clap.lowCutHz}, {kNoiseLPF, clap.highCutHz}, {kNoiseStereoWidth, clap.width}, {kNoiseLevel, m.noiseLevel},
        {kClickType, static_cast<float>(m.click)}, {kClickFreq, m.clickHz}, {kClickLevel, m.clickLevel}, {kClickDecay, m.clickDecayMs},
        {kDistType, static_cast<float>(m.dist)}, {kDistDrive, clap.drive}, {kDistTone, 0}, {kDistMix, clap.driveMix},
        {kCompThresh, -16}, {kCompRatio, 3.2f}, {kCompAttack, 7}, {kCompRelease, 38}, {kCompMakeup, 3.2f}, {kCompMix, 0.80f},
        {kLimiterEnabled, 1}, {kLimiterCeiling, -0.6f}, {kLimiterRelease, 22}, {kReverbSize, 0.56f},
        {kReverbDecay, 320}, {kReverbDamp, 0.42f}, {kReverbGate, clap.gateMs}, {kReverbMix, clap.roomMix},
        {kSlapbackDivision, static_cast<float>(SlapbackDivision::Off)}, {kSlapbackMix, 0}, {kSlapbackFeedback, 0},
        {kClapDoubler, clap.doubler}, {kPitchHumanize, 8.0f},
        {kReverseMode, static_cast<float>(ReverseSyncMode::Off)}, {kReverseMix, 0},
        {kEqLowFreq, 80.0f}, {kEqLowDb, 0.0f}, {kEqMidFreq, 1000.0f}, {kEqMidDb, 0.0f},
        {kEqHighFreq, 7500.0f}, {kEqHighDb, 0.0f}, {kStereoWidth, 1.05f}, {kOutputGain, clap.outDb}
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
