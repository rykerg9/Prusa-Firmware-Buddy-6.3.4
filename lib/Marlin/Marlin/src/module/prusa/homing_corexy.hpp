/*
 * CoreXY precise homing refinement
 * TODO: @wavexx: Add some documentation
 */

#pragma once
#include "../../core/types.h"

// convert raw AB steps to XY mm
void corexy_ab_to_xy(const xy_long_t &steps, xy_pos_t &mm);

// convert raw AB steps to XY mm, filling others from current state
void corexy_ab_to_xyze(const xy_long_t &steps, xyze_pos_t &mm);

enum class CoreXYCalibrationMode {
    on_demand, // Allow automatic calibration
    force, // Force re-calibration
    disallow // Disallow re-calibration
};

/**
 * @brief Rehome XY axes without refinement or retries
 * @param fr_mm_s Homing feedrate
 * @return true on success
 */
bool corexy_rehome_xy(const float fr_mm_s);

/**
 * @brief Refine home origin precisely on core-XY.
 * @warning Must be called *at* home position
 * @param fr_mm_s Homing (and service moves) feedrate
 * @param can_calibrate Allow origin self-calibration to occur
 * @return true on success
 */
bool corexy_home_refine(const float fr_mm_s, CoreXYCalibrationMode mode);

/**
 * @brief Return the calibration status of the home origin
 * @return true if already calibrated
 */
bool corexy_home_is_calibrated();

/**
 * @brief Return the validity of the last refinement attempt
 * @return true if the current home is unstable and requires re-calibration
 */
bool corexy_home_is_unstable();

#if HAS_TRINAMIC && defined(XY_HOMING_MEASURE_SENS_MIN)
/**
 * @brief Calibrate homing sensitivity on TMC
 * @warning Must be called *at* home position
 * @param fr_mm_s Homing (and service moves) feedrate
 * @return true on success
 */
bool corexy_sens_calibrate(const float fr_mm_s);

/**
 * @brief Return the TMC sensitivity calibration status
 * @return true if already calibrated
 */
bool corexy_sens_is_calibrated();
#endif
