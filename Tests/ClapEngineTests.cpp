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

TEST_CASE("ClapEngine WAV Lock-In captures and recalls audio bit-exact", "[core][engine]") {
    ClapEngine engine;
    engine.prepare(48000.0);

    EngineSettings settings;
    engine.lockCurrentClap(settings);

    REQUIRE(engine.isClapLocked());
    REQUIRE(!engine.wavCache().left().empty());

    const auto wavBytes = engine.wavCache().toWavBytes();
    REQUIRE(wavBytes.size() > 44);

    ClapEngine restoredEngine;
    restoredEngine.prepare(48000.0);
    const bool success = restoredEngine.restoreLockedWav(wavBytes);

    REQUIRE(success);
    REQUIRE(restoredEngine.isClapLocked());
    REQUIRE(restoredEngine.wavCache().left().size() == engine.wavCache().left().size());
}
