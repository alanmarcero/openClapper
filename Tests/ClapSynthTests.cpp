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

TEST_CASE("ClapSynth Bandpass body follows its selected noise type", "[core][synth]") {
    ClapSynth whiteSynth;
    ClapSynth pinkSynth;
    whiteSynth.prepare(48000.0);
    pinkSynth.prepare(48000.0);

    ClapSynthSettings whiteSettings;
    whiteSettings.preClapCount = 0;
    whiteSettings.noiseLevel = 0.0f;
    whiteSettings.clickLevel = 0.0f;
    whiteSettings.toneLevel = 0.8f;
    whiteSettings.toneMode = ToneMode::Bandpass;

    auto pinkSettings = whiteSettings;
    pinkSettings.noiseType = NoiseType::Pink;

    std::vector<float> whiteLeft, whiteRight;
    std::vector<float> pinkLeft, pinkRight;
    whiteSynth.renderBlock(whiteLeft, whiteRight, whiteSettings, 1.0f, 69);
    pinkSynth.renderBlock(pinkLeft, pinkRight, pinkSettings, 1.0f, 69);

    float difference = 0.0f;
    for (size_t i = 0; i < std::min(whiteLeft.size(), pinkLeft.size()); ++i)
        difference += std::abs(whiteLeft[i] - pinkLeft[i]);

    REQUIRE(difference > 1.0f);
}

TEST_CASE("ClapSynth produces valid audio block without NaNs", "[core][synth]") {
    ClapSynth synth;
    synth.prepare(48000.0);

    ClapSynthSettings settings;
    std::vector<float> left;
    std::vector<float> right;

    synth.renderBlock(left, right, settings, 1.0f, 69);

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
            synth.renderBlock(left, right, settings, 1.0f, 69);

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

    std::vector<float> left, right;
    synth.renderBlock(left, right, wideFlam, 1.0f, 69);

    // During pre-bursts, left and right channels should not be identical
    float diffSum = 0.0f;
    for (size_t i = 0; i < left.size(); ++i) {
        diffSum += std::abs(left[i] - right[i]);
    }
    REQUIRE(diffSum > 0.01f);
}

TEST_CASE("Pink and lo-fi textures keep independent stereo detail on one-shots and held notes", "[core][synth][stereo]") {
    for (const double rate : {44100.0, 48000.0, 96000.0}) {
        for (int note : {36, 69, 84}) {
            for (const auto type : {NoiseType::Pink, NoiseType::LoFi}) {
                for (bool held : {false, true}) {
                    CAPTURE(rate, note, static_cast<int>(type), held);
                    ClapSynth voice;
                    voice.prepare(rate);
                    ClapSynthSettings sound;
                    sound.noiseType = type;
                    sound.preClapCount = 0;
                    sound.noiseHPF = 50;
                    sound.noiseLPF = 8000;
                    sound.layers[1].solo = true;
                    sound.ampEnabled = held;
                    sound.ampReleaseMs = 60;
                    voice.trigger(sound, note);
                    double ll = 0, rr = 0, lr = 0;
                    const int samples = static_cast<int>(rate * 0.2);
                    for (int i = 0; i < samples; ++i) {
                        if (held && i == samples / 2) voice.noteOff(sound);
                        const auto frame = voice.processSample(sound);
                        ll += frame.left * frame.left;
                        rr += frame.right * frame.right;
                        lr += frame.left * frame.right;
                    }
                    REQUIRE(ll > 1);
                    REQUIRE(rr > 1);
                    REQUIRE(std::abs(lr / std::sqrt(ll * rr)) < 0.25);
                    if (held) REQUIRE_FALSE(voice.isPlaying());
                }
            }
        }
    }
}

TEST_CASE("Flam contacts build toward the main strike at different sample rates", "[core][synth][flam]") {
    for (double rate : {44100.0, 48000.0, 96000.0}) {
        for (int note : {36, 69, 84}) {
            CAPTURE(rate, note);
            ClapSynth voice;
            voice.prepare(rate);
            ClapSynthSettings sound;
            sound.preClapCount = 4;
            sound.preClapJitter = 0;
            sound.preClapSpreadMs = 20;
            sound.preClapDecayMs = 2;
            sound.preClapFlamSpread = 0;
            sound.layers[3].solo = true;
            voice.trigger(sound, note);
            std::array<double, 4> energy{};
            const int window = static_cast<int>(std::round(rate * 0.020));
            for (int i = 0; i < 4 * window; ++i) {
                const auto frame = voice.processSample(sound);
                energy[static_cast<size_t>(i / window)] += frame.left * frame.left + frame.right * frame.right;
            }
            REQUIRE(energy.front() > 0.01);
            REQUIRE(energy.back() > energy.front() * 1.35);
        }
    }
}

TEST_CASE("ClapSynth tone pitch follows MIDI notes", "[core][synth]") {
    ClapSynth synth;
    synth.prepare(48000.0);

    ClapSynthSettings lowPitchSettings;
    lowPitchSettings.toneLevel = 1.0f;
    lowPitchSettings.noiseLevel = 0.0f;
    lowPitchSettings.clickLevel = 0.0f;
    lowPitchSettings.preClapCount = 0;
    ClapSynthSettings highPitchSettings = lowPitchSettings;

    std::vector<float> leftLow, rightLow;
    synth.renderBlock(leftLow, rightLow, lowPitchSettings, 1.0f, 36);

    std::vector<float> leftHigh, rightHigh;
    synth.renderBlock(leftHigh, rightHigh, highPitchSettings, 1.0f, 81);

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

TEST_CASE("ClapSynth tone pitch sweep changes the body at note onset", "[core][synth][pitch]") {
    const auto renderSweep = [](float pitchEnv) {
        ClapSynth synth;
        synth.prepare(48000.0);
        ClapSynthSettings settings;
        settings.preClapCount = 0;
        settings.toneLevel = 1.0f;
        settings.tonePitchEnv = pitchEnv;
        settings.tonePitchDecayMs = 28.0f;
        settings.noiseLevel = 0.0f;
        settings.clickLevel = 0.0f;
        settings.ampEnabled = false;
        std::vector<float> left, right;
        synth.renderBlock(left, right, settings, 1.0f, 57);
        return left;
    };

    const auto flat = renderSweep(0.0f);
    const auto swept = renderSweep(18.0f);
    REQUIRE(flat.size() == swept.size());
    float difference = 0.0f;
    for (size_t i = 0; i < std::min<size_t>(flat.size(), 2048); ++i)
        difference += std::abs(flat[i] - swept[i]);
    REQUIRE(difference > 1.0f);
}

TEST_CASE("ClapSynth body filters isolate low and high content independently", "[core][synth][filter]") {
    const auto renderBody = [](float highpass, float lowpass) {
        ClapSynth synth;
        synth.prepare(48000.0);
        ClapSynthSettings settings;
        settings.preClapCount = 0;
        settings.toneMode = ToneMode::CyberSquare;
        settings.toneLevel = 1.0f;
        settings.toneHPF = highpass;
        settings.toneLPF = lowpass;
        settings.toneDecayMs = 600.0f;
        settings.noiseLevel = 0.0f;
        settings.clickLevel = 0.0f;
        std::vector<float> left, right;
        synth.renderBlock(left, right, settings, 1.0f, 81);
        float energy = 0.0f;
        for (size_t i = 0; i < std::min<size_t>(left.size(), 12000); ++i)
            energy += left[i] * left[i];
        return energy;
    };

    const float fullBand = renderBody(20.0f, 20000.0f);
    const float highPassed = renderBody(4000.0f, 20000.0f);
    const float lowPassed = renderBody(20.0f, 250.0f);
    REQUIRE(fullBand > 0.1f);
    REQUIRE(highPassed < fullBand * 0.45f);
    REQUIRE(lowPassed < fullBand * 0.45f);
}

TEST_CASE("ClapSynth transient decay controls the crack tail", "[core][synth][transient]") {
    const auto renderTail = [](float decayMs) {
        ClapSynth synth;
        synth.prepare(48000.0);
        ClapSynthSettings settings;
        settings.preClapCount = 0;
        settings.toneLevel = 0.0f;
        settings.noiseLevel = 0.0f;
        settings.clickType = ClickType::Crack;
        settings.clickLevel = 1.0f;
        settings.clickDecayMs = decayMs;
        std::vector<float> left, right;
        synth.renderBlock(left, right, settings, 1.0f, 69);
        float energy = 0.0f;
        const size_t start = 500;
        const size_t end = std::min(left.size(), size_t{1500});
        for (size_t i = start; i < end; ++i)
            energy += left[i] * left[i];
        return energy;
    };

    REQUIRE(renderTail(0.5f) < renderTail(8.0f) * 0.02f);
}

TEST_CASE("Full ADSR follows attack decay sustain and a complete twenty second release", "[performance]") {
    ClapSynth envelope, reference;
    envelope.prepare(1000);
    reference.prepare(1000);
    ClapSynthSettings settings;
    settings.preClapCount = 0;
    settings.toneMode = ToneMode::SubThump;
    settings.layers[0].solo = true;
    settings.ampEnabled = true;
    settings.ampAttackMs = 100;
    settings.ampDecayMs = 100;
    settings.ampSustain = 0.25f;
    settings.ampReleaseMs = 20000;
    auto unity = settings;
    unity.ampAttackMs = unity.ampDecayMs = 0;
    unity.ampSustain = 1;
    envelope.trigger(settings, 43);
    reference.trigger(unity, 43);
    for (int sample = 0; sample < 250; ++sample) {
        const float actual = envelope.processSample(settings).left;
        const float raw = reference.processSample(unity).left;
        float gain = 0.25f;
        if (sample < 100) gain = static_cast<float>(sample) / 100.0f;
        else if (sample < 200) gain = 1.0f - 0.75f * static_cast<float>(sample - 100) / 100.0f;
        REQUIRE(std::abs(actual - raw * gain) < 0.00001f);
    }
    envelope.noteOff(settings);
    for (int sample = 0; sample < 20000; ++sample) {
        const float actual = envelope.processSample(settings).left;
        const float raw = reference.processSample(unity).left;
        const float gain = 0.25f * (1.0f - static_cast<float>(sample) / 20000.0f);
        REQUIRE(std::abs(actual - raw * gain) < 0.00001f);
        REQUIRE(envelope.isPlaying());
    }
    REQUIRE(std::abs(envelope.processSample(settings).left) < 0.00001f);
    REQUIRE_FALSE(envelope.isPlaying());
}
