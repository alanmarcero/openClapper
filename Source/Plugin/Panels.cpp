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
        impactIntensity_ = std::clamp(processor_.displayVelocity.load(std::memory_order_relaxed), 0.35f, 1.0f);
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

    // Trigger animation timing across multi-stage choreography
    const double clapAgeMs = juce::Time::getMillisecondCounterHiRes() - clapTriggerTimeMs_;
    constexpr double kAnimDurationMs = 520.0;
    const float clapPhase = (clapAgeMs < 0.0 || clapAgeMs > kAnimDurationMs)
        ? 1.0f
        : static_cast<float>(clapAgeMs / kAnimDurationMs);
    const bool isTriggerActive = (clapPhase < 1.0f);

    // Multi-stage motion curves
    if (clapPhase < 0.16f) {
        // Stage 1: Pre-burst flam windup & hand flutter
        const float flutter = std::sin(clapPhase * 48.0f * std::numbers::pi_v<float>);
        clapPose_ = -(1.0f - clapPhase / 0.16f) * 0.9f + flutter * 0.25f;
        headTilt_ = (clapPhase / 0.16f) * 0.02f;
        shockwave_ = 0.0f;
        impactFlash_ = 0.0f;
    } else if (clapPhase < 0.28f) {
        // Stage 2: Clap impact smack! Instant peak compression and flash
        const float strike = (clapPhase - 0.16f) / 0.12f;
        clapPose_ = -std::sin(strike * std::numbers::pi_v<float>) * 1.0f;
        headTilt_ = 0.0f;
        shockwave_ = (1.0f - strike) * impactIntensity_;
        impactFlash_ = std::pow(1.0f - strike, 1.4f) * impactIntensity_;
    } else if (clapPhase < 1.0f) {
        // Stage 3: Dynamic harmonic recoil bounce & settling
        const float recoil = (clapPhase - 0.28f) / 0.72f;
        const float decay = std::exp(-recoil * 3.8f);
        clapPose_ = std::sin(recoil * std::numbers::pi_v<float> * 3.0f) * decay * 0.85f;
        headTilt_ = std::sin(recoil * std::numbers::pi_v<float> * 2.0f) * decay * 0.035f;
        shockwave_ = std::max(0.0f, (1.0f - recoil * 2.2f) * impactIntensity_);
        impactFlash_ = std::max(0.0f, impactFlash_ * 0.92f);
    } else {
        // Idle tempo groove
        clapPose_ = 0.0f;
        headTilt_ = 0.0f;
        shockwave_ = 0.0f;
        impactFlash_ = 0.0f;
        idleBob_ = std::sin(beatPhase * 2.0f * std::numbers::pi_v<float>) * 2.0f;
    }

    const bool isClapOn = (clapPhase < 0.75f && isTriggerActive);

    // Subwoofer cone behind DJ Clappy with acoustic bass ripples
    const juce::Point<float> coneCentre{ stage.getCentreX(), stage.getCentreY() + 10.0f };
    const float baseRadius = std::min(stage.getWidth(), stage.getHeight()) * 0.38f;
    const float pumpExtra = (pulse * 14.0f) + (impactFlash_ * 12.0f);
    const float pumpRadius = baseRadius + pumpExtra;

    const juce::Rectangle<float> rimRect(
        coneCentre.x - baseRadius - 6.0f, coneCentre.y - baseRadius - 6.0f,
        (baseRadius + 6.0f) * 2.0f, (baseRadius + 6.0f) * 2.0f);

    const auto subGlowColor = isClapOn ? theme::kAccentNeonGreen : theme::kAccentCyan;
    drawGlow(g, rimRect.expanded(18.0f), subGlowColor.withAlpha(0.18f + pulse * 0.40f + impactFlash_ * 0.35f));
    g.setColour(juce::Colour(0xff18222d));
    g.fillEllipse(rimRect);

    // Concentric acoustic bass pressure ripples
    if (pulse > 0.15f || impactFlash_ > 0.05f) {
        for (int r = 1; r <= 3; ++r) {
            const float rippleR = baseRadius * (0.35f + 0.22f * static_cast<float>(r)) + pumpExtra * 0.4f;
            g.setColour(subGlowColor.withAlpha((0.35f - static_cast<float>(r) * 0.08f) * (pulse + impactFlash_)));
            g.drawEllipse(coneCentre.x - rippleR, coneCentre.y - rippleR, rippleR * 2.0f, rippleR * 2.0f, 1.2f);
        }
    }

    g.setColour(subGlowColor.withAlpha(0.35f + pulse * 0.5f + impactFlash_ * 0.4f));
    g.drawEllipse(rimRect.reduced(2.0f), 1.5f);

    const juce::Rectangle<float> coneRect(coneCentre.x - pumpRadius, coneCentre.y - pumpRadius, pumpRadius * 2.0f, pumpRadius * 2.0f);
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff090f15), coneCentre,
        juce::Colour(0xff2b3a4a), coneCentre.translated(pumpRadius, 0.0f), true));
    g.fillEllipse(coneRect);

    // Dynamic sweeping festival spotlight beams
    for (int side = 0; side < 2; ++side) {
        const float srcX = side == 0 ? stage.getX() + 16.0f : stage.getRight() - 16.0f;
        const float floorY = stage.getBottom() - 4.0f;
        const auto beamCol = side == 0 ? theme::kAccentCyan : theme::kAccentAmber;

        const float sweepPhase = (side == 0 ? beatPhase : beatPhase + 0.5f);
        const float sweepOffset = std::sin(sweepPhase * 2.0f * std::numbers::pi_v<float>) * 24.0f;
        const float targetX = stage.getCentreX() + (isTriggerActive ? (side == 0 ? -12.0f : 12.0f) : sweepOffset);
        const float targetY = stage.getY() + 10.0f;

        juce::Path beam;
        beam.startNewSubPath(srcX - 8.0f, floorY);
        beam.lineTo(targetX - 25.0f, targetY);
        beam.lineTo(targetX + 25.0f, targetY);
        beam.lineTo(srcX + 8.0f, floorY);
        beam.closeSubPath();

        const float beamIntensity = 0.16f + pulse * 0.30f + (impactFlash_ * 0.45f);
        g.setGradientFill(juce::ColourGradient(
            beamCol.withAlpha(beamIntensity), srcX, floorY,
            beamCol.withAlpha(0.0f), targetX, targetY, false));
        g.fillPath(beam);
    }

    // Mascot transform parameters
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float shiftY = 0.0f;

    if (isTriggerActive) {
        if (clapPhase < 0.28f) {
            // Compress downward on impact slam
            const float frac = (1.0f - clapPhase / 0.28f);
            scaleX = 1.0f + 0.14f * impactIntensity_ * frac;
            scaleY = 1.0f - 0.10f * impactIntensity_ * frac;
            shiftY = 6.0f * impactIntensity_ * frac;
        } else {
            // Spring recoil rebound
            scaleX = 1.0f - clapPose_ * 0.08f * impactIntensity_;
            scaleY = 1.0f + clapPose_ * 0.10f * impactIntensity_;
            shiftY = -clapPose_ * 10.0f * impactIntensity_;
        }
    } else {
        shiftY = idleBob_;
        scaleY = 1.0f - std::abs(idleBob_) * 0.005f;
    }

    // Expanding shockwave rings & spark flares from robotic hands
    if (shockwave_ > 0.01f) {
        const juce::Point<float> clapImpact{ stage.getCentreX() + 2.0f, stage.getCentreY() + 18.0f + shiftY };

        // Central neon flare
        const float flareRadius = 24.0f + (1.0f - shockwave_) * 32.0f;
        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xffffffff).withAlpha(shockwave_ * 0.9f), clapImpact,
            theme::kAccentNeonGreen.withAlpha(0.0f), clapImpact.translated(flareRadius, 0.0f), true));
        g.fillEllipse(clapImpact.x - flareRadius, clapImpact.y - flareRadius * 0.7f, flareRadius * 2.0f, flareRadius * 1.4f);

        // Concentric cyber shockwave rings
        const std::array<juce::Colour, 4> ringCols = {
            theme::kAccentNeonGreen,
            theme::kAccentCyan,
            theme::kAccentAmber,
            theme::kAccentNeonGreen
        };
        for (size_t w = 0; w < ringCols.size(); ++w) {
            const float r = 14.0f + static_cast<float>(w) * 16.0f + (1.0f - shockwave_) * 60.0f;
            const float ringAlpha = shockwave_ * std::max(0.0f, 0.75f - static_cast<float>(w) * 0.16f);
            g.setColour(ringCols[w].withAlpha(ringAlpha));
            g.drawEllipse(clapImpact.x - r, clapImpact.y - r * 0.65f, r * 2.0f, r * 1.3f, 2.2f);
        }

        // Electric spark rays shooting outward
        constexpr int kNumRays = 8;
        for (int i = 0; i < kNumRays; ++i) {
            const float angle = static_cast<float>(i) * (2.0f * std::numbers::pi_v<float> / static_cast<float>(kNumRays));
            const float innerDist = 12.0f + (1.0f - shockwave_) * 15.0f;
            const float outerDist = innerDist + 18.0f * shockwave_;
            const float x1 = clapImpact.x + std::cos(angle) * innerDist;
            const float y1 = clapImpact.y + std::sin(angle) * innerDist * 0.7f;
            const float x2 = clapImpact.x + std::cos(angle) * outerDist;
            const float y2 = clapImpact.y + std::sin(angle) * outerDist * 0.7f;
            g.setColour(juce::Colours::white.withAlpha(shockwave_ * 0.8f));
            g.drawLine(x1, y1, x2, y2, 1.8f);
        }
    }

    // Draw the 3D DJ Clappy mascot with clap animations and transforms
    if (mascot_.isValid()) {
        const juce::Graphics::ScopedSaveState mascotSave(g);
        const float pivotX = stage.getCentreX();
        const float pivotY = stage.getCentreY() + 12.0f;

        g.addTransform(juce::AffineTransform::translation(-pivotX, -pivotY));
        g.addTransform(juce::AffineTransform::rotation(headTilt_));
        g.addTransform(juce::AffineTransform::scale(scaleX, scaleY));
        g.addTransform(juce::AffineTransform::translation(pivotX, pivotY + shiftY));

        g.setColour(juce::Colours::white);
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        const auto mascotBounds = stage.reduced(10.0f, 6.0f).toNearestInt();
        g.drawImageWithin(mascot_, mascotBounds.getX(), mascotBounds.getY(), mascotBounds.getWidth(), mascotBounds.getHeight(),
                          juce::RectanglePlacement::centred);
    }

    // Visor laser highlight flash across cyber eyewear
    if (impactFlash_ > 0.02f) {
        const float visorY = stage.getCentreY() - 36.0f + shiftY;
        const float visorWidth = 84.0f + impactFlash_ * 30.0f;
        const auto visorRect = juce::Rectangle<float>(stage.getCentreX() - visorWidth * 0.5f, visorY - 7.0f, visorWidth, 14.0f);
        drawGlow(g, visorRect.expanded(12.0f), theme::kAccentCyan.withAlpha(impactFlash_ * 0.85f));

        g.setGradientFill(juce::ColourGradient(
            juce::Colour(0xffffffff).withAlpha(impactFlash_ * 0.95f), stage.getCentreX(), visorY,
            theme::kAccentCyan.withAlpha(0.0f), stage.getCentreX() - visorWidth * 0.5f, visorY, true));
        g.fillRoundedRectangle(visorRect, 4.0f);
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
    const auto ledRect = juce::Rectangle<float>(stage.getX() + 14.0f, stage.getY() + 4.0f, 68.0f, 18.0f);
    if (isClapOn) {
        drawGlow(g, ledRect.expanded(6.0f), theme::kAccentNeonGreen.withAlpha(0.35f + impactFlash_ * 0.4f));
    }
    g.setColour(isClapOn ? theme::kAccentNeonGreen.withAlpha(0.28f + impactFlash_ * 0.25f) : theme::kKnobBody);
    g.fillRoundedRectangle(ledRect, 3.0f);
    g.setColour(isClapOn ? theme::kAccentNeonGreen : theme::kPanelEdge);
    g.drawRoundedRectangle(ledRect, 3.0f, 1.0f);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(isClapOn ? theme::kAccentNeonGreen : theme::kTextDim);
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
      flamSpread_(processor.parameters, openclapper::params::kFlamSpread, "Flam Pan", theme::kAccentCyan),
      preLevel_(processor.parameters, openclapper::params::kPreLevel, "Level", theme::kAccentCyan),

      toneMode_(processor.parameters, openclapper::params::kToneMode, "Mode", theme::kAccentAmber),
      toneOctave_(processor.parameters, openclapper::params::kToneOctave, "Octave", theme::kAccentAmber),
      lockPitchBtn_(processor.parameters, openclapper::params::kLockPitch, "LOCK PITCH", theme::kAccentAmber),
      toneFreq_(processor.parameters, openclapper::params::kToneFreq, "Tone Freq", theme::kAccentAmber),
      toneQ_(processor.parameters, openclapper::params::kToneQ, "Res / Q", theme::kAccentAmber),
      toneDecay_(processor.parameters, openclapper::params::kToneDecay, "Decay", theme::kAccentAmber),
      tonePitchEnv_(processor.parameters, openclapper::params::kTonePitchEnv, "Pitch Drop", theme::kAccentAmber),
      toneLevel_(processor.parameters, openclapper::params::kToneLevel, "Level", theme::kAccentAmber),

      noiseType_(processor.parameters, openclapper::params::kNoiseType, "Type", theme::kAccentViolet),
      noiseDecay_(processor.parameters, openclapper::params::kNoiseDecay, "Decay", theme::kAccentViolet),
      noiseHPF_(processor.parameters, openclapper::params::kNoiseHPF, "HPF", theme::kAccentViolet),
      noiseLPF_(processor.parameters, openclapper::params::kNoiseLPF, "LPF", theme::kAccentViolet),
      noiseWidth_(processor.parameters, openclapper::params::kNoiseStereoWidth, "Width", theme::kAccentViolet),
      noiseLevel_(processor.parameters, openclapper::params::kNoiseLevel, "Level", theme::kAccentViolet),

      clickType_(processor.parameters, openclapper::params::kClickType, "Click Type", theme::kAccentNeonGreen),
      clickLevel_(processor.parameters, openclapper::params::kClickLevel, "Click Lvl", theme::kAccentNeonGreen),
      clickFreq_(processor.parameters, openclapper::params::kClickFreq, "Click Freq", theme::kAccentNeonGreen) {
    addAndMakeVisible(preBursts_);
    addAndMakeVisible(preSpread_);
    addAndMakeVisible(flamSpread_);
    addAndMakeVisible(preLevel_);

    addAndMakeVisible(clickType_);
    addAndMakeVisible(clickLevel_);
    addAndMakeVisible(clickFreq_);

    addAndMakeVisible(toneMode_);
    addAndMakeVisible(toneOctave_);
    addAndMakeVisible(lockPitchBtn_);
    addAndMakeVisible(toneFreq_);
    addAndMakeVisible(toneQ_);
    addAndMakeVisible(toneDecay_);
    addAndMakeVisible(tonePitchEnv_);
    addAndMakeVisible(toneLevel_);

    addAndMakeVisible(noiseType_);
    addAndMakeVisible(noiseDecay_);
    addAndMakeVisible(noiseHPF_);
    addAndMakeVisible(noiseLPF_);
    addAndMakeVisible(noiseWidth_);
    addAndMakeVisible(noiseLevel_);
}

void ClapSynthPanel::paint(juce::Graphics& g) {
    drawPanel(g, getLocalBounds(), "Clap Synthesis Architecture", theme::kAccentCyan);
}

void ClapSynthPanel::resized() {
    auto area = getLocalBounds().reduced(8, 26);
    const int rowHeight = area.getHeight() / 3;

    // Row 1: Pre-Clap Flam (left ~58%) & Click (right ~42%)
    auto row1 = area.removeFromTop(rowHeight);
    auto flamArea = row1.removeFromLeft(static_cast<int>(static_cast<float>(row1.getWidth()) * 0.58f));
    layoutRow(flamArea, { &preBursts_, &preSpread_, &flamSpread_, &preLevel_ });
    layoutRow(row1, { &clickType_, &clickLevel_, &clickFreq_ });

    // Row 2: Resonant Body / Tone (Mode, Octave, Lock Oct/Freq, Tone Freq, Q, Decay, Pitch Drop, Level)
    auto row2 = area.removeFromTop(rowHeight);
    layoutRow(row2, { &toneMode_, &toneOctave_, &lockPitchBtn_, &toneFreq_, &toneQ_, &toneDecay_, &tonePitchEnv_, &toneLevel_ });

    // Row 3: Noise & Sizzle (Type, Decay, HPF, LPF, Width, Level)
    layoutRow(area, { &noiseType_, &noiseDecay_, &noiseHPF_, &noiseLPF_, &noiseWidth_, &noiseLevel_ });
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
      compMakeup_(processor.parameters, openclapper::params::kCompMakeup, "Makeup", theme::kAccentAmber),
      compMix_(processor.parameters, openclapper::params::kCompMix, "Comp Mix", theme::kAccentAmber),

      limiterCeiling_(processor.parameters, openclapper::params::kLimiterCeiling, "Ceiling", theme::kAccentNeonGreen),
      limiterEnabled_(processor.parameters, openclapper::params::kLimiterEnabled, "Limiter", theme::kAccentNeonGreen),

      reverbSize_(processor.parameters, openclapper::params::kReverbSize, "Size", theme::kAccentViolet),
      reverbDecay_(processor.parameters, openclapper::params::kReverbDecay, "Decay", theme::kAccentViolet),
      reverbGate_(processor.parameters, openclapper::params::kReverbGate, "Gate Time", theme::kAccentViolet),
      reverbMix_(processor.parameters, openclapper::params::kReverbMix, "Rev Mix", theme::kAccentViolet),

      reverseMode_(processor.parameters, openclapper::params::kReverseMode, "Rev Sync", theme::kAccentCyan),
      reverseMix_(processor.parameters, openclapper::params::kReverseMix, "Rev Mix", theme::kAccentCyan),
      midiMode_(processor.parameters, openclapper::params::kMidiMode, "MIDI Mode", theme::kTextDim),
      lockInBtn_(processor.parameters, openclapper::params::kLockIn, "LOCK WAV", theme::kAccentAmber),
      masterWidth_(processor.parameters, openclapper::params::kStereoWidth, "Width", theme::kAccentCyan),
      outputGain_(processor.parameters, openclapper::params::kOutputGain, "Output", theme::kAccentAmber) {
    addAndMakeVisible(distType_);
    addAndMakeVisible(distDrive_);
    addAndMakeVisible(distTone_);
    addAndMakeVisible(distMix_);

    addAndMakeVisible(compThresh_);
    addAndMakeVisible(compRatio_);
    addAndMakeVisible(compMakeup_);
    addAndMakeVisible(compMix_);

    addAndMakeVisible(limiterCeiling_);
    addAndMakeVisible(limiterEnabled_);

    addAndMakeVisible(reverbSize_);
    addAndMakeVisible(reverbDecay_);
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

    // Row 1: Distortion (left ~40%) & Dynamics: Compressor + Limiter (right ~60%)
    auto row1 = area.removeFromTop(rowHeight);
    auto distArea = row1.removeFromLeft(static_cast<int>(static_cast<float>(row1.getWidth()) * 0.40f));
    layoutRow(distArea, { &distType_, &distDrive_, &distTone_, &distMix_ });
    layoutRow(row1, { &compThresh_, &compRatio_, &compMakeup_, &compMix_, &limiterCeiling_, &limiterEnabled_ });

    // Row 2: Reverb (left ~40%) & Master/Sync/Lock (right ~60%)
    auto revArea = area.removeFromLeft(static_cast<int>(static_cast<float>(area.getWidth()) * 0.40f));
    layoutRow(revArea, { &reverbSize_, &reverbDecay_, &reverbGate_, &reverbMix_ });
    layoutRow(area, { &reverseMode_, &reverseMix_, &midiMode_, &masterWidth_, &outputGain_, &lockInBtn_ });
}

} // namespace openclapper::ui
