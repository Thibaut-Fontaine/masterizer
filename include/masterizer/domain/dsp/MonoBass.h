#pragma once

namespace masterizer::domain::dsp {

/// Low-frequency mono/M-S settings.
struct MonoBassSettings {
    double transitionHz = 95.0;  // default region ~90-100 Hz
    bool strictMono = false;     // steeper slope below the transition
};

/// Collapses the low end to mono by removing the low-frequency Side component
/// from a stereo pair, in place. A first-order crossover is used, or a steeper
/// (second-order) one when `strictMono` is set.
void applyMonoBass(float* left, float* right, int count, double sampleRate,
                   const MonoBassSettings& settings);

}  // namespace masterizer::domain::dsp
