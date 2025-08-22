#pragma once

#include <optional>
#include <cstdint>

#include <inplace_function.hpp>

struct MeasureBeltTensionSpecificParams {
    /// (meters) Excitation amplitude for the measurement
    float excitation_amplitude_m = 0;

    /// \returns excitation amplitude (in meters)
    using ExcitationAmplitudeFunc = float (*)(float frequency_hz);

    /// Utility ExcitationAmplitudeFunc that linearly changes excitation amplitude based on the specified parameters.
    template <float freq_a_hz, float freq_b_hz, float amplicute_a_m, float amplitude_b_m>
    static constexpr float linearly_varying_amplitude(float freq) {
        return amplicute_a_m + (freq - freq_a_hz) * (amplitude_b_m - amplicute_a_m) / (freq_b_hz - freq_a_hz);
    }

    /// (meters) Excitation amplitude for the measurement, dependent on the frequency.
    /// Overrides \p excitation_amplitude_m
    ExcitationAmplitudeFunc excitation_amplitude_m_func = {};

    /// (Hz) Start frequency of the tuning scan (relates to minimum detectable tension)
    float start_frequency_hz;

    /// (Hz) End frequency of the tuning scan (relates to maximum detectable tensions)
    /// Some XLs have quite tight belts from the manufacturer, so it's recommended to keep this high
    float end_frequency_hz;

    /// (Hz) Increment of the frequency sweep
    float frequency_step_hz;

    /// (1/frequency) How many excitation sine waves we should do
    uint32_t excitation_cycles;

    /// (1/frequency) How many cycles we should wait after excitation
    uint32_t wait_cycles;

    /// (1/frequency) How many cycles we should measure for afterwards
    uint32_t measurement_cycles;

    /// Which harmonic frequency we're measuring
    uint16_t measured_harmonic = 2;
};

struct MeasureBeltTensionParams : public MeasureBeltTensionSpecificParams {
    // Default values determined in BFW-5758, taken from the original prototype implementation.

    /// Belt system we're measuring
    uint8_t belt_system = 0;

    bool calibrate_accelerometer = true;

    /// If set, only the initial setup (homing, selecting tool, positioning the tool) is done.
    /// This is useful for positioning the tool so that the damping gizmos can be put on.
    /// Subsequent tensioning can then be called with \p skip_setup
    bool skip_tuning = false;

    /// If set, the initial setup (homing, selecting tool, positioning the tool) is skipped - it is assumed to be done beforehands
    /// See \p skip_tuning for more info
    bool skip_setup = false;

    struct ProgressCallbackArgs {
        /// Overall progress, 0-1
        float overall_progress = 0;

        /// Last tested frequency
        float last_frequency = NAN;

        /// Response amplitude of the last tested frequency
        float last_result = NAN;

        bool operator==(const ProgressCallbackArgs &) const = default;
        bool operator!=(const ProgressCallbackArgs &) const = default;
    };

    using ProgressCallback = stdext::inplace_function<bool(const ProgressCallbackArgs &args)>;

    /// Callback called after every individual measurement, reporting the overall progress and result of the measurement
    /// If the callback function returns \p false, the procedure gets aborted.
    ProgressCallback progress_callback = {};
};

struct MeasureBeltTensionResult {
    uint8_t belt_system;

    /// The most resonant frequency of the belt
    float resonant_frequency_hz;

    /// \returns force the belt is tensioned with, based on the resonant frequency
    float tension_force_n() const;

    /// \returns number of turns (clockwise -> in the tightening direction) needed to get to the ideal tension
    /// \returns 0 if the tension is within the tolerance
    float adjust_screw_turns() const;
};

/// Measures belt tension by finding out their resonant frequency.
/// Execute only on the marlin thread
std::optional<MeasureBeltTensionResult> measure_belt_tension(const MeasureBeltTensionParams &config);
