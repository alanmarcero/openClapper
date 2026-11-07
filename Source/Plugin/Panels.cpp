#include "Panels.h"
#include "BinaryData.h"
#include "Parameters.h"
#include "PluginProcessor.h"
#include "Theme.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace openclapper::ui {

namespace {

constexpr int kRefreshHz = 60;
constexpr int kCaptionHeight = 18;

} // namespace

// ============================================================================
// ClappyStagePanel
// ============================================================================

ClappyStagePanel::ClappyStagePanel(OpenClapperProcessor& processor)
    : processor_(processor) {
    mascot_ = juce::ImageCache::getFromMemory(BinaryData::djclappy_png, BinaryData::djclappy_pngSize);

    auditionBtn_.setColour(juce::TextButton::buttonColourId, theme::kKnobBody);
    auditionBtn_.setColour(juce::TextButton::textColourOffId, theme::kAccentNeonGreen);
    auditionBtn_.onClick = [this]() {
        processor_.triggerAudition();
    };
    addAndMakeVisible(auditionBtn_);

    startTimerHz(kRefreshHz);
}

ClappyStagePanel::~ClappyStagePanel() {
    stopTimer();
}

void ClappyStagePanel::timerCallback() {
    const uint32_t serial = processor_.displayTriggerCount.load(std::memory_order_relaxed);
    if (serial != seenTriggers_) {
        seenTriggers_ = serial;
        clapTriggerTimeMs_ = juce::Time::getMillisecondCounterHiRes();
    }
    repaint();
}

void ClappyStagePanel::resized() {
    auto area = getLocalBounds().reduced(12, 10);
    auditionBtn_.setBounds(area.removeFromBottom(24).reduced(area.getWidth() / 4, 0));
}

void ClappyStagePanel::paint(juce::Graphics& g) {
    drawPanel(g, getLocalBounds(), "DJ Clappy Stage", theme::kAccentCyan);

    auto stage = getLocalBounds().reduced(8, 28).withTrimmedBottom(28).toFloat();
    const juce::Graphics::ScopedSaveState saved(g);
    g.reduceClipRegion(stage.toNearestInt());

    // Stage deep gradient background
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff0d141e), stage.getTopLeft(),
        juce::Colour(0xff06090d), stage.getBottomRight(), false));
    g.fillRoundedRectangle(stage, 8.0f);
    g.setColour(theme::kPanelEdge.withAlpha(0.7f));
    g.drawRoundedRectangle(stage.reduced(0.5f), 8.0f, 1.0f);

    const float peak = std::clamp(processor_.displayPeak.load(), 0.0f, 1.0f);
    const float beatPhase = std::clamp(processor_.displayBeatPhase.load(), 0.0f, 1.0f);
    const float pulse = std::max(peak, 0.35f * std::pow(1.0f - beatPhase, 3.0f));
    const bool locked = processor_.isClapLockedIn();

    // Trigger animation timing
    const double clapAgeMs = juce::Time::getMillisecondCounterHiRes() - clapTriggerTimeMs_;
    const float clapPhase = (clapAgeMs < 0.0 || clapAgeMs > 320.0)
        ? 1.0f
        : static_cast<float>(clapAgeMs / 320.0);

    // Compute hand clap posture: -1 (approach flam), 0 (impact slap), +1 (recoil spring)
    if (clapPhase < 0.22f) {
        // Multi-flam rapid micro-twitch approaching impact
        const float microFlam = std::sin(clapPhase * 40.0f * std::numbers::pi_v<float>);
        clapPose_ = -(1.0f - clapPhase / 0.22f) * 0.8f + microFlam * 0.2f;
        shockwave_ = 0.0f;
    } else if (clapPhase < 0.38f) {
        // Clap impact smack!
        const float strike = (clapPhase - 0.22f) / 0.16f;
        clapPose_ = 0.0f;
        shockwave_ = 1.0f - strike;
    } else if (clapPhase < 1.0f) {
        // Smooth damped recoil
        const float recoil = (clapPhase - 0.38f) / 0.62f;
        clapPose_ = std::sin(recoil * std::numbers::pi_v<float> * 2.0f) * std::exp(-recoil * 3.5f) * 0.6f;
        shockwave_ = std::max(0.0f, 1.0f - recoil * 2.0f);
    } else {
        clapPose_ = 0.0f;
        shockwave_ = 0.0f;
    }

    const bool isClapOn = (clapPhase < 0.85f);

    // Subwoofer cone behind DJ Clappy
    const juce::Point<float> coneCentre{ stage.getCentreX(), stage.getCentreY() + 10.0f };
    const float baseRadius = std::min(stage.getWidth(), stage.getHeight()) * 0.38f;
    const float pumpRadius = (baseRadius + pulse * 10.0f);
    const juce::Rectangle<float> rimRect(
        coneCentre.x - baseRadius - 6.0f, coneCentre.y - baseRadius - 6.0f,
        (baseRadius + 6.0f) * 2.0f, (baseRadius + 6.0f) * 2.0f);

    drawGlow(g, rimRect.expanded(16.0f), (isClapOn ? theme::kAccentNeonGreen : theme::kAccentCyan).withAlpha(0.15f + pulse * 0.35f));
    g.setColour(juce::Colour(0xff18222d));
    g.fillEllipse(rimRect);
    g.setColour((isClapOn ? theme::kAccentNeonGreen : theme::kAccentCyan).withAlpha(0.35f + pulse * 0.5f));
    g.drawEllipse(rimRect.reduced(2.0f), 1.5f);

    const juce::Rectangle<float> coneRect(coneCentre.x - pumpRadius, coneCentre.y - pumpRadius, pumpRadius * 2.0f, pumpRadius * 2.0f);
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff090f15), coneCentre,
        juce::Colour(0xff2b3a4a), coneCentre.translated(pumpRadius, 0.0f), true));
    g.fillEllipse(coneRect);

    // Stage spotlight beams
    for (int side = 0; side < 2; ++side) {
        const float srcX = side == 0 ? stage.getX() + 20.0f : stage.getRight() - 20.0f;
        const float floorY = stage.getBottom() - 4.0f;
        const auto beamCol = side == 0 ? theme::kAccentCyan : theme::kAccentAmber;
        juce::Path beam;
        beam.startNewSubPath(srcX - 6.0f, floorY);
        beam.lineTo(stage.getCentreX() - 35.0f, stage.getY() + 4.0f);
        beam.lineTo(stage.getCentreX() + 35.0f, stage.getY() + 4.0f);
        beam.lineTo(srcX + 6.0f, floorY);
        beam.closeSubPath();
        g.setGradientFill(juce::ColourGradient(
            beamCol.withAlpha(0.15f + pulse * 0.25f), srcX, floorY,
            beamCol.withAlpha(0.0f), stage.getCentreX(), stage.getY(), false));
        g.fillPath(beam);
    }

    // Shockwave rings emitted from hands on clap impact
    if (shockwave_ > 0.01f) {
        const juce::Point<float> clapImpact{ stage.getCentreX(), stage.getCentreY() + 18.0f };
        for (int w = 0; w < 3; ++w) {
            const float r = 16.0f + static_cast<float>(w) * 16.0f + (1.0f - shockwave_) * 40.0f;
            g.setColour(theme::kAccentNeonGreen.withAlpha(shockwave_ * (0.6f - static_cast<float>(w) * 0.15f)));
            g.drawEllipse(clapImpact.x - r, clapImpact.y - r * 0.7f, r * 2.0f, r * 1.4f, 2.0f);
        }
    }

    // Draw the 3D DJ Clappy mascot with clap animations and transforms
    if (mascot_.isValid()) {
        const juce::Graphics::ScopedSaveState mascotSave(g);
        // Squash on clap impact and recoil bounce
        const float scaleX = 1.0f + clapPose_ * 0.08f;
        const float scaleY = 1.0f - std::abs(clapPose_) * 0.05f;
        const float shiftY = -std::abs(clapPose_) * 4.0f;

        g.addTransform(juce::AffineTransform::translation(-stage.getCentreX(), -(stage.getCentreY() + 12.0f)));
        g.addTransform(juce::AffineTransform::scale(scaleX, scaleY));
        g.addTransform(juce::AffineTransform::translation(stage.getCentreX(), stage.getCentreY() + 12.0f + shiftY));

        g.setColour(juce::Colours::white);
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        const auto mascotBounds = stage.reduced(10.0f, 6.0f).toNearestInt();
        g.drawImageWithin(mascot_, mascotBounds.getX(), mascotBounds.getY(), mascotBounds.getWidth(), mascotBounds.getHeight(),
                          juce::RectanglePlacement::centred);
    }

    // Slogan banner at top of stage: "Clap on. Clap off."
    const auto bannerRect = stage.removeFromTop(24.0f);
    g.setColour(juce::Colour(0xaa060a0f));
    g.fillRoundedRectangle(bannerRect.reduced(14.0f, 2.0f), 4.0f);

    // Strobe / Glow slogan text
    const auto sloganCol = isClapOn ? theme::kAccentNeonGreen : theme::kAccentAmber;
    g.setColour(sloganCol);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("CLAP ON. CLAP OFF.", bannerRect, juce::Justification::centred, true);

    // Neon State Indicator LED
    const auto ledRect = juce::Rectangle<float>(stage.getX() + 14.0f, stage.getY() + 4.0f, 65.0f, 16.0f);
    g.setColour(isClapOn ? theme::kAccentNeonGreen.withAlpha(0.25f) : theme::kKnobBody);
    g.fillRoundedRectangle(ledRect, 3.0f);
    g.setColour(isClapOn ? theme::kAccentNeonGreen : theme::kTextDim);
    g.drawRoundedRectangle(ledRect, 3.0f, 1.0f);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText(isClapOn ? "CLAP ON" : "CLAP OFF", ledRect, juce::Justification::centred, true);

    // Locked WAV badge if active
    if (locked) {
        const auto lockRect = juce::Rectangle<float>(stage.getRight() - 78.0f, stage.getY() + 4.0f, 64.0f, 16.0f);
        g.setColour(theme::kAccentAmber.withAlpha(0.25f));
        g.fillRoundedRectangle(lockRect, 3.0f);
        g.setColour(theme::kAccentAmber);
        g.drawRoundedRectangle(lockRect, 3.0f, 1.0f);
        g.drawText("LOCKED", lockRect, juce::Justification::centred, true);
    }
}

// ============================================================================
// ClapSynthPanel
// ============================================================================

ClapSynthPanel::ClapSynthPanel(OpenClapperProcessor& processor)
    : preBursts_(processor.parameters, openclapper::params::kPreBursts, "Bursts", theme::kAccentCyan),
      preSpread_(processor.parameters, openclapper::params::kPreSpread, "Spread", theme::kAccentCyan),
      preDecay_(processor.parameters, openclapper::params::kPreDecay, "Decay", theme::kAccentCyan),
      preJitter_(processor.parameters, openclapper::params::kPreJitter, "Humanize", theme::kAccentCyan),
      flamSpread_(processor.parameters, openclapper::params::kFlamSpread, "Flam Pan", theme::kAccentCyan),
      preLevel_(processor.parameters, openclapper::params::kPreLevel, "Level", theme::kAccentCyan),

      toneMode_(processor.parameters, openclapper::params::kToneMode, "Mode", theme::kAccentAmber),
      toneNote_(processor.parameters, openclapper::params::kToneNote, "Note", theme::kAccentAmber),
      toneOctave_(processor.parameters, openclapper::params::kToneOctave, "Octave", theme::kAccentAmber),
      toneFine_(processor.parameters, openclapper::params::kToneFine, "Fine", theme::kAccentAmber),
      toneFreq_(processor.parameters, openclapper::params::kToneFreq, "Freq", theme::kAccentAmber),
      toneQ_(processor.parameters, openclapper::params::kToneQ, "Res / Q", theme::kAccentAmber),
      toneDecay_(processor.parameters, openclapper::params::kToneDecay, "Decay", theme::kAccentAmber),
      tonePitchEnv_(processor.parameters, openclapper::params::kTonePitchEnv, "Pitch Drop", theme::kAccentAmber),
      tonePitchDecay_(processor.parameters, openclapper::params::kTonePitchDecay, "Drop Time", theme::kAccentAmber),
      toneLevel_(processor.parameters, openclapper::params::kToneLevel, "Level", theme::kAccentAmber),

      noiseType_(processor.parameters, openclapper::params::kNoiseType, "Type", theme::kAccentViolet),
      noiseAttack_(processor.parameters, openclapper::params::kNoiseAttack, "Attack", theme::kAccentViolet),
      noiseHold_(processor.parameters, openclapper::params::kNoiseHold, "Hold", theme::kAccentViolet),
      noiseDecay_(processor.parameters, openclapper::params::kNoiseDecay, "Decay", theme::kAccentViolet),
      noiseCurve_(processor.parameters, openclapper::params::kNoiseCurve, "Curve", theme::kAccentViolet),
      noiseHPF_(processor.parameters, openclapper::params::kNoiseHPF, "HPF", theme::kAccentViolet),
      noiseLPF_(processor.parameters, openclapper::params::kNoiseLPF, "LPF", theme::kAccentViolet),
      noiseWidth_(processor.parameters, openclapper::params::kNoiseStereoWidth, "Width", theme::kAccentViolet),
      noiseLevel_(processor.parameters, openclapper::params::kNoiseLevel, "Level", theme::kAccentViolet),

      clickType_(processor.parameters, openclapper::params::kClickType, "Click Type", theme::kAccentNeonGreen),
      clickLevel_(processor.parameters, openclapper::params::kClickLevel, "Click Lvl", theme::kAccentNeonGreen),
      clickFreq_(processor.parameters, openclapper::params::kClickFreq, "Click Freq", theme::kAccentNeonGreen) {
    addAndMakeVisible(preBursts_);
    addAndMakeVisible(preSpread_);
    addAndMakeVisible(preDecay_);
    addAndMakeVisible(preJitter_);
    addAndMakeVisible(flamSpread_);
    addAndMakeVisible(preLevel_);

    addAndMakeVisible(toneMode_);
    addAndMakeVisible(toneNote_);
    addAndMakeVisible(toneOctave_);
    addAndMakeVisible(toneFine_);
    addAndMakeVisible(toneFreq_);
    addAndMakeVisible(toneQ_);
    addAndMakeVisible(toneDecay_);
    addAndMakeVisible(tonePitchEnv_);
    addAndMakeVisible(tonePitchDecay_);
    addAndMakeVisible(toneLevel_);

    addAndMakeVisible(noiseType_);
    addAndMakeVisible(noiseAttack_);
    addAndMakeVisible(noiseHold_);
    addAndMakeVisible(noiseDecay_);
    addAndMakeVisible(noiseCurve_);
    addAndMakeVisible(noiseHPF_);
    addAndMakeVisible(noiseLPF_);
    addAndMakeVisible(noiseWidth_);
    addAndMakeVisible(noiseLevel_);

    addAndMakeVisible(clickType_);
    addAndMakeVisible(clickLevel_);
    addAndMakeVisible(clickFreq_);
}

void ClapSynthPanel::paint(juce::Graphics& g) {
    drawPanel(g, getLocalBounds(), "Clap Synthesis Architecture", theme::kAccentCyan);
}

void ClapSynthPanel::resized() {
    auto area = getLocalBounds().reduced(8, 26);
    const int rowHeight = area.getHeight() / 3;

    // Row 1: Pre-Clap Flam (left 2/3) & Click (right 1/3)
    auto row1 = area.removeFromTop(rowHeight);
    auto clickArea = row1.removeFromRight(row1.getWidth() / 3);
    layoutRow(row1, { &preBursts_, &preSpread_, &preDecay_, &preJitter_, &flamSpread_, &preLevel_ });
    layoutRow(clickArea, { &clickType_, &clickLevel_, &clickFreq_ });

    // Row 2: Resonant Body / Tone
    auto row2 = area.removeFromTop(rowHeight);
    layoutRow(row2, { &toneMode_, &toneNote_, &toneOctave_, &toneFine_, &toneFreq_, &toneQ_, &toneDecay_, &tonePitchEnv_, &tonePitchDecay_, &toneLevel_ });

    // Row 3: Noise & Sizzle
    layoutRow(area, { &noiseType_, &noiseAttack_, &noiseHold_, &noiseDecay_, &noiseCurve_, &noiseHPF_, &noiseLPF_, &noiseWidth_, &noiseLevel_ });
}

// ============================================================================
// EffectsPanel
// ============================================================================

EffectsPanel::EffectsPanel(OpenClapperProcessor& processor)
    : distType_(processor.parameters, openclapper::params::kDistType, "Dist Type", theme::kAccentFlame),
      distDrive_(processor.parameters, openclapper::params::kDistDrive, "Drive", theme::kAccentFlame),
      distTone_(processor.parameters, openclapper::params::kDistTone, "Tone", theme::kAccentFlame),
      distMix_(processor.parameters, openclapper::params::kDistMix, "Dist Mix", theme::kAccentFlame),

      compThresh_(processor.parameters, openclapper::params::kCompThresh, "Threshold", theme::kAccentAmber),
      compRatio_(processor.parameters, openclapper::params::kCompRatio, "Ratio", theme::kAccentAmber),
      compAttack_(processor.parameters, openclapper::params::kCompAttack, "Attack", theme::kAccentAmber),
      compRelease_(processor.parameters, openclapper::params::kCompRelease, "Release", theme::kAccentAmber),
      compMakeup_(processor.parameters, openclapper::params::kCompMakeup, "Makeup", theme::kAccentAmber),
      compMix_(processor.parameters, openclapper::params::kCompMix, "Comp Mix", theme::kAccentAmber),

      limiterCeiling_(processor.parameters, openclapper::params::kLimiterCeiling, "Ceiling", theme::kAccentNeonGreen),
      limiterRelease_(processor.parameters, openclapper::params::kLimiterRelease, "Release", theme::kAccentNeonGreen),
      limiterEnabled_(processor.parameters, openclapper::params::kLimiterEnabled, "Limiter", theme::kAccentNeonGreen),

      reverbSize_(processor.parameters, openclapper::params::kReverbSize, "Size", theme::kAccentViolet),
      reverbDecay_(processor.parameters, openclapper::params::kReverbDecay, "Decay", theme::kAccentViolet),
      reverbDamp_(processor.parameters, openclapper::params::kReverbDamp, "Damping", theme::kAccentViolet),
      reverbGate_(processor.parameters, openclapper::params::kReverbGate, "Gate Time", theme::kAccentViolet),
      reverbMix_(processor.parameters, openclapper::params::kReverbMix, "Rev Mix", theme::kAccentViolet),

      reverseMode_(processor.parameters, openclapper::params::kReverseMode, "Rev Sync", theme::kAccentCyan),
      reverseMix_(processor.parameters, openclapper::params::kReverseMix, "Rev Mix", theme::kAccentCyan),
      midiMode_(processor.parameters, openclapper::params::kMidiMode, "MIDI Mode", theme::kTextDim),
      lockInBtn_(processor.parameters, openclapper::params::kLockIn, "LOCK IN", theme::kAccentAmber),
      masterWidth_(processor.parameters, openclapper::params::kStereoWidth, "Width", theme::kAccentCyan),
      outputGain_(processor.parameters, openclapper::params::kOutputGain, "Output", theme::kAccentAmber) {
    addAndMakeVisible(distType_);
    addAndMakeVisible(distDrive_);
    addAndMakeVisible(distTone_);
    addAndMakeVisible(distMix_);

    addAndMakeVisible(compThresh_);
    addAndMakeVisible(compRatio_);
    addAndMakeVisible(compAttack_);
    addAndMakeVisible(compRelease_);
    addAndMakeVisible(compMakeup_);
    addAndMakeVisible(compMix_);

    addAndMakeVisible(limiterCeiling_);
    addAndMakeVisible(limiterRelease_);
    addAndMakeVisible(limiterEnabled_);

    addAndMakeVisible(reverbSize_);
    addAndMakeVisible(reverbDecay_);
    addAndMakeVisible(reverbDamp_);
    addAndMakeVisible(reverbGate_);
    addAndMakeVisible(reverbMix_);

    addAndMakeVisible(reverseMode_);
    addAndMakeVisible(reverseMix_);
    addAndMakeVisible(midiMode_);
    addAndMakeVisible(lockInBtn_);
    addAndMakeVisible(masterWidth_);
    addAndMakeVisible(outputGain_);
}

void EffectsPanel::paint(juce::Graphics& g) {
    drawPanel(g, getLocalBounds(), "Impact Distortion, Dynamics & Space", theme::kAccentFlame);
}

void EffectsPanel::resized() {
    auto area = getLocalBounds().reduced(8, 26);
    const int rowHeight = area.getHeight() / 2;

    // Row 1: Distortion (left) & Dynamics: Compressor + Limiter (right)
    auto row1 = area.removeFromTop(rowHeight);
    auto distArea = row1.removeFromLeft(row1.getWidth() * 4 / 11);
    layoutRow(distArea, { &distType_, &distDrive_, &distTone_, &distMix_ });
    layoutRow(row1, { &compThresh_, &compRatio_, &compAttack_, &compRelease_, &compMakeup_, &compMix_, &limiterCeiling_ });

    // Row 2: Reverb (left) & Master/Sync/Lock (right)
    auto revArea = area.removeFromLeft(area.getWidth() * 5 / 11);
    layoutRow(revArea, { &reverbSize_, &reverbDecay_, &reverbDamp_, &reverbGate_, &reverbMix_ });
    layoutRow(area, { &reverseMode_, &reverseMix_, &midiMode_, &masterWidth_, &outputGain_, &lockInBtn_ });
}

} // namespace openclapper::ui
