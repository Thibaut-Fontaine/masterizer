#include "masterizer/application/RealtimeChain.h"

#include "masterizer/domain/dsp/GainStage.h"
#include "masterizer/domain/dsp/Saturator.h"
#include "masterizer/domain/dsp/StereoWidth.h"

namespace masterizer::application {
namespace {

domain::dsp::LookaheadLimiterSettings limiterFrom(const domain::dsp::LimiterSettings& settings) {
    domain::dsp::LookaheadLimiterSettings result;
    result.ceilingDb = settings.ceilingDb;
    result.releaseSeconds = settings.releaseSeconds;
    return result;
}

}  // namespace

RealtimeChain::RealtimeChain(double sampleRate, int maxBlockSize, const ChainSettings& settings,
                             const RealtimeChainBands& bands) {
    if (settings.monoBassEnabled) {
        monoBass_.emplace(sampleRate, settings.monoBass);
    }
    saturationEnabled_ = settings.saturationEnabled;
    saturation_ = settings.saturation;
    if (settings.resonanceEnabled) {
        resonanceLeft_.emplace(sampleRate, bands.resonance);
        resonanceRight_.emplace(sampleRate, bands.resonance);
    }
    if (settings.eqEnabled) {
        eqLeft_.emplace(sampleRate, bands.eq);
        eqRight_.emplace(sampleRate, bands.eq);
    }
    if (settings.fineEqEnabled) {
        fineEqLeft_.emplace(sampleRate, bands.fineEq);
        fineEqRight_.emplace(sampleRate, bands.fineEq);
    }
    stereoEnabled_ = settings.stereoEnabled;
    stereoWidthScale_ = settings.stereoWidthScale;
    if (settings.transientEnabled) {
        transientLeft_.emplace(sampleRate, settings.transient);
        transientRight_.emplace(sampleRate, settings.transient);
    }
    if (settings.glueEnabled) {
        glueLeft_.emplace(sampleRate, settings.glue);
        glueRight_.emplace(sampleRate, settings.glue);
    }
    if (settings.multibandEnabled) {
        multibandLeft_.emplace(sampleRate, settings.multiband);
        multibandRight_.emplace(sampleRate, settings.multiband);
        multibandLeft_->prepare(maxBlockSize);
        multibandRight_->prepare(maxBlockSize);
    }
    if (settings.dynamicEqEnabled) {
        dynamicEqLeft_.emplace(sampleRate, settings.dynamicEq);
        dynamicEqRight_.emplace(sampleRate, settings.dynamicEq);
    }
    if (settings.tapeEnabled) {
        tapeLeft_.emplace(sampleRate, settings.tape);
        tapeRight_.emplace(sampleRate, settings.tape);
    }
    outputGainDb_ = settings.outputGainDb;
    if (settings.limiterEnabled) {
        const domain::dsp::LookaheadLimiterSettings limiter = limiterFrom(settings.limiter);
        limiterLeft_.emplace(sampleRate, limiter);
        limiterRight_.emplace(sampleRate, limiter);
    }
}

int RealtimeChain::latencySamples() const {
    return limiterLeft_ ? limiterLeft_->latencySamples() : 0;
}

void RealtimeChain::process(float* left, float* right, int count) {
    if (left == nullptr || right == nullptr || count <= 0) {
        return;
    }
    if (monoBass_) {
        monoBass_->process(left, right, count);
    }
    if (saturationEnabled_) {
        domain::dsp::applySaturation(left, count, saturation_);
        domain::dsp::applySaturation(right, count, saturation_);
    }
    if (resonanceLeft_) {
        resonanceLeft_->process(left, count);
        resonanceRight_->process(right, count);
    }
    if (eqLeft_) {
        eqLeft_->process(left, count);
        eqRight_->process(right, count);
    }
    if (fineEqLeft_) {
        fineEqLeft_->process(left, count);
        fineEqRight_->process(right, count);
    }
    if (stereoEnabled_) {
        domain::dsp::applyStereoWidth(left, right, count, stereoWidthScale_);
    }
    if (transientLeft_) {
        transientLeft_->process(left, count);
        transientRight_->process(right, count);
    }
    if (glueLeft_) {
        glueLeft_->process(left, count);
        glueRight_->process(right, count);
    }
    if (multibandLeft_) {
        multibandLeft_->process(left, count);
        multibandRight_->process(right, count);
    }
    if (dynamicEqLeft_) {
        dynamicEqLeft_->process(left, count);
        dynamicEqRight_->process(right, count);
    }
    if (tapeLeft_) {
        tapeLeft_->process(left, count);
        tapeRight_->process(right, count);
    }
    if (outputGainDb_ != 0.0) {
        domain::dsp::applyGain(left, count, outputGainDb_);
        domain::dsp::applyGain(right, count, outputGainDb_);
    }
    if (limiterLeft_) {
        limiterLeft_->process(left, count);
        limiterRight_->process(right, count);
    }
}

}  // namespace masterizer::application
