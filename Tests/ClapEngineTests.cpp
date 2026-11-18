#include "ClapEngine.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace openclapper::core;

TEST_CASE("ClapEngine responds to MIDI trigger and processes audio", "[core][engine]") {
    ClapEngine engine;
    engine.prepare(48000.0);

    EngineSettings settings;
    Transport transport;
    transport.bpm = 138.0;

    std::vector<MidiTrigger> triggers = {
        { .sampleOffset = 10, .noteNumber = 69, .velocity = 0.9f, .audition = false }
    };

    std::vector<float> left(256, 0.0f);
    std::vector<float> right(256, 0.0f);
    float* channels[2] = { left.data(), right.data() };

    engine.process(settings, transport, triggers, channels, 2, 256);

    REQUIRE(engine.triggerSerial() == 1);
    REQUIRE(engine.currentPeak() > 0.0f);
    REQUIRE(engine.currentClapProgress() < 1.0f);
}

TEST_CASE("ClapEngine responds to MIDI CC 74 brightness sweep", "[core][engine]") {
    ClapEngine engine;
    engine.prepare(48000.0);

    EngineSettings settings;
    Transport transport;
    transport.bpm = 138.0;

    std::vector<MidiTrigger> triggers = {
        { .sampleOffset = 0, .noteNumber = 69, .velocity = 1.0f, .audition = false }
    };
    // CC 74 set to very low brightness (value 10)
    std::vector<MidiCcEvent> ccEvents = {
        { .sampleOffset = 0, .controller = 74, .value = 10 }
    };

    std::vector<float> leftDark(1024, 0.0f);
    std::vector<float> rightDark(1024, 0.0f);
    float* channelsDark[2] = { leftDark.data(), rightDark.data() };

    engine.process(settings, transport, triggers, channelsDark, 2, 1024, ccEvents);
    REQUIRE(engine.currentPeak() > 0.0f);
}

TEST_CASE("Rolls cancel on transport seeks rather than playing stale fills", "[performance]") {
    ClapEngine engine;
    engine.prepare(48000);
    EngineSettings settings;
    settings.rollDivision = 2;
    settings.rollHits = 4;
    Transport transport;
    transport.bpm = 120;
    transport.ppqPosition = 0.0;
    std::array<float, 64> left{}, right{};
    float* channels[]{left.data(), right.data()};
    const MidiTrigger trigger{};
    engine.process(settings, transport, std::span(&trigger, 1), channels, 2, 64);
    transport.ppqPosition = 16.0;
    engine.process(settings, transport, {}, channels, 2, 64);
    REQUIRE(engine.triggerSerial() == 1);
}

TEST_CASE("A new note at a seek starts a fresh roll", "[performance]") {
    ClapEngine engine;
    engine.prepare(48000);
    EngineSettings settings;
    settings.rollDivision = 2;
    settings.rollHits = 4;
    Transport transport;
    transport.bpm = 120;
    transport.ppqPosition = 0.0;
    std::vector<float> left(7000), right(7000);
    float* channels[]{left.data(), right.data()};
    engine.process(settings, transport, {}, channels, 2, 64);
    transport.ppqPosition = 16.0;
    engine.handleMidi(settings, transport, MidiTrigger{});
    engine.process(settings, transport, {}, channels, 2, 7000);
    REQUIRE(engine.triggerSerial() == 2);
}

TEST_CASE("Independent notes retain simultaneous rolls inside one MIDI block", "[performance]") {
    ClapEngine engine;
    engine.prepare(48000);
    EngineSettings settings;
    settings.rollDivision = 2;
    settings.rollHits = 2;
    Transport transport;
    transport.bpm = 120;
    std::array<MidiTrigger, 2> triggers{{{0, 60, 1.0f}, {1000, 69, 1.0f}}};
    std::vector<float> left(8000), right(8000);
    float* channels[]{left.data(), right.data()};
    engine.process(settings, transport, triggers, channels, 2, 8000);
    REQUIRE(engine.triggerSerial() == 4);
}

TEST_CASE("Streaming MIDI preserves delayed pre-clap effect timing", "[performance]") {
    ClapEngine streamed, buffered;
    streamed.prepare(48000);
    buffered.prepare(48000);
    EngineSettings settings;
    settings.effects.slapbackDivision = SlapbackDivision::Sixteenth;
    settings.effects.reverbGateMs = 20;
    settings.effects.reverbMix = 1;
    Transport transport;
    transport.bpm = 120;
    std::vector<float> leftA(12000), rightA(12000), leftB(12000), rightB(12000);
    float* channelsA[]{leftA.data(), rightA.data()};
    float* channelsB[]{leftB.data(), rightB.data()};
    const MidiTrigger trigger{};
    streamed.handleMidi(settings, transport, trigger);
    streamed.process(settings, transport, {}, channelsA, 2, 12000);
    buffered.process(settings, transport, std::span(&trigger, 1), channelsB, 2, 12000);
    float difference = 0;
    for (size_t sample = 0; sample < leftA.size(); ++sample)
        difference = std::max(difference, std::abs(leftA[sample] - leftB[sample]));
    REQUIRE(difference < 0.000001f);
}

TEST_CASE("Switching rolls Off cancels a pending fill", "[performance]") {
    ClapEngine engine;
    engine.prepare(48000);
    EngineSettings settings;
    settings.rollDivision = 2;
    settings.rollHits = 4;
    Transport transport;
    std::vector<float> left(24000), right(24000);
    float* channels[]{left.data(), right.data()};
    const MidiTrigger trigger{};
    engine.process(settings, transport, std::span(&trigger, 1), channels, 2, 64);
    settings.rollDivision = 0;
    engine.process(settings, transport, {}, channels, 2, 24000);
    REQUIRE(engine.triggerSerial() == 1);
}

TEST_CASE("A driven clap does not sit off centre or explode", "[core][engine][quality]") {
    ClapEngine engine;
    engine.prepare(48000.0);
    EngineSettings settings;
    settings.synth.preClapCount = 3;
    settings.synth.toneLevel = 0.6f;
    settings.synth.noiseLevel = 0.85f;
    settings.synth.clickLevel = 0.4f;
    settings.synth.impactPunch = 0.4f;
    settings.effects.distType = DistortionType::Tape;
    settings.effects.distDrive = 0.7f;
    settings.effects.distMix = 1.0f;
    settings.effects.distTone = 0.2f;
    settings.effects.compressor.mix = 0.0f;
    settings.effects.reverbMix = 0.0f;
    settings.effects.limiter.enabled = false;
    settings.effects.outputGainDb = 0.0f;

    constexpr int samples = 24000;
    std::vector<float> left(samples), right(samples);
    float* channels[]{left.data(), right.data()};
    Transport transport;
    const MidiTrigger trigger{};
    engine.process(settings, transport, std::span(&trigger, 1), channels, 2, samples);

    double mean = 0.0;
    float peak = 0.0f;
    for (int i = 0; i < samples; ++i) {
        REQUIRE(std::isfinite(left[i]));
        REQUIRE(std::isfinite(right[i]));
        mean += static_cast<double>(left[i]) + right[i];
        peak = std::max(peak, std::max(std::abs(left[i]), std::abs(right[i])));
    }
    mean /= static_cast<double>(samples) * 2.0;
    REQUIRE(peak > 0.02f);
    REQUIRE(std::abs(mean) < 2.0e-4);
}
