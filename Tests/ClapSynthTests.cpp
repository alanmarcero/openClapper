#include "ClapSynth.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace openclapper::core;

TEST_CASE("midiNoteToHz maps MIDI pitch accurately", "[core][synth]") {
    const float a4 = midiNoteToHz(69);
    REQUIRE(std::abs(a4 - 440.0f) < 0.1f);

    const float a3 = midiNoteToHz(57);
    REQUIRE(std::abs(a4 / a3 - 2.0f) < 0.001f);
}

TEST_CASE("ClapSynth produces valid audio block without NaNs", "[core][synth]") {
    ClapSynth synth;
    synth.prepare(48000.0);

    ClapSynthSettings settings;
    std::vector<float> left;
    std::vector<float> right;

    synth.renderBlock(left, right, settings, 1.0f, 0);

    REQUIRE(!left.empty());
    REQUIRE(left.size() == right.size());

    float maxPeak = 0.0f;
    for (size_t i = 0; i < left.size(); ++i) {
        REQUIRE_FALSE(std::isnan(left[i]));
        REQUIRE_FALSE(std::isnan(right[i]));
        REQUIRE_FALSE(std::isinf(left[i]));
        REQUIRE_FALSE(std::isinf(right[i]));
        maxPeak = std::max(maxPeak, std::max(std::abs(left[i]), std::abs(right[i])));
    }

    REQUIRE(maxPeak > 0.05f); // Must produce an audible signal
}

TEST_CASE("ClapSynth finishes and stops playing after duration", "[core][synth]") {
    ClapSynth synth;
    synth.prepare(48000.0);

    ClapSynthSettings settings;
    settings.noiseDecayMs = 30.0f;
    settings.toneDecayMs = 30.0f;
    settings.reverbDecayMs = 50.0f;
    settings.reverbGateMs = 40.0f;

    synth.trigger(settings, 69, 1.0f);
    REQUIRE(synth.isPlaying());

    // Process until finished
    int maxSamples = 48000;
    while (synth.isPlaying() && --maxSamples > 0) {
        [[maybe_unused]] const auto s = synth.processSample(settings);
    }

    REQUIRE_FALSE(synth.isPlaying());
    REQUIRE(synth.getEnvelopeProgress() >= 1.0f);
}

TEST_CASE("GatedStereoReverb respects gate cutoff", "[core][synth]") {
    GatedStereoReverb reverb;
    reverb.prepare(48000.0);

    // Feed an impulse at t=0
    [[maybe_unused]] const auto initFrame = reverb.process(1.0f, 1.0f, 0.5f, 500.0f, 0.2f, 50.0f, 48000.0);

    // Process until impulse travels through delay tank (~30ms = ~1440 samples)
    float maxTail = 0.0f;
    for (int i = 0; i < 2000; ++i) {
        const auto f = reverb.process(0.0f, 0.0f, 0.5f, 500.0f, 0.2f, 50.0f, 48000.0);
        maxTail = std::max(maxTail, std::max(std::abs(f.left), std::abs(f.right)));
    }
    REQUIRE(maxTail > 0.01f);

    // Advance past gate time (50 ms gate + 15ms release = 65 ms = 3120 samples)
    StereoFrame gatedFrame = { 0.0f, 0.0f };
    for (int i = 0; i < 3000; ++i) {
        gatedFrame = reverb.process(0.0f, 0.0f, 0.5f, 500.0f, 0.2f, 50.0f, 48000.0);
    }

    REQUIRE(gatedFrame.left == 0.0f);
    REQUIRE(gatedFrame.right == 0.0f);
}

TEST_CASE("ClapSynth renders across all ToneModes and ClickTypes without errors", "[core][synth]") {
    ClapSynth synth;
    synth.prepare(48000.0);

    const ToneMode modes[] = { ToneMode::Bandpass, ToneMode::SubThump, ToneMode::SaturatedTri, ToneMode::CyberSquare };
    const ClickType clicks[] = { ClickType::Needle, ClickType::Crack, ClickType::Thump };

    for (auto mode : modes) {
        for (auto click : clicks) {
            ClapSynthSettings settings;
            settings.toneMode = mode;
            settings.clickType = click;
            settings.toneLevel = 0.8f;
            settings.clickLevel = 0.5f;

            std::vector<float> left, right;
            synth.renderBlock(left, right, settings, 1.0f, 0);

            REQUIRE(!left.empty());
            REQUIRE(left.size() == right.size());

            float peak = 0.0f;
            for (size_t i = 0; i < left.size(); ++i) {
                REQUIRE_FALSE(std::isnan(left[i]));
                REQUIRE_FALSE(std::isnan(right[i]));
                peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
            }
            REQUIRE(peak > 0.1f);
        }
    }
}

TEST_CASE("ClapSynth flam spread creates stereo differentiation during pre-bursts", "[core][synth]") {
    ClapSynth synth;
    synth.prepare(48000.0);

    ClapSynthSettings wideFlam;
    wideFlam.preClapCount = 4;
    wideFlam.preClapSpreadMs = 15.0f;
    wideFlam.preClapFlamSpread = 1.0f; // maximum stereo spread
    wideFlam.toneLevel = 0.0f;
    wideFlam.noiseLevel = 0.0f;
    wideFlam.clickLevel = 0.0f;
    wideFlam.reverbMix = 0.0f;

    std::vector<float> left, right;
    synth.renderBlock(left, right, wideFlam, 1.0f, 0);

    // During pre-bursts, left and right channels should not be identical
    float diffSum = 0.0f;
    for (size_t i = 0; i < left.size(); ++i) {
        diffSum += std::abs(left[i] - right[i]);
    }
    REQUIRE(diffSum > 0.01f);
}

TEST_CASE("ClapSynth tone pitch responds to note and octave settings", "[core][synth]") {
    ClapSynth synth;
    synth.prepare(48000.0);

    ClapSynthSettings lowPitchSettings;
    lowPitchSettings.toneLevel = 1.0f;
    lowPitchSettings.noiseLevel = 0.0f;
    lowPitchSettings.clickLevel = 0.0f;
    lowPitchSettings.reverbMix = 0.0f;
    lowPitchSettings.preClapCount = 0;
    ClapSynthSettings highPitchSettings = lowPitchSettings;

    std::vector<float> leftLow, rightLow;
    synth.renderBlock(leftLow, rightLow, lowPitchSettings, 1.0f, 36 - 69);

    std::vector<float> leftHigh, rightHigh;
    synth.renderBlock(leftHigh, rightHigh, highPitchSettings, 1.0f, 81 - 69);

    REQUIRE(!leftLow.empty());
    REQUIRE(!leftHigh.empty());

    // Waveforms with different note/octave pitches must not be identical
    float diffSum = 0.0f;
    const size_t len = std::min(leftLow.size(), leftHigh.size());
    for (size_t i = 0; i < len; ++i) {
        diffSum += std::abs(leftLow[i] - leftHigh[i]);
    }
    REQUIRE(diffSum > 1.0f);
}
