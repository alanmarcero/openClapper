#include "Presets.h"
#include "Parameters.h"

namespace openclapper::presets {
namespace {
using namespace params;
using namespace core;

struct Material {
    NoiseType noise;
    ClickType click;
    float bodyLevel, resonance, noiseLevel, clickHz, clickLevel, clickDecayMs, pitchSweep;
};

constexpr Material palm{NoiseType::Pink, ClickType::Crack, 0.72f, 2.8f, 1, 3100, 0.22f, 0.8f, 4};
constexpr Material chalk{NoiseType::White, ClickType::Crack, 0.50f, 3.6f, 0.80f, 4500, 0.17f, 0.65f, 2};
constexpr Material rim{NoiseType::Velvet, ClickType::Needle, 0.23f, 2.2f, 0.79f, 5900, 0.13f, 0.5f, 7};
constexpr Material paper{NoiseType::LoFi, ClickType::Thump, 0.61f, 3.2f, 0.72f, 3600, 0.19f, 0.6f, 3};
constexpr Material silk{NoiseType::White, ClickType::Thump, 0.68f, 2.2f, 0.86f, 2400, 0.17f, 0.7f, 6};

struct Clap {
    const char* name;
    const Material& material;
    int bursts;
    float spacingMs, burstDecayMs, jitter, pan, width, bodyMs, noiseMs, lowCutHz, highCutHz, roomMix, gateMs;
};

constexpr Clap bank[]{
    // Close hands: small stereo images and dry early reflections.
    {"Close / Downbeat",       chalk, 3, 5.0f, 3.4f, .09f, .24f, .76f, 34,  78,  690,  9300, .03f,  45},
    {"Close / Cupped Palm",    palm,  2, 5.5f, 4.6f, .22f, .16f, .62f, 52,  80,  440,  7400, .02f,  45},
    {"Close / Dry Chalk",      chalk, 2, 4.5f, 2.5f, .05f, .10f, .42f, 27,  59, 1050,  8800,    0,  35},
    {"Close / Paper Fold",     paper, 3, 5.0f, 4.0f, .16f, .33f, .70f, 37,  72,  670,  7300,    0,  40},
    {"Close / Fingertips",     rim,   1, 4.0f, 2.0f, .03f, .05f, .46f, 18,  48, 1400, 11000, .02f,  35},
    {"Close / Soft Heel",      silk,  2, 5.5f, 4.7f, .21f, .22f, .62f, 45,  92,  530,  7500, .05f,  50},
    {"Close / Mono Print",     palm,  3, 5.0f, 3.0f, .04f,    0,    0, 39,  65,  610,  8600,    0,  40},
    {"Close / Wood Block",     paper, 1, 5.0f, 3.0f, .11f, .18f, .51f, 29,  53,  770,  6100, .04f,  40},
    {"Close / Front Row",      chalk, 4, 5.0f, 3.2f, .13f, .38f, .84f, 41,  84,  740, 11400, .06f,  55},

    // Flams: the spacing and relative placement supply the movement.
    {"Flam / Two Hands",       palm,  2, 8.0f, 3.8f, .13f, .55f, .86f, 38,  72,  590,  8600, .04f,  55},
    {"Flam / Loose Knuckle",   paper, 3, 7.0f, 4.6f, .32f, .66f, .89f, 42,  83,  720,  6800, .05f,  55},
    {"Flam / Tripwire",        rim,   3, 5.5f, 2.1f, .06f, .42f, .76f, 20,  54, 1220, 10800, .03f,  40},
    {"Flam / Lean Back",       silk,  2, 9.0f, 4.8f, .26f, .60f, .88f, 43,  79,  550,  8100, .06f,  60},
    {"Flam / Broken Pair",     chalk, 2, 6.0f, 2.8f, .36f, .74f, .96f, 30,  69,  920, 10300, .03f,  50},
    {"Flam / Four Corners",    palm,  4, 5.5f, 3.4f, .18f, .82f, .95f, 42,  82,  580,  9300, .04f,  60},
    {"Flam / Five Across",     chalk, 5, 5.0f, 2.6f, .16f, .90f, 1.04f, 32, 67,  810, 10500, .05f,  60},
    {"Flam / Pocket Drag",     paper, 2, 9.5f, 5.2f, .24f, .38f, .72f, 40,  85,  640,  7100, .06f,  65},
    {"Flam / Split Second",    rim,   4, 5.0f, 2.4f, .07f, .71f, .94f, 23,  60, 1120, 11200, .03f,  50},

    // Ensembles: stagger several strikes across the stereo image.
    {"Wide / Balcony",         silk,  4, 5.0f, 4.1f, .23f, .81f, 1.28f, 43, 102,  640, 11100, .08f,  80},
    {"Wide / Opposite Walls",  chalk, 2, 5.5f, 3.1f, .10f, 1.0f, 1.45f, 32,  90,  810, 10700, .06f,  75},
    {"Wide / Palm Circle",     palm,  5, 5.0f, 4.7f, .30f, .88f, 1.22f, 50, 100,  490,  8900, .09f,  80},
    {"Wide / Staggered Line",  paper, 4, 5.0f, 3.6f, .26f, .95f, 1.17f, 35,  79,  760,  7800, .05f,  70},
    {"Wide / Silver Edge",     rim,   3, 4.5f, 2.5f, .14f, .83f, 1.30f, 23,  68, 1160, 12800, .05f,  60},
    {"Wide / Upper Deck",      chalk, 5, 5.0f, 3.0f, .21f, .93f, 1.39f, 41, 106,  870, 12200, .11f,  90},
    {"Wide / Shoulder Room",   palm,  3, 5.5f, 4.0f, .28f, .76f, 1.11f, 53,  91,  520,  8100, .06f,  70},
    {"Wide / Silk Fans",       silk,  4, 5.0f, 4.4f, .20f, .98f, 1.48f, 36,  97,  760,  9300, .07f,  85},
    {"Wide / Side Street",     paper, 3, 5.0f, 3.7f, .29f, .86f, 1.25f, 38,  86,  820,  7100, .04f,  70},

    // Gates: an audible early room with a deliberately short closing time.
    {"Gate / Short Plate",     chalk, 3, 5.0f, 3.8f, .08f, .42f, 1.04f, 38,  85,  720, 10100, .21f,  95},
    {"Gate / Concrete",        paper, 2, 5.5f, 4.5f, .19f, .50f, .90f, 48,  83,  630,  6900, .25f,  90},
    {"Gate / Glass Ceiling",   rim,   3, 4.5f, 2.7f, .08f, .67f, 1.18f, 26,  79, 1040, 12500, .23f, 100},
    {"Gate / Booth Air",       palm,  2, 5.0f, 3.3f, .16f, .22f, .74f, 46,  77,  500,  8500, .16f,  70},
    {"Gate / Archway",         silk,  4, 5.0f, 4.0f, .23f, .64f, 1.19f, 50, 105,  610,  9700, .26f, 120},
    {"Gate / Low Roof",        palm,  3, 5.5f, 4.2f, .19f, .37f, .82f, 55,  89,  460,  7000, .22f,  80},
    {"Gate / Frosted Tile",    chalk, 2, 4.5f, 2.6f, .07f, .54f, 1.12f, 29,  70,  980, 11000, .24f,  85},
    {"Gate / Corner Pocket",   paper, 4, 5.0f, 3.3f, .21f, .59f, .94f, 37,  82,  750,  8000, .20f,  75},
    {"Gate / High Gallery",    silk,  3, 5.0f, 4.8f, .26f, .74f, 1.27f, 45, 110,  670, 10600, .28f, 125},

    // Short hits leave space for sequenced sixteenths and triplets.
    {"Roll / Sixteenth",       chalk, 1, 4.5f, 2.2f, .02f, .12f, .67f, 21,  43,  940, 10500,    0,  30},
    {"Roll / Soft Shoe",       palm,  2, 4.5f, 2.6f, .11f, .20f, .58f, 28,  51,  620,  7600, .02f,  35},
    {"Roll / Ratchet",         rim,   3, 4.0f, 2.0f, .03f, .36f, .81f, 16,  37, 1420, 11800,    0,  25},
    {"Roll / Felt Pad",        silk,  1, 5.0f, 3.1f, .14f, .16f, .48f, 32,  58,  510,  7300, .03f,  35},
    {"Roll / Brush Cut",       paper, 2, 5.0f, 2.6f, .15f, .39f, .70f, 25,  47,  810,  7800,    0,  30},
    {"Roll / Pinwheel",        rim,   2, 5.0f, 2.1f, .08f, .70f, 1.09f, 18,  42, 1280, 11300, .02f,  30},
    {"Roll / Ghost Palm",      palm,  1, 4.0f, 2.2f, .13f, .08f, .37f, 24,  45,  700,  7200,    0,  25},
    {"Roll / Clock Hands",     chalk, 3, 4.5f, 2.3f, .03f, .32f, .69f, 20,  48, 1010,  9500, .02f,  35},
    {"Roll / Last Accent",     silk,  2, 5.0f, 3.3f, .12f, .48f, .89f, 33,  61,  650,  9200, .05f,  45}
};

FactoryPreset makePreset(const Clap& clap) {
    const auto& m = clap.material;
    auto values = std::vector<std::pair<juce::String, float>>{
        {kPreBursts, static_cast<float>(clap.bursts)}, {kPreSpread, clap.spacingMs}, {kPreDecay, clap.burstDecayMs},
        {kPreJitter, clap.jitter}, {kFlamSpread, clap.pan}, {kPreLevel, 0.73f},
        {kToneQ, m.resonance}, {kToneDecay, clap.bodyMs},
        {kTonePitchEnv, m.pitchSweep}, {kTonePitchDecay, 9}, {kToneHPF, 290},
        {kToneLPF, std::clamp(clap.highCutHz * 1.35f, 8500.0f, 18000.0f)}, {kToneLevel, m.bodyLevel},
        {kNoiseType, static_cast<float>(m.noise)}, {kNoiseAttack, 0.3f}, {kNoiseHold, 1.5f}, {kNoiseDecay, clap.noiseMs},
        {kNoiseCurve, -0.85f}, {kNoiseHPF, clap.lowCutHz}, {kNoiseLPF, clap.highCutHz}, {kNoiseStereoWidth, clap.width}, {kNoiseLevel, m.noiseLevel},
        {kClickType, static_cast<float>(m.click)}, {kClickFreq, m.clickHz}, {kClickLevel, m.clickLevel}, {kClickDecay, m.clickDecayMs},
        {kDistType, static_cast<float>(DistortionType::Tape)}, {kDistDrive, 0.07f}, {kDistTone, 0}, {kDistMix, 0.08f},
        {kCompThresh, -13}, {kCompRatio, 2.3f}, {kCompAttack, 6}, {kCompRelease, 45}, {kCompMakeup, 1.5f}, {kCompMix, 0.70f},
        {kLimiterEnabled, 1}, {kLimiterCeiling, -1}, {kLimiterRelease, 18}, {kReverbSize, 0.47f},
        {kReverbDecay, 240}, {kReverbDamp, 0.46f}, {kReverbGate, clap.gateMs}, {kReverbMix, clap.roomMix},
        {kSlapbackDivision, static_cast<float>(SlapbackDivision::Off)}, {kSlapbackMix, 0}, {kSlapbackFeedback, 0},
        {kClapDoubler, 0}, {kPitchHumanize, 0},
        {kReverseMode, static_cast<float>(ReverseSyncMode::Off)}, {kReverseMix, 0}, {kStereoWidth, 1}, {kOutputGain, -1}
    };
    for (const auto& parameter : kPerformanceParameters) values.emplace_back(parameter.id, parameter.initial);
    values.emplace_back("roll_division", 0);
    for (size_t i = 0; i < kLayerNames.size(); ++i) {
        values.emplace_back(kLayerMute[i], 0);
        values.emplace_back(kLayerSolo[i], 0);
        if (i < kLayerVelocity.size()) values.emplace_back(kLayerVelocity[i], 1);
    }
    return {clap.name, std::move(values)};
}
} // namespace

const std::vector<FactoryPreset>& factoryPresets() {
    static const auto presets = [] {
        std::vector<FactoryPreset> result;
        result.reserve(std::size(bank));
        for (const auto& clap : bank) result.push_back(makePreset(clap));
        return result;
    }();
    return presets;
}

void apply(const FactoryPreset& preset, juce::AudioProcessorValueTreeState& state) {
    for (const auto& [paramId, value] : preset.values) {
        if (auto* param = state.getParameter(paramId)) {
            param->setValueNotifyingHost(param->convertTo0to1(value));
        }
    }
}

} // namespace openclapper::presets
