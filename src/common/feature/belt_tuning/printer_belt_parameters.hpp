#pragma once

#include <array>

#include <Configuration.h>
#include <feature/precise_stepping/common.hpp>

#include "belt_tuning.hpp"

struct PrinterBeltParameters {
    // Although CoreXY machines have two belts, they actually share tension, so they behave as a single belt system
    // Bedslingers have two separate belts
    static constexpr size_t belt_system_count = ENABLED(COREXY) ? 1 : 2;

    struct BeltSystemParameters {
        /// Position of the toolhead at which the measurements should be performed
        xyz_pos_t measurement_pos;

        /// Axis flags used for exciting the belt system
        StepEventFlag_t axis_flags;

        /// (meters) Nominal length of the belt system
        float nominal_length_m;

        /// (kg/meter) Nominal weigt the a belt
        float nominal_weight_kg_m;

        /// (Newtons) Target tension force
        float target_tension_force_n;

        /// (Netwons) Tolerance from the target tension force that is still considered acceptable.
        /// If the measured tension is within (target +- dev), then the tensioning is considered ok
        float target_tension_tolerance_n;

        /// Default parameters used for belt tuning
        MeasureBeltTensionSpecificParams belt_tuning_params;

        /// Calculates belt tension from their resonant frequency.
        /// \returns tension (in Newtons)
        constexpr float resonant_frequency_to_tension(float resonant_frequency_hz) const {
            // Formula taken from http://www.hyperphysics.gsu.edu/hbase/Waves/string.html
            return 4 * nominal_weight_kg_m * nominal_length_m * nominal_length_m * resonant_frequency_hz * resonant_frequency_hz;
        }

        /// Calculates reonant frequency from the tension
        /// \returns resonant frequency (in Hz)
        constexpr float tension_to_resonant_frequency(float tension_n) const {
            return sqrt(tension_n / (4 * nominal_weight_kg_m * nominal_length_m * nominal_length_m));
        }
    };

    std::array<BeltSystemParameters, belt_system_count> belt_system;
};

#if PRINTER_IS_PRUSA_XL()
static constexpr PrinterBeltParameters printer_belt_parameters {
    .belt_system = {
        PrinterBeltParameters::BeltSystemParameters {
            .measurement_pos = { .x = 342, .y = 110, .z = 10 },
            .axis_flags = STEP_EVENT_FLAG_STEP_X | STEP_EVENT_FLAG_STEP_Y | STEP_EVENT_FLAG_Y_DIR, // Vibrate the toolhead front and back
            .nominal_length_m = 0.395f,
            .nominal_weight_kg_m = 0.007569f,
            .target_tension_force_n = 18,
            .target_tension_tolerance_n = 1,
            .belt_tuning_params = {
                .excitation_amplitude_m_func = MeasureBeltTensionSpecificParams::linearly_varying_amplitude<50.0f, 95.0f, 0.00005f, 0.00009f>,
                .start_frequency_hz = 50,
                .end_frequency_hz = 95,
                .frequency_step_hz = 0.5,
                .excitation_cycles = 50,
                .wait_cycles = 1,
                .measurement_cycles = 30,
            },
        },
    },
};

#elif PRINTER_IS_PRUSA_iX()
static constexpr PrinterBeltParameters printer_belt_parameters {
    .belt_system = {
        PrinterBeltParameters::BeltSystemParameters {
            .measurement_pos = { .x = 257, .y = 8, .z = 10 },
            .axis_flags = STEP_EVENT_FLAG_STEP_X | STEP_EVENT_FLAG_STEP_Y | STEP_EVENT_FLAG_Y_DIR, // Vibrate the toolhead front and back
            .nominal_length_m = 0.300f,
            .nominal_weight_kg_m = 0.007569f,
            .target_tension_force_n = 18,
            .target_tension_tolerance_n = 1,
            .belt_tuning_params = {
                .excitation_amplitude_m_func = MeasureBeltTensionSpecificParams::linearly_varying_amplitude<75.0f, 105.0f, 0.00007f, 0.00009f>,
                .start_frequency_hz = 75,
                .end_frequency_hz = 105,
                .frequency_step_hz = 0.5,
                .excitation_cycles = 40,
                .wait_cycles = 1,
                .measurement_cycles = 30,
            },

        },
    },
};

#else
    #error Mising belt tensioning parameters for the printer
#endif
