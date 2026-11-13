#include "Presets.h"
#include "Parameters.h"
#include <algorithm>
#include <initializer_list>

namespace openclapper::presets {

using namespace openclapper::params;
using namespace openclapper::core;

namespace {

using Override = std::pair<const char*, float>;

template <typename Choice>
constexpr float presetChoice(Choice choice) noexcept {
    return static_cast<float>(choice);
}

std::vector<std::pair<juce::String, float>> defaultValues() {
    return {
        { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.15f }, { kFlamSpread, 0.45f }, { kPreLevel, 0.85f },
        { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 3.2f }, { kToneDecay, 120.0f }, { kToneLevel, 0.75f },
        { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 180.0f },
        { kNoiseCurve, -0.3f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 9500.0f }, { kNoiseStereoWidth, 1.0f }, { kNoiseLevel, 0.85f },
        { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.4f }, { kClickFreq, 4500.0f },
        { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.12f }, { kDistTone, 0.0f }, { kDistMix, 0.2f },
        { kCompThresh, -14.0f }, { kCompRatio, 4.0f }, { kCompAttack, 5.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 3.0f }, { kCompMix, 1.0f },
        { kLimiterCeiling, -0.3f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
        { kReverbSize, 0.65f }, { kReverbDecay, 450.0f }, { kReverbDamp, 0.4f }, { kReverbGate, 160.0f }, { kReverbMix, 0.35f },
        { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f },
        { kReverseMode, presetChoice(ReverseSyncMode::Off) }, { kReverseMix, 0.5f }, { kStereoWidth, 1.0f }, { kOutputGain, 0.0f }
    };
}

FactoryPreset makePreset(const char* name, std::initializer_list<Override> overrides) {
    auto values = defaultValues();
    for (const auto& [paramId, value] : overrides) {
        const auto found = std::find_if(values.begin(), values.end(), [paramId](const auto& entry) {
            return entry.first == paramId;
        });
        if (found != values.end()) {
            found->second = value;
        }
    }
    return { name, std::move(values) };
}

} // namespace

const std::vector<FactoryPreset>& factoryPresets() {
    static const std::vector<FactoryPreset> presets = {
        makePreset("01 Mainstage Anthem Gate", {
            { kPreBursts, 3.0f }, { kPreSpread, 10.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.82f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 4.0f }, { kToneDecay, 125.0f }, { kToneLevel, 0.78f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 170.0f }, { kNoiseHPF, 380.0f }, { kNoiseLPF, 10500.0f }, { kNoiseLevel, 0.88f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.48f }, { kClickFreq, 5200.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.10f }, { kDistTone, 0.05f }, { kDistMix, 0.18f },
            { kCompThresh, -15.0f }, { kCompRatio, 4.5f }, { kCompAttack, 3.5f }, { kCompRelease, 70.0f }, { kCompMakeup, 3.5f },
            { kReverbSize, 0.68f }, { kReverbDecay, 420.0f }, { kReverbGate, 150.0f }, { kReverbMix, 0.32f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.05f }
        }),
        makePreset("02 Uplift Wide Gate", {
            { kPreBursts, 4.0f }, { kPreSpread, 14.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.90f },
            { kToneQ, 3.8f }, { kToneDecay, 180.0f }, { kToneLevel, 0.70f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 0.4f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 240.0f }, { kNoiseHPF, 320.0f }, { kNoiseLPF, 12500.0f }, { kNoiseStereoWidth, 1.6f }, { kNoiseLevel, 0.90f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.36f }, { kClickFreq, 4900.0f },
            { kDistDrive, 0.08f }, { kDistTone, 0.10f }, { kDistMix, 0.14f },
            { kCompThresh, -14.0f }, { kCompRatio, 3.2f }, { kCompAttack, 8.0f }, { kCompRelease, 110.0f }, { kCompMakeup, 2.4f }, { kCompMix, 0.90f },
            { kReverbSize, 0.86f }, { kReverbDecay, 900.0f }, { kReverbDamp, 0.28f }, { kReverbGate, 240.0f }, { kReverbMix, 0.48f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.55f }, { kOutputGain, -0.5f }
        }),
        makePreset("03 Euphoric Bloom 1/8 Slap", {
            { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.14f }, { kPreLevel, 0.88f },
            { kToneMode, presetChoice(ToneMode::SaturatedTri) }, { kToneQ, 4.8f }, { kToneDecay, 155.0f }, { kToneLevel, 0.86f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseHold, 3.0f }, { kNoiseDecay, 220.0f }, { kNoiseCurve, -0.18f }, { kNoiseHPF, 360.0f }, { kNoiseLPF, 13000.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.88f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.42f }, { kClickFreq, 5600.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.14f }, { kDistTone, 0.08f }, { kDistMix, 0.20f },
            { kCompThresh, -16.0f }, { kCompRatio, 4.8f }, { kCompAttack, 4.0f }, { kCompRelease, 85.0f }, { kCompMakeup, 4.0f },
            { kReverbSize, 0.78f }, { kReverbDecay, 720.0f }, { kReverbGate, 210.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Eighth) }, { kSlapbackMix, 0.22f }, { kStereoWidth, 1.35f }
        }),
        makePreset("04 Festival Stack", {
            { kPreBursts, 5.0f }, { kPreSpread, 16.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.24f }, { kPreLevel, 0.96f }, { kFlamSpread, 0.68f },
            { kToneMode, presetChoice(ToneMode::SubThump) }, { kToneQ, 2.4f }, { kToneDecay, 210.0f }, { kToneLevel, 0.92f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseHold, 5.0f }, { kNoiseDecay, 290.0f }, { kNoiseHPF, 260.0f }, { kNoiseLPF, 9000.0f }, { kNoiseStereoWidth, 1.5f }, { kNoiseLevel, 0.95f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.52f }, { kClickFreq, 3200.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.22f }, { kDistTone, -0.08f }, { kDistMix, 0.28f },
            { kCompThresh, -21.0f }, { kCompRatio, 5.5f }, { kCompAttack, 6.0f }, { kCompRelease, 100.0f }, { kCompMakeup, 5.5f },
            { kLimiterCeiling, -0.2f }, { kReverbSize, 0.90f }, { kReverbDecay, 880.0f }, { kReverbGate, 240.0f }, { kReverbMix, 0.43f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.35f }, { kOutputGain, 0.5f }
        }),
        makePreset("05 138 Peak Punch", {
            { kPreBursts, 3.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.84f },
            { kToneQ, 4.5f }, { kToneDecay, 100.0f }, { kToneLevel, 0.82f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 145.0f }, { kNoiseCurve, -0.42f }, { kNoiseHPF, 410.0f }, { kNoiseLPF, 11000.0f }, { kNoiseStereoWidth, 1.18f }, { kNoiseLevel, 0.90f },
            { kClickLevel, 0.54f }, { kClickFreq, 6100.0f },
            { kDistDrive, 0.12f }, { kDistTone, 0.08f }, { kDistMix, 0.18f },
            { kCompThresh, -15.0f }, { kCompRatio, 5.0f }, { kCompAttack, 2.5f }, { kCompRelease, 65.0f }, { kCompMakeup, 3.2f },
            { kReverbSize, 0.58f }, { kReverbDecay, 340.0f }, { kReverbGate, 120.0f }, { kReverbMix, 0.25f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.16f }, { kStereoWidth, 1.18f }
        }),
        makePreset("06 Dutch Hard Gate", {
            { kPreBursts, 2.0f }, { kPreSpread, 7.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.78f },
            { kToneMode, presetChoice(ToneMode::SaturatedTri) }, { kToneQ, 6.8f }, { kToneDecay, 90.0f }, { kToneLevel, 0.98f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseDecay, 115.0f }, { kNoiseHPF, 500.0f }, { kNoiseLPF, 12500.0f }, { kNoiseStereoWidth, 0.9f }, { kNoiseLevel, 0.75f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.72f }, { kClickFreq, 7200.0f },
            { kDistType, presetChoice(DistortionType::HardClip) }, { kDistDrive, 0.25f }, { kDistTone, 0.15f }, { kDistMix, 0.30f },
            { kCompThresh, -20.0f }, { kCompRatio, 8.0f }, { kCompAttack, 1.0f }, { kCompRelease, 48.0f }, { kCompMakeup, 5.5f },
            { kLimiterCeiling, -0.15f }, { kReverbSize, 0.42f }, { kReverbDecay, 220.0f }, { kReverbGate, 72.0f }, { kReverbMix, 0.20f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 0.95f }, { kOutputGain, 0.3f }
        }),
        makePreset("07 Progressive Velvet Air", {
            { kPreBursts, 3.0f }, { kPreSpread, 14.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.20f }, { kPreLevel, 0.76f },
            { kToneQ, 2.8f }, { kToneDecay, 145.0f }, { kToneLevel, 0.63f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseAttack, 2.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 230.0f }, { kNoiseCurve, -0.15f }, { kNoiseHPF, 300.0f }, { kNoiseLPF, 8200.0f }, { kNoiseStereoWidth, 1.4f }, { kNoiseLevel, 0.82f },
            { kClickLevel, 0.24f }, { kClickFreq, 3600.0f },
            { kDistDrive, 0.05f }, { kDistTone, -0.12f }, { kDistMix, 0.12f },
            { kCompThresh, -12.0f }, { kCompRatio, 3.0f }, { kCompAttack, 10.0f }, { kCompRelease, 110.0f }, { kCompMakeup, 2.0f }, { kCompMix, 0.85f },
            { kReverbSize, 0.72f }, { kReverbDecay, 580.0f }, { kReverbDamp, 0.52f }, { kReverbGate, 190.0f }, { kReverbMix, 0.38f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.25f }, { kOutputGain, -0.6f }
        }),
        makePreset("08 Future Rave Cyber Slap", {
            { kPreBursts, 3.0f }, { kPreSpread, 11.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.13f }, { kPreLevel, 0.88f },
            { kToneMode, presetChoice(ToneMode::CyberSquare) }, { kToneQ, 5.4f }, { kToneDecay, 135.0f }, { kToneLevel, 0.86f },
            { kNoiseType, presetChoice(NoiseType::LoFi) }, { kNoiseHold, 3.0f }, { kNoiseDecay, 170.0f }, { kNoiseCurve, -0.35f }, { kNoiseHPF, 460.0f }, { kNoiseLPF, 11800.0f }, { kNoiseStereoWidth, 1.45f }, { kNoiseLevel, 0.86f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.60f }, { kClickFreq, 5900.0f },
            { kDistType, presetChoice(DistortionType::Wavefold) }, { kDistDrive, 0.22f }, { kDistTone, 0.18f }, { kDistMix, 0.26f },
            { kCompThresh, -17.0f }, { kCompRatio, 6.0f }, { kCompAttack, 2.5f }, { kCompRelease, 65.0f }, { kCompMakeup, 4.5f },
            { kReverbSize, 0.76f }, { kReverbDecay, 510.0f }, { kReverbGate, 155.0f }, { kReverbMix, 0.36f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedEighth) }, { kSlapbackMix, 0.26f }, { kStereoWidth, 1.30f }
        }),
        makePreset("09 Euro Gated Classic", {
            { kPreBursts, 2.0f }, { kPreSpread, 15.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.80f },
            { kToneQ, 3.2f }, { kToneDecay, 150.0f }, { kToneLevel, 0.78f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseHold, 4.0f }, { kNoiseDecay, 205.0f }, { kNoiseHPF, 340.0f }, { kNoiseLPF, 10000.0f }, { kNoiseStereoWidth, 1.1f }, { kNoiseLevel, 0.86f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.44f }, { kClickFreq, 4700.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.09f }, { kDistTone, 0.03f }, { kDistMix, 0.17f },
            { kCompThresh, -15.0f }, { kCompRatio, 4.0f }, { kCompAttack, 5.0f }, { kCompRelease, 75.0f }, { kCompMakeup, 3.0f },
            { kReverbSize, 0.72f }, { kReverbDecay, 650.0f }, { kReverbDamp, 0.38f }, { kReverbGate, 165.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.18f }, { kStereoWidth, 1.12f }
        }),
        makePreset("10 Hands Up Shimmer", {
            { kPreBursts, 4.0f }, { kPreSpread, 13.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.16f }, { kPreLevel, 0.91f }, { kFlamSpread, 0.62f },
            { kToneQ, 4.2f }, { kToneDecay, 145.0f }, { kToneLevel, 0.76f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 0.6f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 270.0f }, { kNoiseHPF, 330.0f }, { kNoiseLPF, 15000.0f }, { kNoiseStereoWidth, 1.72f }, { kNoiseLevel, 0.91f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.34f }, { kClickFreq, 6400.0f },
            { kDistDrive, 0.07f }, { kDistTone, 0.16f }, { kDistMix, 0.13f },
            { kCompThresh, -13.0f }, { kCompRatio, 3.6f }, { kCompAttack, 7.0f }, { kCompRelease, 100.0f }, { kCompMakeup, 2.4f },
            { kReverbSize, 0.88f }, { kReverbDecay, 820.0f }, { kReverbDamp, 0.30f }, { kReverbGate, 210.0f }, { kReverbMix, 0.46f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedSixteenth) }, { kSlapbackMix, 0.18f }, { kStereoWidth, 1.60f }, { kOutputGain, -0.4f }
        }),
        makePreset("11 Progressive Glass", {
            { kPreBursts, 2.0f }, { kPreSpread, 10.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.72f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 6.2f }, { kToneDecay, 110.0f }, { kToneLevel, 0.70f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 190.0f }, { kNoiseCurve, -0.20f }, { kNoiseHPF, 560.0f }, { kNoiseLPF, 14500.0f }, { kNoiseStereoWidth, 1.15f }, { kNoiseLevel, 0.76f },
            { kClickLevel, 0.28f }, { kClickFreq, 6800.0f }, { kDistDrive, 0.04f }, { kDistTone, 0.14f }, { kDistMix, 0.10f },
            { kCompThresh, -11.0f }, { kCompRatio, 2.8f }, { kCompAttack, 12.0f }, { kCompRelease, 130.0f }, { kCompMakeup, 1.8f }, { kCompMix, 0.82f },
            { kReverbSize, 0.82f }, { kReverbDecay, 780.0f }, { kReverbGate, 230.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Eighth) }, { kSlapbackMix, 0.16f }, { kStereoWidth, 1.20f }
        }),
        makePreset("12 Sunset Wide", {
            { kPreBursts, 3.0f }, { kPreSpread, 17.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.25f }, { kPreLevel, 0.72f }, { kFlamSpread, 0.78f },
            { kToneMode, presetChoice(ToneMode::SubThump) }, { kToneQ, 2.2f }, { kToneDecay, 240.0f }, { kToneLevel, 0.63f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 1.2f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 380.0f }, { kNoiseCurve, 0.08f }, { kNoiseHPF, 240.0f }, { kNoiseLPF, 7600.0f }, { kNoiseStereoWidth, 1.85f }, { kNoiseLevel, 0.78f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.24f }, { kClickFreq, 2900.0f },
            { kDistDrive, 0.03f }, { kDistTone, -0.12f }, { kDistMix, 0.08f },
            { kCompThresh, -10.0f }, { kCompRatio, 2.5f }, { kCompAttack, 16.0f }, { kCompRelease, 180.0f }, { kCompMakeup, 1.0f }, { kCompMix, 0.80f },
            { kReverbSize, 0.92f }, { kReverbDecay, 1200.0f }, { kReverbDamp, 0.38f }, { kReverbGate, 300.0f }, { kReverbMix, 0.42f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.65f }, { kOutputGain, -1.0f }
        }),
        makePreset("13 Rolling Progressive", {
            { kPreBursts, 2.0f }, { kPreSpread, 7.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.04f }, { kPreLevel, 0.68f },
            { kToneQ, 3.6f }, { kToneDecay, 85.0f }, { kToneLevel, 0.72f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseDecay, 95.0f }, { kNoiseCurve, -0.48f }, { kNoiseHPF, 440.0f }, { kNoiseLPF, 10800.0f }, { kNoiseStereoWidth, 0.92f }, { kNoiseLevel, 0.72f },
            { kClickLevel, 0.38f }, { kClickFreq, 5600.0f }, { kDistDrive, 0.09f }, { kDistTone, 0.08f }, { kDistMix, 0.16f },
            { kCompThresh, -16.0f }, { kCompRatio, 4.0f }, { kCompAttack, 1.2f }, { kCompRelease, 45.0f }, { kCompMakeup, 3.4f },
            { kReverbSize, 0.48f }, { kReverbDecay, 240.0f }, { kReverbGate, 85.0f }, { kReverbMix, 0.20f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.24f }, { kStereoWidth, 0.98f }
        }),
        makePreset("14 Deep Pool Clap", {
            { kPreBursts, 2.0f }, { kPreSpread, 18.0f }, { kPreDecay, 15.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.58f },
            { kToneMode, presetChoice(ToneMode::SubThump) }, { kToneQ, 1.8f }, { kToneDecay, 290.0f }, { kToneLevel, 0.78f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 1.8f }, { kNoiseHold, 5.0f }, { kNoiseDecay, 330.0f }, { kNoiseHPF, 200.0f }, { kNoiseLPF, 6800.0f }, { kNoiseStereoWidth, 1.1f }, { kNoiseLevel, 0.70f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.20f }, { kClickFreq, 2600.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.06f }, { kDistTone, -0.18f }, { kDistMix, 0.10f },
            { kCompThresh, -11.0f }, { kCompRatio, 2.8f }, { kCompAttack, 11.0f }, { kCompRelease, 150.0f }, { kCompMakeup, 1.8f },
            { kReverbSize, 0.78f }, { kReverbDecay, 900.0f }, { kReverbDamp, 0.58f }, { kReverbGate, 260.0f }, { kReverbMix, 0.35f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.10f }, { kOutputGain, -0.8f }
        }),
        makePreset("15 Late Night Pulse", {
            { kPreBursts, 2.0f }, { kPreSpread, 8.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.62f },
            { kToneQ, 3.0f }, { kToneDecay, 95.0f }, { kToneLevel, 0.64f },
            { kNoiseType, presetChoice(NoiseType::LoFi) }, { kNoiseDecay, 125.0f }, { kNoiseCurve, -0.38f }, { kNoiseHPF, 420.0f }, { kNoiseLPF, 8200.0f }, { kNoiseStereoWidth, 0.78f }, { kNoiseLevel, 0.65f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.28f }, { kClickFreq, 4300.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.14f }, { kDistTone, -0.20f }, { kDistMix, 0.20f },
            { kCompThresh, -18.0f }, { kCompRatio, 4.0f }, { kCompAttack, 3.0f }, { kCompRelease, 60.0f }, { kCompMakeup, 2.5f },
            { kReverbSize, 0.38f }, { kReverbDecay, 210.0f }, { kReverbDamp, 0.65f }, { kReverbGate, 75.0f }, { kReverbMix, 0.16f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.15f }, { kStereoWidth, 0.85f }
        }),
        makePreset("16 Aurora Whisper", {
            { kPreBursts, 3.0f }, { kPreSpread, 20.0f }, { kPreDecay, 16.0f }, { kPreJitter, 0.30f }, { kPreLevel, 0.54f }, { kFlamSpread, 0.82f },
            { kToneQ, 2.6f }, { kToneDecay, 250.0f }, { kToneLevel, 0.58f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseAttack, 3.0f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 410.0f }, { kNoiseCurve, 0.12f }, { kNoiseHPF, 260.0f }, { kNoiseLPF, 9000.0f }, { kNoiseStereoWidth, 1.75f }, { kNoiseLevel, 0.72f },
            { kClickLevel, 0.18f }, { kClickFreq, 3900.0f }, { kDistDrive, 0.02f }, { kDistMix, 0.06f },
            { kCompThresh, -9.0f }, { kCompRatio, 2.0f }, { kCompAttack, 18.0f }, { kCompRelease, 210.0f }, { kCompMakeup, 0.8f }, { kCompMix, 0.75f },
            { kReverbSize, 0.94f }, { kReverbDecay, 1450.0f }, { kReverbDamp, 0.50f }, { kReverbGate, 320.0f }, { kReverbMix, 0.38f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedEighth) }, { kSlapbackMix, 0.12f }, { kStereoWidth, 1.7f }, { kOutputGain, -1.5f }
        }),
        makePreset("17 Supersaw Haze", {
            { kPreBursts, 4.0f }, { kPreSpread, 12.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.19f }, { kPreLevel, 0.84f }, { kFlamSpread, 0.58f },
            { kToneMode, presetChoice(ToneMode::CyberSquare) }, { kToneQ, 3.3f }, { kToneDecay, 185.0f }, { kToneLevel, 0.70f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 0.8f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 310.0f }, { kNoiseHPF, 300.0f }, { kNoiseLPF, 15000.0f }, { kNoiseStereoWidth, 1.7f }, { kNoiseLevel, 0.84f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.30f }, { kClickFreq, 5200.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.10f }, { kDistTone, 0.13f }, { kDistMix, 0.16f },
            { kCompThresh, -14.0f }, { kCompRatio, 3.6f }, { kCompAttack, 7.0f }, { kCompRelease, 115.0f }, { kCompMakeup, 2.8f },
            { kReverbSize, 0.82f }, { kReverbDecay, 740.0f }, { kReverbGate, 205.0f }, { kReverbMix, 0.36f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.55f }
        }),
        makePreset("18 Progressive Dotted Slap", {
            { kPreBursts, 2.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.07f }, { kPreLevel, 0.74f },
            { kToneQ, 4.1f }, { kToneDecay, 115.0f }, { kToneLevel, 0.70f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 155.0f }, { kNoiseHPF, 390.0f }, { kNoiseLPF, 11500.0f }, { kNoiseStereoWidth, 1.2f }, { kNoiseLevel, 0.78f },
            { kClickLevel, 0.42f }, { kClickFreq, 5800.0f }, { kDistDrive, 0.08f }, { kDistTone, 0.04f }, { kDistMix, 0.14f },
            { kCompThresh, -15.0f }, { kCompRatio, 4.0f }, { kCompAttack, 3.0f }, { kCompRelease, 60.0f }, { kCompMakeup, 3.0f },
            { kReverbSize, 0.60f }, { kReverbDecay, 360.0f }, { kReverbGate, 125.0f }, { kReverbMix, 0.26f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedSixteenth) }, { kSlapbackMix, 0.27f }, { kStereoWidth, 1.08f }
        }),
        makePreset("19 Psy Fractal Snap", {
            { kPreBursts, 3.0f }, { kPreSpread, 8.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.78f },
            { kToneMode, presetChoice(ToneMode::SaturatedTri) }, { kToneQ, 8.5f }, { kToneDecay, 80.0f }, { kToneLevel, 0.94f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseDecay, 115.0f }, { kNoiseCurve, -0.52f }, { kNoiseHPF, 520.0f }, { kNoiseLPF, 15000.0f }, { kNoiseStereoWidth, 1.05f }, { kNoiseLevel, 0.80f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.64f }, { kClickFreq, 7600.0f },
            { kDistType, presetChoice(DistortionType::Wavefold) }, { kDistDrive, 0.23f }, { kDistTone, 0.24f }, { kDistMix, 0.28f },
            { kCompThresh, -18.0f }, { kCompRatio, 7.0f }, { kCompAttack, 1.0f }, { kCompRelease, 42.0f }, { kCompMakeup, 4.4f },
            { kReverbSize, 0.42f }, { kReverbDecay, 230.0f }, { kReverbDamp, 0.62f }, { kReverbGate, 80.0f }, { kReverbMix, 0.18f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.20f }, { kStereoWidth, 1.05f }
        }),
        makePreset("20 Offbeat Goa", {
            { kPreBursts, 4.0f }, { kPreSpread, 11.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.17f }, { kPreLevel, 0.83f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 5.6f }, { kToneDecay, 165.0f }, { kToneLevel, 0.82f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseHold, 3.0f }, { kNoiseDecay, 225.0f }, { kNoiseCurve, -0.24f }, { kNoiseHPF, 360.0f }, { kNoiseLPF, 12800.0f }, { kNoiseStereoWidth, 1.25f }, { kNoiseLevel, 0.86f },
            { kClickLevel, 0.50f }, { kClickFreq, 6400.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.15f }, { kDistTone, 0.15f }, { kDistMix, 0.21f },
            { kCompThresh, -17.0f }, { kCompRatio, 4.5f }, { kCompAttack, 3.0f }, { kCompRelease, 70.0f }, { kCompMakeup, 3.8f },
            { kReverbSize, 0.70f }, { kReverbDecay, 560.0f }, { kReverbGate, 180.0f }, { kReverbMix, 0.34f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedEighth) }, { kSlapbackMix, 0.25f },
            { kReverseMode, presetChoice(ReverseSyncMode::Beat4Auto) }, { kReverseMix, 0.28f }, { kStereoWidth, 1.28f }
        }),
        makePreset("21 Spiral Acid Bite", {
            { kPreBursts, 2.0f }, { kPreSpread, 6.0f }, { kPreDecay, 4.0f }, { kPreJitter, 0.03f }, { kPreLevel, 0.72f },
            { kToneMode, presetChoice(ToneMode::SaturatedTri) }, { kToneQ, 9.0f }, { kToneDecay, 75.0f }, { kToneLevel, 0.96f },
            { kNoiseType, presetChoice(NoiseType::LoFi) }, { kNoiseDecay, 90.0f }, { kNoiseHPF, 620.0f }, { kNoiseLPF, 14200.0f }, { kNoiseStereoWidth, 0.85f }, { kNoiseLevel, 0.78f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.70f }, { kClickFreq, 8200.0f },
            { kDistType, presetChoice(DistortionType::HardClip) }, { kDistDrive, 0.24f }, { kDistTone, 0.28f }, { kDistMix, 0.27f },
            { kCompThresh, -17.0f }, { kCompRatio, 7.0f }, { kCompAttack, 1.0f }, { kCompRelease, 38.0f }, { kCompMakeup, 4.2f },
            { kReverbSize, 0.34f }, { kReverbDecay, 170.0f }, { kReverbGate, 55.0f }, { kReverbMix, 0.14f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.16f }, { kStereoWidth, 0.92f }
        }),
        makePreset("22 Cosmic Ping 1/4", {
            { kPreBursts, 2.0f }, { kPreSpread, 11.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.70f },
            { kToneMode, presetChoice(ToneMode::CyberSquare) }, { kToneQ, 5.2f }, { kToneDecay, 180.0f }, { kToneLevel, 0.82f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseDecay, 175.0f }, { kNoiseHPF, 500.0f }, { kNoiseLPF, 12800.0f }, { kNoiseStereoWidth, 1.1f }, { kNoiseLevel, 0.76f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.42f }, { kClickFreq, 9000.0f },
            { kDistType, presetChoice(DistortionType::Wavefold) }, { kDistDrive, 0.14f }, { kDistTone, 0.21f }, { kDistMix, 0.18f },
            { kCompThresh, -13.0f }, { kCompRatio, 3.4f }, { kCompAttack, 8.0f }, { kCompRelease, 130.0f }, { kCompMakeup, 2.0f },
            { kReverbSize, 0.88f }, { kReverbDecay, 700.0f }, { kReverbGate, 240.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Quarter) }, { kSlapbackMix, 0.27f }, { kStereoWidth, 1.25f }
        }),
        makePreset("23 Full-On Bloom", {
            { kPreBursts, 5.0f }, { kPreSpread, 13.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.90f }, { kFlamSpread, 0.70f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 4.6f }, { kToneDecay, 160.0f }, { kToneLevel, 0.85f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseHold, 4.0f }, { kNoiseDecay, 270.0f }, { kNoiseHPF, 340.0f }, { kNoiseLPF, 14500.0f }, { kNoiseStereoWidth, 1.55f }, { kNoiseLevel, 0.92f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.42f }, { kClickFreq, 5700.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.14f }, { kDistTone, 0.08f }, { kDistMix, 0.22f },
            { kCompThresh, -18.0f }, { kCompRatio, 5.0f }, { kCompAttack, 3.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 4.5f },
            { kReverbSize, 0.82f }, { kReverbDecay, 680.0f }, { kReverbGate, 210.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Eighth) }, { kSlapbackMix, 0.18f }, { kStereoWidth, 1.48f }
        }),
        makePreset("24 Forest Crackle", {
            { kPreBursts, 4.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.32f }, { kPreLevel, 0.82f }, { kFlamSpread, 0.74f },
            { kToneQ, 5.2f }, { kToneDecay, 105.0f }, { kToneLevel, 0.68f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseDecay, 145.0f }, { kNoiseCurve, -0.55f }, { kNoiseHPF, 600.0f }, { kNoiseLPF, 12500.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.84f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.55f }, { kClickFreq, 7000.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.12f }, { kDistTone, 0.08f }, { kDistMix, 0.18f },
            { kCompThresh, -16.0f }, { kCompRatio, 4.3f }, { kCompAttack, 2.0f }, { kCompRelease, 58.0f }, { kCompMakeup, 3.4f },
            { kReverbSize, 0.60f }, { kReverbDecay, 400.0f }, { kReverbGate, 135.0f }, { kReverbMix, 0.27f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedSixteenth) }, { kSlapbackMix, 0.15f }, { kStereoWidth, 1.25f }
        }),
        makePreset("25 Night Psy Metal", {
            { kPreBursts, 3.0f }, { kPreSpread, 7.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.04f }, { kPreLevel, 0.70f },
            { kToneMode, presetChoice(ToneMode::CyberSquare) }, { kToneQ, 8.2f }, { kToneDecay, 95.0f }, { kToneLevel, 0.94f },
            { kNoiseType, presetChoice(NoiseType::LoFi) }, { kNoiseDecay, 120.0f }, { kNoiseHPF, 540.0f }, { kNoiseLPF, 15500.0f }, { kNoiseStereoWidth, 1.0f }, { kNoiseLevel, 0.82f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.62f }, { kClickFreq, 4400.0f },
            { kDistType, presetChoice(DistortionType::HardClip) }, { kDistDrive, 0.28f }, { kDistTone, 0.14f }, { kDistMix, 0.31f },
            { kCompThresh, -20.0f }, { kCompRatio, 8.5f }, { kCompAttack, 0.8f }, { kCompRelease, 40.0f }, { kCompMakeup, 5.2f },
            { kLimiterCeiling, -0.15f }, { kReverbSize, 0.38f }, { kReverbDecay, 190.0f }, { kReverbGate, 65.0f }, { kReverbMix, 0.16f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.02f }, { kOutputGain, 0.4f }
        }),
        makePreset("26 Dark Psy Pulse", {
            { kPreBursts, 2.0f }, { kPreSpread, 8.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.06f }, { kPreLevel, 0.62f },
            { kToneMode, presetChoice(ToneMode::SubThump) }, { kToneQ, 2.0f }, { kToneDecay, 190.0f }, { kToneLevel, 0.88f },
            { kNoiseType, presetChoice(NoiseType::LoFi) }, { kNoiseDecay, 180.0f }, { kNoiseCurve, -0.42f }, { kNoiseHPF, 230.0f }, { kNoiseLPF, 8200.0f }, { kNoiseStereoWidth, 0.8f }, { kNoiseLevel, 0.72f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.34f }, { kClickFreq, 2500.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.19f }, { kDistTone, -0.2f }, { kDistMix, 0.26f },
            { kCompThresh, -19.0f }, { kCompRatio, 6.0f }, { kCompAttack, 2.0f }, { kCompRelease, 55.0f }, { kCompMakeup, 4.0f },
            { kReverbSize, 0.48f }, { kReverbDecay, 260.0f }, { kReverbDamp, 0.68f }, { kReverbGate, 90.0f }, { kReverbMix, 0.20f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.17f }, { kStereoWidth, 0.88f }
        }),
        makePreset("27 Tech Trance Stab", {
            { kPreBursts, 2.0f }, { kPreSpread, 6.0f }, { kPreDecay, 4.0f }, { kPreJitter, 0.04f }, { kPreLevel, 0.75f },
            { kToneQ, 6.0f }, { kToneDecay, 72.0f }, { kToneLevel, 0.83f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 105.0f }, { kNoiseCurve, -0.48f }, { kNoiseHPF, 520.0f }, { kNoiseLPF, 13500.0f }, { kNoiseStereoWidth, 0.9f }, { kNoiseLevel, 0.78f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.68f }, { kClickFreq, 7600.0f },
            { kDistType, presetChoice(DistortionType::Wavefold) }, { kDistDrive, 0.18f }, { kDistTone, 0.24f }, { kDistMix, 0.22f },
            { kCompThresh, -17.0f }, { kCompRatio, 6.4f }, { kCompAttack, 1.0f }, { kCompRelease, 42.0f }, { kCompMakeup, 4.0f },
            { kReverbSize, 0.42f }, { kReverbDecay, 220.0f }, { kReverbGate, 75.0f }, { kReverbMix, 0.18f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.25f }, { kStereoWidth, 1.0f }
        }),
        makePreset("28 Rave Siren Pickup", {
            { kPreBursts, 3.0f }, { kPreSpread, 10.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.80f },
            { kToneMode, presetChoice(ToneMode::CyberSquare) }, { kToneQ, 4.8f }, { kToneDecay, 155.0f }, { kToneLevel, 0.80f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 0.5f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 230.0f }, { kNoiseHPF, 410.0f }, { kNoiseLPF, 14500.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.84f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.48f }, { kClickFreq, 8800.0f },
            { kDistType, presetChoice(DistortionType::Wavefold) }, { kDistDrive, 0.17f }, { kDistTone, 0.22f }, { kDistMix, 0.23f },
            { kCompThresh, -16.0f }, { kCompRatio, 4.5f }, { kCompAttack, 3.0f }, { kCompRelease, 75.0f }, { kCompMakeup, 3.5f },
            { kReverbSize, 0.72f }, { kReverbDecay, 600.0f }, { kReverbGate, 185.0f }, { kReverbMix, 0.32f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedEighth) }, { kSlapbackMix, 0.22f },
            { kReverseMode, presetChoice(ReverseSyncMode::BeatEnd) }, { kReverseMix, 0.34f }, { kStereoWidth, 1.25f }
        }),
        makePreset("29 Millennium Hands Up", {
            { kPreBursts, 4.0f }, { kPreSpread, 14.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.90f }, { kFlamSpread, 0.62f },
            { kToneQ, 4.1f }, { kToneDecay, 155.0f }, { kToneLevel, 0.77f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseHold, 4.0f }, { kNoiseDecay, 255.0f }, { kNoiseHPF, 330.0f }, { kNoiseLPF, 14200.0f }, { kNoiseStereoWidth, 1.58f }, { kNoiseLevel, 0.89f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.40f }, { kClickFreq, 5200.0f },
            { kDistDrive, 0.12f }, { kDistTone, 0.10f }, { kDistMix, 0.18f },
            { kCompThresh, -17.0f }, { kCompRatio, 4.8f }, { kCompAttack, 4.0f }, { kCompRelease, 85.0f }, { kCompMakeup, 4.0f },
            { kReverbSize, 0.84f }, { kReverbDecay, 760.0f }, { kReverbDamp, 0.34f }, { kReverbGate, 210.0f }, { kReverbMix, 0.43f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedSixteenth) }, { kSlapbackMix, 0.18f }, { kStereoWidth, 1.52f }
        }),
        makePreset("30 Classic 138 Gate", {
            { kPreBursts, 3.0f }, { kPreSpread, 13.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.12f }, { kPreLevel, 0.84f },
            { kToneQ, 3.5f }, { kToneDecay, 125.0f }, { kToneLevel, 0.78f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseHold, 2.0f }, { kNoiseDecay, 185.0f }, { kNoiseHPF, 360.0f }, { kNoiseLPF, 10000.0f }, { kNoiseStereoWidth, 1.05f }, { kNoiseLevel, 0.85f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.38f }, { kClickFreq, 5000.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.08f }, { kDistMix, 0.15f },
            { kCompThresh, -14.0f }, { kCompRatio, 4.0f }, { kCompAttack, 5.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 3.0f },
            { kReverbSize, 0.68f }, { kReverbDecay, 500.0f }, { kReverbDamp, 0.42f }, { kReverbGate, 160.0f }, { kReverbMix, 0.35f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.05f }
        }),
        makePreset("31 Dutch Lead Impact", {
            { kPreBursts, 3.0f }, { kPreSpread, 8.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.80f },
            { kToneMode, presetChoice(ToneMode::SaturatedTri) }, { kToneQ, 7.2f }, { kToneDecay, 105.0f }, { kToneLevel, 0.92f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 135.0f }, { kNoiseHPF, 470.0f }, { kNoiseLPF, 13500.0f }, { kNoiseStereoWidth, 1.0f }, { kNoiseLevel, 0.82f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.60f }, { kClickFreq, 6900.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.20f }, { kDistTone, 0.20f }, { kDistMix, 0.25f },
            { kCompThresh, -18.0f }, { kCompRatio, 6.0f }, { kCompAttack, 1.5f }, { kCompRelease, 48.0f }, { kCompMakeup, 4.5f },
            { kReverbSize, 0.52f }, { kReverbDecay, 300.0f }, { kReverbGate, 100.0f }, { kReverbMix, 0.23f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.18f }, { kStereoWidth, 1.0f }
        }),
        makePreset("32 Reverse Skyline", {
            { kPreBursts, 3.0f }, { kPreSpread, 15.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.16f }, { kPreLevel, 0.82f },
            { kToneQ, 3.6f }, { kToneDecay, 160.0f }, { kToneLevel, 0.74f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 1.2f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 300.0f }, { kNoiseHPF, 300.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 1.5f }, { kNoiseLevel, 0.80f },
            { kClickLevel, 0.38f }, { kClickFreq, 5000.0f },
            { kDistDrive, 0.07f }, { kDistMix, 0.12f }, { kCompThresh, -13.0f }, { kCompRatio, 3.5f }, { kCompAttack, 7.0f }, { kCompRelease, 100.0f }, { kCompMakeup, 2.5f },
            { kReverbSize, 0.85f }, { kReverbDecay, 850.0f }, { kReverbGate, 240.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedEighth) }, { kSlapbackMix, 0.18f },
            { kReverseMode, presetChoice(ReverseSyncMode::Beat4Auto) }, { kReverseMix, 0.42f }, { kStereoWidth, 1.45f }
        }),
        makePreset("33 Big Room Lift", {
            { kPreBursts, 5.0f }, { kPreSpread, 17.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.22f }, { kPreLevel, 0.94f }, { kFlamSpread, 0.72f },
            { kToneMode, presetChoice(ToneMode::SubThump) }, { kToneQ, 2.2f }, { kToneDecay, 230.0f }, { kToneLevel, 0.90f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseHold, 5.0f }, { kNoiseDecay, 310.0f }, { kNoiseHPF, 250.0f }, { kNoiseLPF, 9000.0f }, { kNoiseStereoWidth, 1.6f }, { kNoiseLevel, 0.95f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.48f }, { kClickFreq, 3500.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.20f }, { kDistTone, -0.08f }, { kDistMix, 0.26f },
            { kCompThresh, -20.0f }, { kCompRatio, 5.8f }, { kCompAttack, 6.0f }, { kCompRelease, 115.0f }, { kCompMakeup, 5.8f },
            { kLimiterCeiling, -0.1f }, { kReverbSize, 0.92f }, { kReverbDecay, 900.0f }, { kReverbGate, 255.0f }, { kReverbMix, 0.46f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedEighth) }, { kSlapbackMix, 0.18f }, { kStereoWidth, 1.42f }, { kOutputGain, 0.6f }
        }),
        makePreset("34 Piano Anthem Gate", {
            { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.11f }, { kPreLevel, 0.82f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 3.2f }, { kToneDecay, 210.0f }, { kToneLevel, 0.60f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseHold, 3.0f }, { kNoiseDecay, 230.0f }, { kNoiseHPF, 330.0f }, { kNoiseLPF, 10500.0f }, { kNoiseStereoWidth, 1.2f }, { kNoiseLevel, 0.81f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.28f }, { kClickFreq, 3000.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.06f }, { kDistTone, 0.02f }, { kDistMix, 0.12f },
            { kCompThresh, -12.0f }, { kCompRatio, 3.4f }, { kCompAttack, 9.0f }, { kCompRelease, 125.0f }, { kCompMakeup, 2.0f },
            { kReverbSize, 0.88f }, { kReverbDecay, 900.0f }, { kReverbDamp, 0.38f }, { kReverbGate, 260.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 1.20f }
        }),
        makePreset("35 Laser Stadium", {
            { kPreBursts, 2.0f }, { kPreSpread, 8.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.82f },
            { kToneMode, presetChoice(ToneMode::CyberSquare) }, { kToneQ, 7.0f }, { kToneDecay, 95.0f }, { kToneLevel, 0.88f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 135.0f }, { kNoiseHPF, 620.0f }, { kNoiseLPF, 16500.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.85f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.72f }, { kClickFreq, 9900.0f },
            { kDistType, presetChoice(DistortionType::Wavefold) }, { kDistDrive, 0.18f }, { kDistTone, 0.28f }, { kDistMix, 0.23f },
            { kCompThresh, -18.0f }, { kCompRatio, 6.5f }, { kCompAttack, 1.2f }, { kCompRelease, 45.0f }, { kCompMakeup, 4.8f },
            { kReverbSize, 0.62f }, { kReverbDecay, 350.0f }, { kReverbGate, 125.0f }, { kReverbMix, 0.24f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Eighth) }, { kSlapbackMix, 0.20f }, { kStereoWidth, 1.24f }
        }),
        makePreset("36 Vintage Euro Lift", {
            { kPreBursts, 3.0f }, { kPreSpread, 15.0f }, { kPreDecay, 11.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.86f },
            { kToneQ, 3.0f }, { kToneDecay, 145.0f }, { kToneLevel, 0.74f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseHold, 3.0f }, { kNoiseDecay, 220.0f }, { kNoiseHPF, 320.0f }, { kNoiseLPF, 10500.0f }, { kNoiseStereoWidth, 1.28f }, { kNoiseLevel, 0.84f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.40f }, { kClickFreq, 4500.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.16f }, { kDistTone, -0.06f }, { kDistMix, 0.22f },
            { kCompThresh, -15.0f }, { kCompRatio, 4.4f }, { kCompAttack, 4.5f }, { kCompRelease, 88.0f }, { kCompMakeup, 3.2f },
            { kReverbSize, 0.70f }, { kReverbDecay, 620.0f }, { kReverbDamp, 0.44f }, { kReverbGate, 185.0f }, { kReverbMix, 0.36f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.17f }, { kStereoWidth, 1.24f }
        }),
        makePreset("37 Minimal Club Tap", {
            { kPreBursts, 1.0f }, { kPreSpread, 5.0f }, { kPreDecay, 4.0f }, { kPreJitter, 0.0f }, { kPreLevel, 0.45f }, { kFlamSpread, 0.0f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 2.5f }, { kToneDecay, 65.0f }, { kToneLevel, 0.48f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 105.0f }, { kNoiseCurve, -0.55f }, { kNoiseHPF, 480.0f }, { kNoiseLPF, 8500.0f }, { kNoiseStereoWidth, 0.55f }, { kNoiseLevel, 0.60f },
            { kClickLevel, 0.35f }, { kClickFreq, 4600.0f }, { kDistDrive, 0.0f }, { kDistMix, 0.0f },
            { kCompThresh, -10.0f }, { kCompRatio, 2.0f }, { kCompAttack, 14.0f }, { kCompRelease, 120.0f }, { kCompMakeup, 1.2f }, { kCompMix, 0.80f },
            { kReverbSize, 0.28f }, { kReverbDecay, 130.0f }, { kReverbGate, 45.0f }, { kReverbMix, 0.10f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Eighth) }, { kSlapbackMix, 0.15f }, { kStereoWidth, 0.72f }, { kOutputGain, -1.5f }
        }),
        makePreset("38 Warm Dub 1/8", {
            { kPreBursts, 2.0f }, { kPreSpread, 12.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.20f }, { kPreLevel, 0.68f },
            { kToneMode, presetChoice(ToneMode::SubThump) }, { kToneQ, 2.1f }, { kToneDecay, 230.0f }, { kToneLevel, 0.67f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 1.4f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 340.0f }, { kNoiseHPF, 220.0f }, { kNoiseLPF, 7800.0f }, { kNoiseStereoWidth, 1.28f }, { kNoiseLevel, 0.76f },
            { kClickType, presetChoice(ClickType::Thump) }, { kClickLevel, 0.22f }, { kClickFreq, 2800.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.10f }, { kDistTone, -0.20f }, { kDistMix, 0.16f },
            { kCompThresh, -11.0f }, { kCompRatio, 2.6f }, { kCompAttack, 13.0f }, { kCompRelease, 165.0f }, { kCompMakeup, 1.8f },
            { kReverbSize, 0.72f }, { kReverbDecay, 680.0f }, { kReverbDamp, 0.62f }, { kReverbGate, 210.0f }, { kReverbMix, 0.32f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Eighth) }, { kSlapbackMix, 0.30f }, { kStereoWidth, 1.15f }, { kOutputGain, -0.8f }
        }),
        makePreset("39 Dry Utility Clap", {
            { kPreBursts, 2.0f }, { kPreSpread, 8.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.04f }, { kPreLevel, 0.75f },
            { kToneQ, 3.2f }, { kToneDecay, 80.0f }, { kToneLevel, 0.70f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 120.0f }, { kNoiseCurve, -0.5f }, { kNoiseHPF, 420.0f }, { kNoiseLPF, 10000.0f }, { kNoiseStereoWidth, 0.85f }, { kNoiseLevel, 0.78f },
            { kClickType, presetChoice(ClickType::Needle) }, { kClickLevel, 0.42f }, { kClickFreq, 5300.0f },
            { kDistType, presetChoice(DistortionType::Tape) }, { kDistDrive, 0.05f }, { kDistMix, 0.10f },
            { kCompThresh, -13.0f }, { kCompRatio, 3.0f }, { kCompAttack, 6.0f }, { kCompRelease, 75.0f }, { kCompMakeup, 2.0f },
            { kReverbSize, 0.25f }, { kReverbDecay, 100.0f }, { kReverbGate, 25.0f }, { kReverbMix, 0.08f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 0.85f }, { kOutputGain, -0.5f }
        }),
        makePreset("40 Wide Utility Clap", {
            { kPreBursts, 3.0f }, { kPreSpread, 11.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.12f }, { kPreLevel, 0.80f }, { kFlamSpread, 0.70f },
            { kToneQ, 3.0f }, { kToneDecay, 105.0f }, { kToneLevel, 0.67f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 185.0f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 1.85f }, { kNoiseLevel, 0.86f },
            { kClickLevel, 0.36f }, { kClickFreq, 4900.0f }, { kDistDrive, 0.04f }, { kDistMix, 0.08f },
            { kCompThresh, -12.0f }, { kCompRatio, 3.0f }, { kCompAttack, 8.0f }, { kCompRelease, 95.0f }, { kCompMakeup, 2.0f },
            { kReverbSize, 0.52f }, { kReverbDecay, 320.0f }, { kReverbGate, 110.0f }, { kReverbMix, 0.18f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedSixteenth) }, { kSlapbackMix, 0.12f }, { kStereoWidth, 1.75f }
        }),
        makePreset("41 Tight Roll Clap", {
            { kPreBursts, 1.0f }, { kPreSpread, 4.0f }, { kPreDecay, 3.0f }, { kPreJitter, 0.0f }, { kPreLevel, 0.62f }, { kFlamSpread, 0.25f },
            { kToneQ, 3.6f }, { kToneDecay, 48.0f }, { kToneLevel, 0.61f },
            { kNoiseType, presetChoice(NoiseType::Velvet) }, { kNoiseDecay, 70.0f }, { kNoiseCurve, -0.6f }, { kNoiseHPF, 520.0f }, { kNoiseLPF, 9000.0f }, { kNoiseStereoWidth, 0.75f }, { kNoiseLevel, 0.70f },
            { kClickLevel, 0.40f }, { kClickFreq, 6000.0f }, { kDistDrive, 0.03f }, { kDistMix, 0.07f },
            { kCompThresh, -15.0f }, { kCompRatio, 4.5f }, { kCompAttack, 0.8f }, { kCompRelease, 28.0f }, { kCompMakeup, 3.0f },
            { kReverbSize, 0.30f }, { kReverbDecay, 120.0f }, { kReverbGate, 40.0f }, { kReverbMix, 0.08f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Sixteenth) }, { kSlapbackMix, 0.15f }, { kStereoWidth, 0.82f }
        }),
        makePreset("42 Quarter Note Bloom", {
            { kPreBursts, 3.0f }, { kPreSpread, 13.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.82f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 3.9f }, { kToneDecay, 180.0f }, { kToneLevel, 0.76f },
            { kNoiseType, presetChoice(NoiseType::Pink) }, { kNoiseAttack, 0.8f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 260.0f }, { kNoiseHPF, 330.0f }, { kNoiseLPF, 13500.0f }, { kNoiseStereoWidth, 1.45f }, { kNoiseLevel, 0.86f },
            { kClickLevel, 0.38f }, { kClickFreq, 5400.0f },
            { kDistType, presetChoice(DistortionType::Tube) }, { kDistDrive, 0.10f }, { kDistTone, 0.08f }, { kDistMix, 0.16f },
            { kCompThresh, -14.0f }, { kCompRatio, 3.5f }, { kCompAttack, 6.0f }, { kCompRelease, 110.0f }, { kCompMakeup, 2.5f },
            { kReverbSize, 0.88f }, { kReverbDecay, 840.0f }, { kReverbGate, 230.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Quarter) }, { kSlapbackMix, 0.24f }, { kStereoWidth, 1.42f }
        }),
        makePreset("43 MIDI Reference Clap", {
            { kPreBursts, 1.0f }, { kPreSpread, 8.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.0f }, { kPreLevel, 0.68f }, { kFlamSpread, 0.0f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 3.2f }, { kToneDecay, 110.0f }, { kToneLevel, 0.66f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseDecay, 130.0f }, { kNoiseCurve, -0.35f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 9500.0f }, { kNoiseStereoWidth, 0.9f }, { kNoiseLevel, 0.70f },
            { kClickLevel, 0.30f }, { kClickFreq, 4500.0f }, { kDistDrive, 0.0f }, { kDistMix, 0.0f },
            { kCompThresh, -10.0f }, { kCompRatio, 2.0f }, { kCompAttack, 12.0f }, { kCompRelease, 120.0f }, { kCompMakeup, 1.0f }, { kCompMix, 0.85f },
            { kReverbSize, 0.48f }, { kReverbDecay, 280.0f }, { kReverbGate, 95.0f }, { kReverbMix, 0.18f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::Off) }, { kSlapbackMix, 0.0f }, { kStereoWidth, 0.90f }, { kOutputGain, -1.0f }
        }),
        makePreset("44 Euphoric Trance Lift", {
            { kPreBursts, 4.0f }, { kPreSpread, 14.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.88f }, { kFlamSpread, 0.64f },
            { kToneMode, presetChoice(ToneMode::Bandpass) }, { kToneQ, 3.7f }, { kToneDecay, 165.0f }, { kToneLevel, 0.79f },
            { kNoiseType, presetChoice(NoiseType::White) }, { kNoiseAttack, 0.3f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 245.0f }, { kNoiseCurve, -0.22f }, { kNoiseHPF, 370.0f }, { kNoiseLPF, 14500.0f }, { kNoiseStereoWidth, 1.48f }, { kNoiseLevel, 0.90f },
            { kClickType, presetChoice(ClickType::Crack) }, { kClickLevel, 0.40f }, { kClickFreq, 5600.0f },
            { kDistDrive, 0.08f }, { kDistTone, 0.04f }, { kDistMix, 0.14f },
            { kCompThresh, -14.0f }, { kCompRatio, 3.8f }, { kCompAttack, 5.0f }, { kCompRelease, 90.0f }, { kCompMakeup, 3.0f },
            { kLimiterCeiling, -0.2f }, { kReverbSize, 0.80f }, { kReverbDecay, 720.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 215.0f }, { kReverbMix, 0.40f },
            { kSlapbackDivision, presetChoice(SlapbackDivision::DottedEighth) }, { kSlapbackMix, 0.19f }, { kStereoWidth, 1.44f }
        })
    };
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
