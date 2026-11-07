#include "Effects.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace openclapper::core;

TEST_CASE("DistortionShaper saturates predictably across all types", "[core][effects]") {
    DistortionShaper shaper;
    const double sampleRate = 48000.0;

    const std::array<DistortionType, 4> types = {
        DistortionType::Tape,
        DistortionType::Tube,
        DistortionType::HardClip,
        DistortionType::Wavefold
    };

    for (auto type : types) {
        shaper.reset();
        const float outLow = shaper.processSample(0.5f, type, 0.0f, 0.0f, sampleRate);
        REQUIRE(std::abs(outLow - 0.5f) < 0.001f); // 0 drive is bypass

        const float outDriven = shaper.processSample(0.5f, type, 0.8f, 0.0f, sampleRate);
        REQUIRE_FALSE(std::isnan(outDriven));
        REQUIRE(std::abs(outDriven) <= 1.5f); // Output remains bounded
    }
}

TEST_CASE("StereoCompressor applies gain reduction on loud transients", "[core][effects]") {
    StereoCompressor comp;
    comp.prepare(48000.0);

    CompressorSettings settings;
    settings.thresholdDb = -12.0f;
    settings.ratio = 6.0f;
    settings.attackMs = 0.5f;
    settings.releaseMs = 50.0f;
    settings.makeupDb = 0.0f;
    settings.mix = 1.0f;

    // Process a loud 0 dBFS signal block
    float sampleL = 1.0f;
    float sampleR = 1.0f;
    for (int i = 0; i < 500; ++i) {
        sampleL = 1.0f;
        sampleR = 1.0f;
        comp.process(sampleL, sampleR, settings);
    }

    // Must be compressed below 1.0 (gain reduction applied)
    REQUIRE(sampleL < 0.6f);
    REQUIRE(sampleR < 0.6f);
    REQUIRE(sampleL > 0.01f);
}

TEST_CASE("BrickwallLimiter prevents signal from exceeding ceiling", "[core][effects]") {
    BrickwallLimiter limiter;
    limiter.prepare(48000.0);

    LimiterSettings settings;
    settings.ceilingDb = -0.5f; // ~0.944 linear
    settings.enabled = true;

    const float ceilingLin = std::pow(10.0f, -0.5f / 20.0f);

    // Feed massive overs (+12 dBFS)
    for (int i = 0; i < 200; ++i) {
        float sampleL = 4.0f;
        float sampleR = 4.0f;
        limiter.process(sampleL, sampleR, settings);

        REQUIRE(std::abs(sampleL) <= (ceilingLin + 0.001f));
        REQUIRE(std::abs(sampleR) <= (ceilingLin + 0.001f));
    }
}

TEST_CASE("Reverse sync timing targets are exact", "[core][effects]") {
    const double ppq = 3.25; // Beat 4 of bar 1 (in 4/4 meter)
    const double beatsPerBar = 4.0;

    const double targetBeat4 = reverseTargetPpq(ReverseSyncMode::Beat4Auto, ppq, beatsPerBar);
    REQUIRE(targetBeat4 == 4.0); // Next bar starts at 4.0

    const double targetBeatEnd = reverseTargetPpq(ReverseSyncMode::BeatEnd, ppq, beatsPerBar);
    REQUIRE(targetBeatEnd == 4.0);

    const double targetHalfBeat = reverseTargetPpq(ReverseSyncMode::HalfBeat, ppq, beatsPerBar);
    REQUIRE(targetHalfBeat == 3.5);
}
