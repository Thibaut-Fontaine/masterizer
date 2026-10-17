#pragma once

namespace masterizer::domain::dsp {

/// Subtle tape-style colouration. All values are deliberately small; the stage
/// is intended to be hard to perceive. `driveDb <= 0` disables saturation.
struct TapeSettings {
    double driveDb = 1.5;      // gentle saturation
    double headBumpDb = 0.5;   // low shelf around 70 Hz
    double hfRolloffDb = -0.5; // high shelf around 14 kHz
    double mix = 1.0;          // wet/dry
};

/// Applies head bump -> saturation -> HF roll-off to one channel.
void applyTapeStage(float* samples, int count, double sampleRate, const TapeSettings& settings);

}  // namespace masterizer::domain::dsp
