#include "Effects.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <numbers>
#include <ranges>

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

TEST_CASE("DistortionShaper uses 4x oversampled DC-blocked ADAA2 to suppress harmonic foldback", "[core][effects][distortion]") {
    constexpr double sr = 48000.0;
    constexpr int warmup = 4096;
    constexpr int N = 16384;
    constexpr double testHz = 11300.0;

    DistortionShaper tapeShaper;
    std::vector<double> outTape(N, 0.0);
    double dcSum = 0.0;
    std::ranges::for_each(std::views::iota(0, warmup + N), [&](int n) {
        const float x = 0.45f * static_cast<float>(std::sin(2.0 * std::numbers::pi * testHz * static_cast<double>(n) / sr));
        const float y = tapeShaper.processSample(x, DistortionType::Tape, 0.85f, 0.0f, sr);
        if (n >= warmup) {
            outTape[static_cast<size_t>(n - warmup)] = y;
            dcSum += y;
        }
    });

    const auto dftPower = [&](double freqHz) {
        double re = 0.0;
        double im = 0.0;
        std::ranges::for_each(std::views::iota(0, N), [&](int n) {
            const double w = 0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * static_cast<double>(n) / (N - 1));
            const double angle = 2.0 * std::numbers::pi * freqHz * static_cast<double>(n) / sr;
            re += outTape[static_cast<size_t>(n)] * w * std::cos(angle);
            im -= outTape[static_cast<size_t>(n)] * w * std::sin(angle);
        });
        return re * re + im * im;
    };

    const double fundPower = dftPower(testHz);
    const double evenAlias2800 = dftPower(48000.0 - 4.0 * testHz);
    const double oddAlias14100 = dftPower(48000.0 - 3.0 * testHz);
    REQUIRE(fundPower > 1.0);
    REQUIRE(evenAlias2800 < fundPower * 1.0e-6);
    REQUIRE(oddAlias14100 < fundPower * 2.0e-5);
    REQUIRE(std::abs(dcSum / static_cast<double>(N)) < 1.0e-3);

    std::ranges::for_each(std::array{ DistortionType::Tube, DistortionType::HardClip, DistortionType::Wavefold }, [&](DistortionType mode) {
        DistortionShaper modeShaper;
        double modeDc = 0.0;
        double modeEnergy = 0.0;
        std::ranges::for_each(std::views::iota(0, warmup + N), [&](int n) {
            const float x = 0.45f * static_cast<float>(std::sin(2.0 * std::numbers::pi * testHz * static_cast<double>(n) / sr));
            const float y = modeShaper.processSample(x, mode, 0.8f, 0.0f, sr);
            REQUIRE(std::isfinite(y));
            if (n >= warmup) {
                modeDc += y;
                modeEnergy += static_cast<double>(y) * y;
            }
        });
        REQUIRE(modeEnergy > 1.0);
        REQUIRE(std::abs(modeDc / static_cast<double>(N)) < 1.0e-3);
    });
}

TEST_CASE("BrickwallLimiter applies 64-sample true-peak lookahead without flattening pre-clap flams", "[core][effects][limiter]") {
    constexpr double sr = 48000.0;
    BrickwallLimiter limiter;
    limiter.prepare(sr);

    LimiterSettings settings;
    settings.ceilingDb = -1.0f;
    settings.releaseMs = 30.0f;
    settings.enabled = true;
    const float ceiling = std::pow(10.0f, settings.ceilingDb / 20.0f);

    std::vector<float> flamOut(200, 0.0f);
    std::ranges::for_each(std::views::iota(0, 200), [&](int n) {
        float l = 0.0f;
        if (n == 0) l = 0.30f;
        if (n == 58) l = 3.0f;
        float r = l;
        limiter.process(l, r, settings);
        flamOut[static_cast<size_t>(n)] = l;
    });

    float preLatencyMax = 0.0f;
    std::ranges::for_each(std::views::iota(0, 64), [&](int n) {
        preLatencyMax = std::max(preLatencyMax, std::abs(flamOut[static_cast<size_t>(n)]));
    });
    REQUIRE(preLatencyMax == 0.0f);
    REQUIRE(flamOut[64] > 0.27f);
    REQUIRE(std::abs(flamOut[64 + 58]) <= ceiling + 0.002f);

    // 12 kHz cosine at pi/4 phase has sample values <= 0.884 (< ceiling) but continuous true peak = 1.25.
    limiter.reset();
    float steadySamplePeak = 0.0f;
    std::ranges::for_each(std::views::iota(0, 600), [&](int n) {
        float l = 1.25f * static_cast<float>(std::cos(0.5 * std::numbers::pi * static_cast<double>(n) + 0.25 * std::numbers::pi));
        float r = l;
        limiter.process(l, r, settings);
        if (n >= 200) {
            steadySamplePeak = std::max(steadySamplePeak, std::abs(l));
        }
    });
    REQUIRE(steadySamplePeak <= ceiling * 0.74f);
}
