#include "Presets.h"
#include "Parameters.h"

namespace openclapper::presets {

using namespace openclapper::params;

const std::vector<FactoryPreset>& factoryPresets() {
    static const std::vector<FactoryPreset> kPresets = {
        {
            "01 Init Trance Clap",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.85f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1100.0f }, { kToneQ, 3.2f }, { kToneDecay, 120.0f }, { kTonePitchEnv, 7.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.75f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 180.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 9500.0f }, { kNoiseStereoWidth, 1.0f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.4f }, { kClickFreq, 4500.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.11f }, { kDistTone, 0.0f }, { kDistMix, 0.19f },
                { kCompThresh, -14.0f }, { kCompRatio, 4.0f }, { kCompAttack, 5.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 3.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.65f }, { kReverbDecay, 450.0f }, { kReverbDamp, 0.4f }, { kReverbGate, 160.0f }, { kReverbMix, 0.35f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.0f }, { kOutputGain, 0.0f }
            }
        },
        {
            "02 Uplifting Euphoria",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 14.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.22f }, { kPreLevel, 0.90f },
                { kToneNote, 11.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1350.0f }, { kToneQ, 4.5f }, { kToneDecay, 160.0f }, { kTonePitchEnv, 9.0f }, { kTonePitchDecay, 22.0f }, { kToneLevel, 0.82f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.5f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 240.0f }, { kNoiseCurve, -0.2f }, { kNoiseHPF, 420.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 1.4f }, { kNoiseLevel, 0.92f },
                { kClickLevel, 0.45f }, { kClickFreq, 5200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.14f }, { kDistTone, 0.15f }, { kDistMix, 0.22f },
                { kCompThresh, -16.0f }, { kCompRatio, 5.0f }, { kCompAttack, 4.0f }, { kCompRelease, 90.0f }, { kCompMakeup, 4.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 25.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.85f }, { kReverbDecay, 850.0f }, { kReverbDamp, 0.3f }, { kReverbGate, 220.0f }, { kReverbMix, 0.48f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.25f }, { kOutputGain, 0.5f }
            }
        },
        {
            "03 Psytrance Laser Snap",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 7.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.70f },
                { kToneNote, 4.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1650.0f }, { kToneQ, 7.5f }, { kToneDecay, 75.0f }, { kTonePitchEnv, 18.0f }, { kTonePitchDecay, 12.0f }, { kToneLevel, 0.95f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.0f }, { kNoiseDecay, 90.0f }, { kNoiseCurve, -0.6f }, { kNoiseHPF, 500.0f }, { kNoiseLPF, 14000.0f }, { kNoiseStereoWidth, 0.8f }, { kNoiseLevel, 0.80f },
                { kClickLevel, 0.65f }, { kClickFreq, 7000.0f },
                { kDistType, 2.0f }, { kDistDrive, 0.20f }, { kDistTone, 0.25f }, { kDistMix, 0.25f },
                { kCompThresh, -18.0f }, { kCompRatio, 8.0f }, { kCompAttack, 1.0f }, { kCompRelease, 40.0f }, { kCompMakeup, 5.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 15.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.40f }, { kReverbDecay, 180.0f }, { kReverbDamp, 0.6f }, { kReverbGate, 70.0f }, { kReverbMix, 0.20f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.3f }, { kStereoWidth, 1.0f }, { kOutputGain, 0.0f }
            }
        },
        {
            "04 Big Room Festival Stomp",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 16.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.25f }, { kPreLevel, 0.95f },
                { kToneNote, 7.0f }, { kToneOctave, 2.0f }, { kToneFreq, 780.0f }, { kToneQ, 2.5f }, { kToneDecay, 180.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 25.0f }, { kToneLevel, 0.88f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 5.0f }, { kNoiseDecay, 260.0f }, { kNoiseCurve, -0.1f }, { kNoiseHPF, 280.0f }, { kNoiseLPF, 8500.0f }, { kNoiseStereoWidth, 1.6f }, { kNoiseLevel, 0.95f },
                { kClickLevel, 0.55f }, { kClickFreq, 3800.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.24f }, { kDistTone, -0.05f }, { kDistMix, 0.28f },
                { kCompThresh, -20.0f }, { kCompRatio, 6.0f }, { kCompAttack, 6.0f }, { kCompRelease, 120.0f }, { kCompMakeup, 6.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.1f }, { kLimiterRelease, 30.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.90f }, { kReverbDecay, 950.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 280.0f }, { kReverbMix, 0.50f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.3f }, { kOutputGain, 1.0f }
            }
        },
        {
            "05 138 BPM Peak Time",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 10.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.12f }, { kPreLevel, 0.85f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1250.0f }, { kToneQ, 4.0f }, { kToneDecay, 110.0f }, { kTonePitchEnv, 8.0f }, { kTonePitchDecay, 15.0f }, { kToneLevel, 0.80f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 150.0f }, { kNoiseCurve, -0.4f }, { kNoiseHPF, 400.0f }, { kNoiseLPF, 10500.0f }, { kNoiseStereoWidth, 1.2f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.50f }, { kClickFreq, 5000.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.13f }, { kDistTone, 0.1f }, { kDistMix, 0.19f },
                { kCompThresh, -15.0f }, { kCompRatio, 5.0f }, { kCompAttack, 3.0f }, { kCompRelease, 70.0f }, { kCompMakeup, 3.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.60f }, { kReverbDecay, 380.0f }, { kReverbDamp, 0.45f }, { kReverbGate, 140.0f }, { kReverbMix, 0.32f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.15f }, { kOutputGain, 0.0f }
            }
        },
        {
            "06 Dutch Hardstyle Smacker",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.90f },
                { kToneNote, 0.0f }, { kToneOctave, 3.0f }, { kToneFreq, 950.0f }, { kToneQ, 5.5f }, { kToneDecay, 95.0f }, { kTonePitchEnv, 14.0f }, { kTonePitchDecay, 14.0f }, { kToneLevel, 0.95f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 120.0f }, { kNoiseCurve, -0.5f }, { kNoiseHPF, 320.0f }, { kNoiseLPF, 11000.0f }, { kNoiseStereoWidth, 0.9f }, { kNoiseLevel, 0.95f },
                { kClickLevel, 0.70f }, { kClickFreq, 6200.0f },
                { kDistType, 2.0f }, { kDistDrive, 0.33f }, { kDistTone, 0.20f }, { kDistMix, 0.36f },
                { kCompThresh, -22.0f }, { kCompRatio, 12.0f }, { kCompAttack, 0.8f }, { kCompRelease, 50.0f }, { kCompMakeup, 7.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.1f }, { kLimiterRelease, 15.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.50f }, { kReverbDecay, 250.0f }, { kReverbDamp, 0.5f }, { kReverbGate, 90.0f }, { kReverbMix, 0.25f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.05f }, { kOutputGain, 0.5f }
            }
        },
        {
            "07 Progressive Velvet Air",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 14.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.20f }, { kPreLevel, 0.75f },
                { kToneNote, 7.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1050.0f }, { kToneQ, 2.8f }, { kToneDecay, 140.0f }, { kTonePitchEnv, 4.0f }, { kTonePitchDecay, 20.0f }, { kToneLevel, 0.65f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 2.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 220.0f }, { kNoiseCurve, -0.2f }, { kNoiseHPF, 300.0f }, { kNoiseLPF, 8000.0f }, { kNoiseStereoWidth, 1.3f }, { kNoiseLevel, 0.80f },
                { kClickLevel, 0.25f }, { kClickFreq, 3500.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.08f }, { kDistTone, -0.15f }, { kDistMix, 0.14f },
                { kCompThresh, -12.0f }, { kCompRatio, 3.0f }, { kCompAttack, 10.0f }, { kCompRelease, 100.0f }, { kCompMakeup, 2.0f }, { kCompMix, 0.85f },
                { kLimiterCeiling, -0.4f }, { kLimiterRelease, 25.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.70f }, { kReverbDecay, 550.0f }, { kReverbDamp, 0.5f }, { kReverbGate, 180.0f }, { kReverbMix, 0.40f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.2f }, { kOutputGain, -0.5f }
            }
        },
        {
            "08 Future Rave Cyber Slap",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 11.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.88f },
                { kToneNote, 2.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1400.0f }, { kToneQ, 4.8f }, { kToneDecay, 130.0f }, { kTonePitchEnv, 11.0f }, { kTonePitchDecay, 16.0f }, { kToneLevel, 0.85f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 160.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 450.0f }, { kNoiseLPF, 11500.0f }, { kNoiseStereoWidth, 1.5f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.60f }, { kClickFreq, 5800.0f },
                { kDistType, 3.0f }, { kDistDrive, 0.22f }, { kDistTone, 0.20f }, { kDistMix, 0.25f },
                { kCompThresh, -17.0f }, { kCompRatio, 6.0f }, { kCompAttack, 2.5f }, { kCompRelease, 65.0f }, { kCompMakeup, 4.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.75f }, { kReverbDecay, 500.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 150.0f }, { kReverbMix, 0.38f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.25f }, { kOutputGain, 0.0f }
            }
        },
        {
            "09 90s Eurodance Gated Classic",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 15.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.80f },
                { kToneNote, 9.0f }, { kToneOctave, 2.0f }, { kToneFreq, 850.0f }, { kToneQ, 3.0f }, { kToneDecay, 150.0f }, { kTonePitchEnv, 5.0f }, { kTonePitchDecay, 25.0f }, { kToneLevel, 0.80f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 200.0f }, { kNoiseCurve, -0.2f }, { kNoiseHPF, 300.0f }, { kNoiseLPF, 9000.0f }, { kNoiseStereoWidth, 1.1f }, { kNoiseLevel, 0.88f },
                { kClickLevel, 0.40f }, { kClickFreq, 4200.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.13f }, { kDistTone, 0.0f }, { kDistMix, 0.17f },
                { kCompThresh, -14.0f }, { kCompRatio, 4.5f }, { kCompAttack, 4.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 3.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 22.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.80f }, { kReverbDecay, 700.0f }, { kReverbDamp, 0.25f }, { kReverbGate, 200.0f }, { kReverbMix, 0.55f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.1f }, { kOutputGain, 0.0f }
            }
        },
        {
            "10 Vocal Layering Clap",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 16.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.30f }, { kPreLevel, 0.85f },
                { kToneNote, 5.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1800.0f }, { kToneQ, 3.5f }, { kToneDecay, 100.0f }, { kTonePitchEnv, 3.0f }, { kTonePitchDecay, 15.0f }, { kToneLevel, 0.60f },
                { kNoiseType, 1.0f }, { kNoiseAttack, 1.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 210.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 600.0f }, { kNoiseLPF, 13000.0f }, { kNoiseStereoWidth, 1.8f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.30f }, { kClickFreq, 5500.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.07f }, { kDistTone, 0.10f }, { kDistMix, 0.11f },
                { kCompThresh, -12.0f }, { kCompRatio, 3.5f }, { kCompAttack, 8.0f }, { kCompRelease, 90.0f }, { kCompMakeup, 2.5f }, { kCompMix, 0.9f },
                { kLimiterCeiling, -0.4f }, { kLimiterRelease, 25.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.75f }, { kReverbDecay, 600.0f }, { kReverbDamp, 0.4f }, { kReverbGate, 180.0f }, { kReverbMix, 0.42f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.45f }, { kOutputGain, -0.5f }
            }
        },
        {
            "11 Electro Sizzle Punch",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.90f },
                { kToneNote, 9.0f }, { kToneOctave, 2.0f }, { kToneFreq, 920.0f }, { kToneQ, 4.2f }, { kToneDecay, 110.0f }, { kTonePitchEnv, 12.0f }, { kTonePitchDecay, 15.0f }, { kToneLevel, 0.90f },
                { kNoiseType, 3.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 140.0f }, { kNoiseCurve, -0.4f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 10000.0f }, { kNoiseStereoWidth, 1.1f }, { kNoiseLevel, 0.92f },
                { kClickLevel, 0.65f }, { kClickFreq, 6000.0f },
                { kDistType, 2.0f }, { kDistDrive, 0.26f }, { kDistTone, 0.15f }, { kDistMix, 0.28f },
                { kCompThresh, -18.0f }, { kCompRatio, 7.0f }, { kCompAttack, 2.0f }, { kCompRelease, 60.0f }, { kCompMakeup, 5.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 18.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.55f }, { kReverbDecay, 320.0f }, { kReverbDamp, 0.5f }, { kReverbGate, 110.0f }, { kReverbMix, 0.28f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.1f }, { kOutputGain, 0.2f }
            }
        },
        {
            "12 Tech Trance Acid Bite",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 8.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.75f },
                { kToneNote, 0.0f }, { kToneOctave, 4.0f }, { kToneFreq, 1950.0f }, { kToneQ, 8.5f }, { kToneDecay, 85.0f }, { kTonePitchEnv, 15.0f }, { kTonePitchDecay, 12.0f }, { kToneLevel, 0.92f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.0f }, { kNoiseDecay, 110.0f }, { kNoiseCurve, -0.5f }, { kNoiseHPF, 480.0f }, { kNoiseLPF, 12500.0f }, { kNoiseStereoWidth, 1.0f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.70f }, { kClickFreq, 7200.0f },
                { kDistType, 3.0f }, { kDistDrive, 0.24f }, { kDistTone, 0.25f }, { kDistMix, 0.26f },
                { kCompThresh, -16.0f }, { kCompRatio, 7.0f }, { kCompAttack, 1.5f }, { kCompRelease, 45.0f }, { kCompMakeup, 4.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 16.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.45f }, { kReverbDecay, 220.0f }, { kReverbDamp, 0.55f }, { kReverbGate, 80.0f }, { kReverbMix, 0.22f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.3f }, { kStereoWidth, 1.05f }, { kOutputGain, 0.0f }
            }
        },
        {
            "13 Hands Up German Party",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 13.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.92f },
                { kToneNote, 11.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1450.0f }, { kToneQ, 4.0f }, { kToneDecay, 140.0f }, { kTonePitchEnv, 8.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.85f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 190.0f }, { kNoiseCurve, -0.25f }, { kNoiseHPF, 380.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.92f },
                { kClickLevel, 0.55f }, { kClickFreq, 5500.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.15f }, { kDistTone, 0.10f }, { kDistMix, 0.21f },
                { kCompThresh, -15.0f }, { kCompRatio, 5.0f }, { kCompAttack, 3.5f }, { kCompRelease, 75.0f }, { kCompMakeup, 4.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 22.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.70f }, { kReverbDecay, 500.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 170.0f }, { kReverbMix, 0.40f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.45f }, { kStereoWidth, 1.2f }, { kOutputGain, 0.2f }
            }
        },
        {
            "14 Stadium Crowd Swarm",
            {
                { kPreBursts, 5.0f }, { kPreSpread, 22.0f }, { kPreDecay, 15.0f }, { kPreJitter, 0.45f }, { kPreLevel, 0.98f },
                { kToneNote, 7.0f }, { kToneOctave, 2.0f }, { kToneFreq, 900.0f }, { kToneQ, 2.0f }, { kToneDecay, 200.0f }, { kTonePitchEnv, 4.0f }, { kTonePitchDecay, 30.0f }, { kToneLevel, 0.70f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 1.5f }, { kNoiseHold, 8.0f }, { kNoiseDecay, 320.0f }, { kNoiseCurve, 0.0f }, { kNoiseHPF, 250.0f }, { kNoiseLPF, 8500.0f }, { kNoiseStereoWidth, 1.9f }, { kNoiseLevel, 0.95f },
                { kClickLevel, 0.35f }, { kClickFreq, 3200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.10f }, { kDistTone, -0.10f }, { kDistMix, 0.17f },
                { kCompThresh, -14.0f }, { kCompRatio, 4.0f }, { kCompAttack, 12.0f }, { kCompRelease, 140.0f }, { kCompMakeup, 3.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 35.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.95f }, { kReverbDecay, 1200.0f }, { kReverbDamp, 0.4f }, { kReverbGate, 320.0f }, { kReverbMix, 0.55f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.5f }, { kOutputGain, 0.5f }
            }
        },
        {
            "15 Cyberpunk Neon Slam",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.90f },
                { kToneNote, 3.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1200.0f }, { kToneQ, 5.0f }, { kToneDecay, 140.0f }, { kTonePitchEnv, 14.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.90f },
                { kNoiseType, 3.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 190.0f }, { kNoiseCurve, -0.35f }, { kNoiseHPF, 340.0f }, { kNoiseLPF, 11000.0f }, { kNoiseStereoWidth, 1.6f }, { kNoiseLevel, 0.92f },
                { kClickLevel, 0.65f }, { kClickFreq, 5200.0f },
                { kDistType, 3.0f }, { kDistDrive, 0.29f }, { kDistTone, 0.15f }, { kDistMix, 0.30f },
                { kCompThresh, -19.0f }, { kCompRatio, 8.0f }, { kCompAttack, 2.0f }, { kCompRelease, 70.0f }, { kCompMakeup, 5.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.1f }, { kLimiterRelease, 22.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.80f }, { kReverbDecay, 650.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 190.0f }, { kReverbMix, 0.42f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.45f }, { kStereoWidth, 1.35f }, { kOutputGain, 0.3f }
            }
        },
        {
            "16 Melodic Progressive Dream",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 13.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.20f }, { kPreLevel, 0.82f },
                { kToneNote, 7.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1150.0f }, { kToneQ, 3.0f }, { kToneDecay, 150.0f }, { kTonePitchEnv, 5.0f }, { kTonePitchDecay, 20.0f }, { kToneLevel, 0.72f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.5f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 220.0f }, { kNoiseCurve, -0.2f }, { kNoiseHPF, 320.0f }, { kNoiseLPF, 9000.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.35f }, { kClickFreq, 4200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.09f }, { kDistTone, -0.05f }, { kDistMix, 0.15f },
                { kCompThresh, -13.0f }, { kCompRatio, 3.8f }, { kCompAttack, 6.0f }, { kCompRelease, 90.0f }, { kCompMakeup, 2.5f }, { kCompMix, 0.95f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 25.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.80f }, { kReverbDecay, 750.0f }, { kReverbDamp, 0.4f }, { kReverbGate, 210.0f }, { kReverbMix, 0.44f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.25f }, { kOutputGain, 0.0f }
            }
        },
        {
            "17 Melbourne Bounce Spank",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 10.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.88f },
                { kToneNote, 5.0f }, { kToneOctave, 2.0f }, { kToneFreq, 820.0f }, { kToneQ, 4.5f }, { kToneDecay, 120.0f }, { kTonePitchEnv, 16.0f }, { kTonePitchDecay, 14.0f }, { kToneLevel, 0.92f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 140.0f }, { kNoiseCurve, -0.45f }, { kNoiseHPF, 300.0f }, { kNoiseLPF, 10500.0f }, { kNoiseStereoWidth, 1.0f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.60f }, { kClickFreq, 5200.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.21f }, { kDistTone, 0.10f }, { kDistMix, 0.25f },
                { kCompThresh, -17.0f }, { kCompRatio, 6.0f }, { kCompAttack, 2.0f }, { kCompRelease, 55.0f }, { kCompMakeup, 4.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 18.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.50f }, { kReverbDecay, 280.0f }, { kReverbDamp, 0.45f }, { kReverbGate, 100.0f }, { kReverbMix, 0.25f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.1f }, { kOutputGain, 0.2f }
            }
        },
        {
            "18 Vocal Chop Pre-Swell",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.85f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1200.0f }, { kToneQ, 3.5f }, { kToneDecay, 130.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.78f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 180.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 10000.0f }, { kNoiseStereoWidth, 1.25f }, { kNoiseLevel, 0.88f },
                { kClickLevel, 0.45f }, { kClickFreq, 4800.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.12f }, { kDistTone, 0.05f }, { kDistMix, 0.19f },
                { kCompThresh, -14.0f }, { kCompRatio, 4.5f }, { kCompAttack, 4.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 3.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 22.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.70f }, { kReverbDecay, 500.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 160.0f }, { kReverbMix, 0.38f },
                { kReverseMode, 1.0f }, { kReverseMix, 0.65f }, { kStereoWidth, 1.2f }, { kOutputGain, 0.0f }
            }
        },
        {
            "19 Raw Warehouse Techno",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 11.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.80f },
                { kToneNote, 7.0f }, { kToneOctave, 2.0f }, { kToneFreq, 750.0f }, { kToneQ, 3.0f }, { kToneDecay, 100.0f }, { kTonePitchEnv, 8.0f }, { kTonePitchDecay, 15.0f }, { kToneLevel, 0.85f },
                { kNoiseType, 3.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.0f }, { kNoiseDecay, 130.0f }, { kNoiseCurve, -0.5f }, { kNoiseHPF, 220.0f }, { kNoiseLPF, 7000.0f }, { kNoiseStereoWidth, 0.85f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.40f }, { kClickFreq, 3200.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.24f }, { kDistTone, -0.25f }, { kDistMix, 0.28f },
                { kCompThresh, -16.0f }, { kCompRatio, 6.0f }, { kCompAttack, 3.0f }, { kCompRelease, 60.0f }, { kCompMakeup, 4.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.60f }, { kReverbDecay, 350.0f }, { kReverbDamp, 0.65f }, { kReverbGate, 120.0f }, { kReverbMix, 0.28f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.35f }, { kStereoWidth, 0.95f }, { kOutputGain, 0.0f }
            }
        },
        {
            "20 Trance Energy 2000",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 13.0f }, { kPreDecay, 9.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.88f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1200.0f }, { kToneQ, 3.5f }, { kToneDecay, 140.0f }, { kTonePitchEnv, 7.0f }, { kTonePitchDecay, 20.0f }, { kToneLevel, 0.80f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 200.0f }, { kNoiseCurve, -0.25f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 11000.0f }, { kNoiseStereoWidth, 1.3f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.45f }, { kClickFreq, 4800.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.12f }, { kDistTone, 0.05f }, { kDistMix, 0.19f },
                { kCompThresh, -15.0f }, { kCompRatio, 4.5f }, { kCompAttack, 4.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 3.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 22.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.85f }, { kReverbDecay, 800.0f }, { kReverbDamp, 0.3f }, { kReverbGate, 220.0f }, { kReverbMix, 0.50f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.2f }, { kOutputGain, 0.2f }
            }
        },
        {
            "21 Dual Flam Stereo Split",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 18.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.25f }, { kPreLevel, 0.95f },
                { kToneNote, 11.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1300.0f }, { kToneQ, 3.2f }, { kToneDecay, 130.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.75f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 170.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 400.0f }, { kNoiseLPF, 10000.0f }, { kNoiseStereoWidth, 1.9f }, { kNoiseLevel, 0.88f },
                { kClickLevel, 0.40f }, { kClickFreq, 4600.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.11f }, { kDistTone, 0.0f }, { kDistMix, 0.18f },
                { kCompThresh, -14.0f }, { kCompRatio, 4.0f }, { kCompAttack, 5.0f }, { kCompRelease, 75.0f }, { kCompMakeup, 3.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.65f }, { kReverbDecay, 450.0f }, { kReverbDamp, 0.4f }, { kReverbGate, 160.0f }, { kReverbMix, 0.35f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.45f }, { kOutputGain, 0.0f }
            }
        },
        {
            "22 Hardcore Punch In Face",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 8.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.95f },
                { kToneNote, 0.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1050.0f }, { kToneQ, 6.0f }, { kToneDecay, 90.0f }, { kTonePitchEnv, 16.0f }, { kTonePitchDecay, 12.0f }, { kToneLevel, 0.98f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 110.0f }, { kNoiseCurve, -0.5f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 0.9f }, { kNoiseLevel, 0.95f },
                { kClickLevel, 0.85f }, { kClickFreq, 7500.0f },
                { kDistType, 2.0f }, { kDistDrive, 0.37f }, { kDistTone, 0.30f }, { kDistMix, 0.41f },
                { kCompThresh, -24.0f }, { kCompRatio, 16.0f }, { kCompAttack, 0.5f }, { kCompRelease, 40.0f }, { kCompMakeup, 8.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, 0.0f }, { kLimiterRelease, 12.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.40f }, { kReverbDecay, 200.0f }, { kReverbDamp, 0.6f }, { kReverbGate, 70.0f }, { kReverbMix, 0.18f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.3f }, { kStereoWidth, 1.0f }, { kOutputGain, 0.8f }
            }
        },
        {
            "23 Sunset Balearic Chill",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 16.0f }, { kPreDecay, 12.0f }, { kPreJitter, 0.30f }, { kPreLevel, 0.75f },
                { kToneNote, 9.0f }, { kToneOctave, 2.0f }, { kToneFreq, 950.0f }, { kToneQ, 2.2f }, { kToneDecay, 160.0f }, { kTonePitchEnv, 3.0f }, { kTonePitchDecay, 25.0f }, { kToneLevel, 0.60f },
                { kNoiseType, 1.0f }, { kNoiseAttack, 2.0f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 240.0f }, { kNoiseCurve, -0.15f }, { kNoiseHPF, 280.0f }, { kNoiseLPF, 7500.0f }, { kNoiseStereoWidth, 1.4f }, { kNoiseLevel, 0.78f },
                { kClickLevel, 0.20f }, { kClickFreq, 3000.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.05f }, { kDistTone, -0.20f }, { kDistMix, 0.11f },
                { kCompThresh, -10.0f }, { kCompRatio, 2.8f }, { kCompAttack, 12.0f }, { kCompRelease, 120.0f }, { kCompMakeup, 1.5f }, { kCompMix, 0.8f },
                { kLimiterCeiling, -0.5f }, { kLimiterRelease, 30.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.85f }, { kReverbDecay, 800.0f }, { kReverbDamp, 0.55f }, { kReverbGate, 240.0f }, { kReverbMix, 0.45f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.3f }, { kOutputGain, -1.0f }
            }
        },
        {
            "24 Goa Psy Resonance Ring",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.80f },
                { kToneNote, 2.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1550.0f }, { kToneQ, 9.5f }, { kToneDecay, 140.0f }, { kTonePitchEnv, 14.0f }, { kTonePitchDecay, 16.0f }, { kToneLevel, 0.95f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 130.0f }, { kNoiseCurve, -0.4f }, { kNoiseHPF, 420.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 1.2f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.55f }, { kClickFreq, 6200.0f },
                { kDistType, 3.0f }, { kDistDrive, 0.20f }, { kDistTone, 0.15f }, { kDistMix, 0.23f },
                { kCompThresh, -15.0f }, { kCompRatio, 5.5f }, { kCompAttack, 2.5f }, { kCompRelease, 60.0f }, { kCompMakeup, 4.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 18.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.65f }, { kReverbDecay, 400.0f }, { kReverbDamp, 0.45f }, { kReverbGate, 130.0f }, { kReverbMix, 0.30f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.15f }, { kOutputGain, 0.0f }
            }
        },
        {
            "25 Mainstage Drop Impact",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 15.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.20f }, { kPreLevel, 0.95f },
                { kToneNote, 9.0f }, { kToneOctave, 2.0f }, { kToneFreq, 980.0f }, { kToneQ, 3.5f }, { kToneDecay, 170.0f }, { kTonePitchEnv, 8.0f }, { kTonePitchDecay, 22.0f }, { kToneLevel, 0.90f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 4.0f }, { kNoiseDecay, 240.0f }, { kNoiseCurve, -0.2f }, { kNoiseHPF, 320.0f }, { kNoiseLPF, 11000.0f }, { kNoiseStereoWidth, 1.6f }, { kNoiseLevel, 0.95f },
                { kClickLevel, 0.65f }, { kClickFreq, 5200.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.22f }, { kDistTone, 0.10f }, { kDistMix, 0.25f },
                { kCompThresh, -18.0f }, { kCompRatio, 6.5f }, { kCompAttack, 3.5f }, { kCompRelease, 85.0f }, { kCompMakeup, 5.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.1f }, { kLimiterRelease, 25.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.85f }, { kReverbDecay, 850.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 230.0f }, { kReverbMix, 0.48f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.35f }, { kOutputGain, 0.5f }
            }
        },
        {
            "26 Minimal Click & Flam",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 12.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.70f },
                { kToneNote, 4.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1400.0f }, { kToneQ, 2.5f }, { kToneDecay, 50.0f }, { kTonePitchEnv, 2.0f }, { kTonePitchDecay, 10.0f }, { kToneLevel, 0.50f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.0f }, { kNoiseDecay, 60.0f }, { kNoiseCurve, -0.7f }, { kNoiseHPF, 600.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 0.9f }, { kNoiseLevel, 0.75f },
                { kClickLevel, 0.60f }, { kClickFreq, 6500.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.04f }, { kDistTone, 0.0f }, { kDistMix, 0.08f },
                { kCompThresh, -10.0f }, { kCompRatio, 3.0f }, { kCompAttack, 4.0f }, { kCompRelease, 40.0f }, { kCompMakeup, 1.5f }, { kCompMix, 0.9f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 15.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.20f }, { kReverbDecay, 80.0f }, { kReverbDamp, 0.7f }, { kReverbGate, 30.0f }, { kReverbMix, 0.10f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.2f }, { kStereoWidth, 1.0f }, { kOutputGain, -0.5f }
            }
        },
        {
            "27 Dirty Dutch Pitch Drop",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 10.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.88f },
                { kToneNote, 7.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1300.0f }, { kToneQ, 6.0f }, { kToneDecay, 130.0f }, { kTonePitchEnv, 22.0f }, { kTonePitchDecay, 25.0f }, { kToneLevel, 0.95f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 150.0f }, { kNoiseCurve, -0.35f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 10500.0f }, { kNoiseStereoWidth, 1.15f }, { kNoiseLevel, 0.88f },
                { kClickLevel, 0.65f }, { kClickFreq, 5500.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.24f }, { kDistTone, 0.15f }, { kDistMix, 0.26f },
                { kCompThresh, -17.0f }, { kCompRatio, 6.0f }, { kCompAttack, 2.5f }, { kCompRelease, 65.0f }, { kCompMakeup, 4.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.55f }, { kReverbDecay, 350.0f }, { kReverbDamp, 0.45f }, { kReverbGate, 120.0f }, { kReverbMix, 0.30f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.15f }, { kOutputGain, 0.0f }
            }
        },
        {
            "28 Gated Snare-Clap Hybrid",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.12f }, { kPreLevel, 0.85f },
                { kToneNote, 2.0f }, { kToneOctave, 1.0f }, { kToneFreq, 240.0f }, { kToneQ, 2.5f }, { kToneDecay, 180.0f }, { kTonePitchEnv, 12.0f }, { kTonePitchDecay, 25.0f }, { kToneLevel, 0.90f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 210.0f }, { kNoiseCurve, -0.25f }, { kNoiseHPF, 180.0f }, { kNoiseLPF, 9500.0f }, { kNoiseStereoWidth, 1.3f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.50f }, { kClickFreq, 4200.0f },
                { kDistType, 1.0f }, { kDistDrive, 0.18f }, { kDistTone, 0.0f }, { kDistMix, 0.22f },
                { kCompThresh, -16.0f }, { kCompRatio, 5.0f }, { kCompAttack, 4.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 4.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 22.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.75f }, { kReverbDecay, 600.0f }, { kReverbDamp, 0.4f }, { kReverbGate, 190.0f }, { kReverbMix, 0.42f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.45f }, { kStereoWidth, 1.2f }, { kOutputGain, 0.2f }
            }
        },
        {
            "29 Euphoric Whiteout Noise",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 16.0f }, { kPreDecay, 11.0f }, { kPreJitter, 0.25f }, { kPreLevel, 0.95f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1400.0f }, { kToneQ, 3.5f }, { kToneDecay, 160.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 22.0f }, { kToneLevel, 0.70f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 5.0f }, { kNoiseDecay, 320.0f }, { kNoiseCurve, -0.15f }, { kNoiseHPF, 450.0f }, { kNoiseLPF, 15000.0f }, { kNoiseStereoWidth, 1.8f }, { kNoiseLevel, 0.98f },
                { kClickLevel, 0.45f }, { kClickFreq, 5500.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.15f }, { kDistTone, 0.20f }, { kDistMix, 0.21f },
                { kCompThresh, -17.0f }, { kCompRatio, 5.5f }, { kCompAttack, 4.0f }, { kCompRelease, 100.0f }, { kCompMakeup, 4.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 28.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.90f }, { kReverbDecay, 950.0f }, { kReverbDamp, 0.3f }, { kReverbGate, 260.0f }, { kReverbMix, 0.52f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.4f }, { kOutputGain, 0.3f }
            }
        },
        {
            "30 Cyber Clapper Signature",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.88f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1150.0f }, { kToneQ, 4.0f }, { kToneDecay, 135.0f }, { kTonePitchEnv, 9.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.85f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 180.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 10500.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.55f }, { kClickFreq, 5000.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.17f }, { kDistTone, 0.10f }, { kDistMix, 0.22f },
                { kCompThresh, -15.0f }, { kCompRatio, 5.0f }, { kCompAttack, 3.5f }, { kCompRelease, 75.0f }, { kCompMakeup, 4.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.75f }, { kReverbDecay, 600.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 180.0f }, { kReverbMix, 0.40f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.45f }, { kStereoWidth, 1.25f }, { kOutputGain, 0.2f }
            }
        },
        {
            "31 Fast Roll 16th Anti-Mud",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 7.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.05f }, { kPreLevel, 0.75f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1200.0f }, { kToneQ, 3.5f }, { kToneDecay, 70.0f }, { kTonePitchEnv, 4.0f }, { kTonePitchDecay, 12.0f }, { kToneLevel, 0.75f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.0f }, { kNoiseDecay, 85.0f }, { kNoiseCurve, -0.6f }, { kNoiseHPF, 450.0f }, { kNoiseLPF, 11000.0f }, { kNoiseStereoWidth, 0.95f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.45f }, { kClickFreq, 5200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.09f }, { kDistTone, 0.05f }, { kDistMix, 0.14f },
                { kCompThresh, -13.0f }, { kCompRatio, 4.0f }, { kCompAttack, 2.5f }, { kCompRelease, 45.0f }, { kCompMakeup, 2.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 15.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.35f }, { kReverbDecay, 150.0f }, { kReverbDamp, 0.55f }, { kReverbGate, 65.0f }, { kReverbMix, 0.20f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.3f }, { kStereoWidth, 1.05f }, { kOutputGain, 0.0f }
            }
        },
        {
            "32 Beat 4 Reverse Pickup",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.15f }, { kPreLevel, 0.88f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1150.0f }, { kToneQ, 3.8f }, { kToneDecay, 130.0f }, { kTonePitchEnv, 7.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.80f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 180.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 360.0f }, { kNoiseLPF, 10000.0f }, { kNoiseStereoWidth, 1.3f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.50f }, { kClickFreq, 4800.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.13f }, { kDistTone, 0.05f }, { kDistMix, 0.19f },
                { kCompThresh, -15.0f }, { kCompRatio, 4.5f }, { kCompAttack, 3.5f }, { kCompRelease, 75.0f }, { kCompMakeup, 3.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.70f }, { kReverbDecay, 550.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 180.0f }, { kReverbMix, 0.40f },
                { kReverseMode, 2.0f }, { kReverseMix, 0.75f }, { kStereoWidth, 1.25f }, { kOutputGain, 0.2f }
            }
        },
        {
            "33 Clean Club Mainstage",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 11.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.12f }, { kPreLevel, 0.82f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1100.0f }, { kToneQ, 3.0f }, { kToneDecay, 110.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 16.0f }, { kToneLevel, 0.70f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 175.0f }, { kNoiseCurve, -0.25f }, { kNoiseHPF, 380.0f }, { kNoiseLPF, 12500.0f }, { kNoiseStereoWidth, 1.2f }, { kNoiseLevel, 0.88f },
                { kClickLevel, 0.40f }, { kClickFreq, 5000.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -12.0f }, { kCompRatio, 3.5f }, { kCompAttack, 6.0f }, { kCompRelease, 70.0f }, { kCompMakeup, 2.0f }, { kCompMix, 0.9f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.60f }, { kReverbDecay, 400.0f }, { kReverbDamp, 0.35f }, { kReverbGate, 140.0f }, { kReverbMix, 0.30f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.15f }, { kOutputGain, 0.0f }
            }
        },
        {
            "34 Clean Future House Snap",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.80f },
                { kToneNote, 8.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1050.0f }, { kToneQ, 3.6f }, { kToneDecay, 95.0f }, { kTonePitchEnv, 8.0f }, { kTonePitchDecay, 14.0f }, { kToneLevel, 0.78f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.0f }, { kNoiseDecay, 120.0f }, { kNoiseCurve, -0.4f }, { kNoiseHPF, 450.0f }, { kNoiseLPF, 13000.0f }, { kNoiseStereoWidth, 1.1f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.50f }, { kClickFreq, 6200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -14.0f }, { kCompRatio, 4.0f }, { kCompAttack, 4.0f }, { kCompRelease, 60.0f }, { kCompMakeup, 2.5f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 15.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.45f }, { kReverbDecay, 250.0f }, { kReverbDamp, 0.45f }, { kReverbGate, 100.0f }, { kReverbMix, 0.22f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.3f }, { kStereoWidth, 1.1f }, { kOutputGain, 0.0f }
            }
        },
        {
            "35 Clean Melodic Progressive",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 15.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.18f }, { kPreLevel, 0.85f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1250.0f }, { kToneQ, 3.2f }, { kToneDecay, 150.0f }, { kTonePitchEnv, 5.0f }, { kTonePitchDecay, 20.0f }, { kToneLevel, 0.72f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.8f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 260.0f }, { kNoiseCurve, -0.15f }, { kNoiseHPF, 340.0f }, { kNoiseLPF, 14000.0f }, { kNoiseStereoWidth, 1.45f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.35f }, { kClickFreq, 4800.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -16.0f }, { kCompRatio, 3.2f }, { kCompAttack, 8.0f }, { kCompRelease, 100.0f }, { kCompMakeup, 3.0f }, { kCompMix, 0.85f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 25.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.75f }, { kReverbDecay, 650.0f }, { kReverbDamp, 0.30f }, { kReverbGate, 200.0f }, { kReverbMix, 0.38f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.35f }, { kOutputGain, 0.0f }
            }
        },
        {
            "36 Clean Deep House Velvet",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 12.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.14f }, { kPreLevel, 0.78f },
                { kToneNote, 5.0f }, { kToneOctave, 3.0f }, { kToneFreq, 980.0f }, { kToneQ, 2.8f }, { kToneDecay, 130.0f }, { kTonePitchEnv, 4.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.75f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.2f }, { kNoiseHold, 2.0f }, { kNoiseDecay, 160.0f }, { kNoiseCurve, -0.35f }, { kNoiseHPF, 320.0f }, { kNoiseLPF, 9000.0f }, { kNoiseStereoWidth, 1.15f }, { kNoiseLevel, 0.82f },
                { kClickLevel, 0.30f }, { kClickFreq, 4200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -15.0f }, { kCompRatio, 3.0f }, { kCompAttack, 10.0f }, { kCompRelease, 90.0f }, { kCompMakeup, 2.8f }, { kCompMix, 0.9f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 25.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.55f }, { kReverbDecay, 380.0f }, { kReverbDamp, 0.50f }, { kReverbGate, 130.0f }, { kReverbMix, 0.26f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.3f }, { kStereoWidth, 1.15f }, { kOutputGain, 0.0f }
            }
        },
        {
            "37 Clean Disco Nu-Funk Slap",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 8.0f }, { kPreDecay, 5.0f }, { kPreJitter, 0.06f }, { kPreLevel, 0.82f },
                { kToneNote, 0.0f }, { kToneOctave, 4.0f }, { kToneFreq, 1320.0f }, { kToneQ, 3.4f }, { kToneDecay, 100.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 15.0f }, { kToneLevel, 0.80f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.5f }, { kNoiseDecay, 140.0f }, { kNoiseCurve, -0.4f }, { kNoiseHPF, 400.0f }, { kNoiseLPF, 11500.0f }, { kNoiseStereoWidth, 1.25f }, { kNoiseLevel, 0.86f },
                { kClickLevel, 0.45f }, { kClickFreq, 5800.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -13.0f }, { kCompRatio, 4.2f }, { kCompAttack, 4.0f }, { kCompRelease, 65.0f }, { kCompMakeup, 2.6f }, { kCompMix, 0.95f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 18.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.50f }, { kReverbDecay, 280.0f }, { kReverbDamp, 0.40f }, { kReverbGate, 110.0f }, { kReverbMix, 0.25f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.35f }, { kStereoWidth, 1.2f }, { kOutputGain, 0.0f }
            }
        },
        {
            "38 Clean Tropical Pop Slap",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 10.0f }, { kPreDecay, 7.0f }, { kPreJitter, 0.10f }, { kPreLevel, 0.76f },
                { kToneNote, 7.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1150.0f }, { kToneQ, 2.9f }, { kToneDecay, 110.0f }, { kTonePitchEnv, 5.0f }, { kTonePitchDecay, 16.0f }, { kToneLevel, 0.70f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.5f }, { kNoiseDecay, 140.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 360.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 1.3f }, { kNoiseLevel, 0.84f },
                { kClickLevel, 0.38f }, { kClickFreq, 5200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -12.0f }, { kCompRatio, 3.2f }, { kCompAttack, 5.0f }, { kCompRelease, 70.0f }, { kCompMakeup, 2.2f }, { kCompMix, 0.9f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.52f }, { kReverbDecay, 320.0f }, { kReverbDamp, 0.42f }, { kReverbGate, 120.0f }, { kReverbMix, 0.28f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.3f }, { kStereoWidth, 1.22f }, { kOutputGain, 0.0f }
            }
        },
        {
            "39 Clean Tech House Tight",
            {
                { kPreBursts, 2.0f }, { kPreSpread, 6.0f }, { kPreDecay, 4.0f }, { kPreJitter, 0.04f }, { kPreLevel, 0.80f },
                { kToneNote, 2.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1200.0f }, { kToneQ, 4.0f }, { kToneDecay, 70.0f }, { kTonePitchEnv, 9.0f }, { kTonePitchDecay, 12.0f }, { kToneLevel, 0.84f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 0.8f }, { kNoiseDecay, 95.0f }, { kNoiseCurve, -0.6f }, { kNoiseHPF, 500.0f }, { kNoiseLPF, 13500.0f }, { kNoiseStereoWidth, 1.05f }, { kNoiseLevel, 0.85f },
                { kClickLevel, 0.55f }, { kClickFreq, 6500.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -15.0f }, { kCompRatio, 5.0f }, { kCompAttack, 2.0f }, { kCompRelease, 45.0f }, { kCompMakeup, 3.2f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 15.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.35f }, { kReverbDecay, 160.0f }, { kReverbDamp, 0.55f }, { kReverbGate, 70.0f }, { kReverbMix, 0.16f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.25f }, { kStereoWidth, 1.05f }, { kOutputGain, 0.0f }
            }
        },
        {
            "40 Clean Electro Pop Crisp",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 9.0f }, { kPreDecay, 6.0f }, { kPreJitter, 0.08f }, { kPreLevel, 0.84f },
                { kToneNote, 11.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1400.0f }, { kToneQ, 3.5f }, { kToneDecay, 115.0f }, { kTonePitchEnv, 7.0f }, { kTonePitchDecay, 15.0f }, { kToneLevel, 0.78f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 1.2f }, { kNoiseDecay, 165.0f }, { kNoiseCurve, -0.3f }, { kNoiseHPF, 420.0f }, { kNoiseLPF, 14500.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.88f },
                { kClickLevel, 0.48f }, { kClickFreq, 5800.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -14.0f }, { kCompRatio, 3.8f }, { kCompAttack, 3.5f }, { kCompRelease, 60.0f }, { kCompMakeup, 2.6f }, { kCompMix, 0.95f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 18.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.58f }, { kReverbDecay, 360.0f }, { kReverbDamp, 0.38f }, { kReverbGate, 130.0f }, { kReverbMix, 0.28f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.35f }, { kStereoWidth, 1.25f }, { kOutputGain, 0.0f }
            }
        },
        {
            "41 Clean Future Bass Air",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 16.0f }, { kPreDecay, 10.0f }, { kPreJitter, 0.20f }, { kPreLevel, 0.88f },
                { kToneNote, 4.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1100.0f }, { kToneQ, 2.8f }, { kToneDecay, 140.0f }, { kTonePitchEnv, 5.0f }, { kTonePitchDecay, 20.0f }, { kToneLevel, 0.74f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.6f }, { kNoiseHold, 2.5f }, { kNoiseDecay, 240.0f }, { kNoiseCurve, -0.2f }, { kNoiseHPF, 350.0f }, { kNoiseLPF, 15000.0f }, { kNoiseStereoWidth, 1.5f }, { kNoiseLevel, 0.92f },
                { kClickLevel, 0.35f }, { kClickFreq, 4600.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -15.0f }, { kCompRatio, 3.5f }, { kCompAttack, 6.0f }, { kCompRelease, 85.0f }, { kCompMakeup, 2.8f }, { kCompMix, 0.9f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 22.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.80f }, { kReverbDecay, 720.0f }, { kReverbDamp, 0.32f }, { kReverbGate, 220.0f }, { kReverbMix, 0.42f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.5f }, { kStereoWidth, 1.4f }, { kOutputGain, 0.0f }
            }
        },
        {
            "42 Clean Stadium Anthem Slap",
            {
                { kPreBursts, 4.0f }, { kPreSpread, 18.0f }, { kPreDecay, 11.0f }, { kPreJitter, 0.22f }, { kPreLevel, 0.90f },
                { kToneNote, 9.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1200.0f }, { kToneQ, 3.3f }, { kToneDecay, 160.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 22.0f }, { kToneLevel, 0.76f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 3.0f }, { kNoiseDecay, 230.0f }, { kNoiseCurve, -0.25f }, { kNoiseHPF, 370.0f }, { kNoiseLPF, 13500.0f }, { kNoiseStereoWidth, 1.4f }, { kNoiseLevel, 0.92f },
                { kClickLevel, 0.45f }, { kClickFreq, 5000.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -15.0f }, { kCompRatio, 4.0f }, { kCompAttack, 5.0f }, { kCompRelease, 80.0f }, { kCompMakeup, 3.0f }, { kCompMix, 0.95f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.85f }, { kReverbDecay, 850.0f }, { kReverbDamp, 0.28f }, { kReverbGate, 180.0f }, { kReverbMix, 0.44f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.3f }, { kOutputGain, 0.0f }
            }
        },
        {
            "43 Clean Minimal Micro Clap",
            {
                { kPreBursts, 1.0f }, { kPreSpread, 5.0f }, { kPreDecay, 3.0f }, { kPreJitter, 0.02f }, { kPreLevel, 0.70f },
                { kToneNote, 0.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1400.0f }, { kToneQ, 4.5f }, { kToneDecay, 55.0f }, { kTonePitchEnv, 10.0f }, { kTonePitchDecay, 8.0f }, { kToneLevel, 0.75f },
                { kNoiseType, 2.0f }, { kNoiseAttack, 0.0f }, { kNoiseHold, 0.5f }, { kNoiseDecay, 60.0f }, { kNoiseCurve, -0.7f }, { kNoiseHPF, 650.0f }, { kNoiseLPF, 12000.0f }, { kNoiseStereoWidth, 0.9f }, { kNoiseLevel, 0.78f },
                { kClickLevel, 0.50f }, { kClickFreq, 7200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -12.0f }, { kCompRatio, 4.5f }, { kCompAttack, 1.5f }, { kCompRelease, 35.0f }, { kCompMakeup, 2.0f }, { kCompMix, 1.0f },
                { kLimiterCeiling, -0.3f }, { kLimiterRelease, 12.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.25f }, { kReverbDecay, 90.0f }, { kReverbDamp, 0.65f }, { kReverbGate, 40.0f }, { kReverbMix, 0.10f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.2f }, { kStereoWidth, 0.95f }, { kOutputGain, 0.0f }
            }
        },
        {
            "44 Clean Euphoric Trance Lift",
            {
                { kPreBursts, 3.0f }, { kPreSpread, 13.0f }, { kPreDecay, 8.0f }, { kPreJitter, 0.16f }, { kPreLevel, 0.86f },
                { kToneNote, 11.0f }, { kToneOctave, 3.0f }, { kToneFreq, 1200.0f }, { kToneQ, 3.4f }, { kToneDecay, 140.0f }, { kTonePitchEnv, 6.0f }, { kTonePitchDecay, 18.0f }, { kToneLevel, 0.76f },
                { kNoiseType, 0.0f }, { kNoiseAttack, 0.2f }, { kNoiseHold, 2.5f }, { kNoiseDecay, 210.0f }, { kNoiseCurve, -0.22f }, { kNoiseHPF, 380.0f }, { kNoiseLPF, 13000.0f }, { kNoiseStereoWidth, 1.35f }, { kNoiseLevel, 0.90f },
                { kClickLevel, 0.42f }, { kClickFreq, 5200.0f },
                { kDistType, 0.0f }, { kDistDrive, 0.0f }, { kDistTone, 0.0f }, { kDistMix, 0.0f },
                { kCompThresh, -14.0f }, { kCompRatio, 3.6f }, { kCompAttack, 4.5f }, { kCompRelease, 75.0f }, { kCompMakeup, 2.8f }, { kCompMix, 0.95f },
                { kLimiterCeiling, -0.2f }, { kLimiterRelease, 20.0f }, { kLimiterEnabled, 1.0f },
                { kReverbSize, 0.72f }, { kReverbDecay, 580.0f }, { kReverbDamp, 0.34f }, { kReverbGate, 180.0f }, { kReverbMix, 0.40f },
                { kReverseMode, 0.0f }, { kReverseMix, 0.4f }, { kStereoWidth, 1.25f }, { kOutputGain, 0.0f }
            }
        }
    };
    return kPresets;
}

void apply(const FactoryPreset& preset, juce::AudioProcessorValueTreeState& state, bool preservePitch) {
    for (const auto& [paramId, value] : preset.values) {
        if (preservePitch && (paramId == kToneOctave || paramId == kToneFreq || paramId == kToneNote || paramId == kToneFine)) {
            continue;
        }
        if (auto* param = state.getParameter(paramId)) {
            param->setValueNotifyingHost(param->convertTo0to1(value));
        }
    }
}

} // namespace openclapper::presets
