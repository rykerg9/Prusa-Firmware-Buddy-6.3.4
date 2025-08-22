#include "warning_type.hpp"

PhasesWarning warning_type_phase(WarningType warning) {
    switch (warning) {

    default:
        // Intentionally returning Warning by default - only a few warnings use different phase
        return PhasesWarning::Warning;

    case WarningType::MetricsConfigChangePrompt:
        return PhasesWarning::MetricsConfigChangePrompt;

#if HAS_BED_PROBE
    case WarningType::ProbingFailed:
        return PhasesWarning::ProbingFailed;
#endif

    case WarningType::FilamentSensorStuckHelp:
        return PhasesWarning::FilamentSensorStuckHelp;

#if HAS_MMU2()
    case WarningType::FilamentSensorStuckHelpMMU:
        return PhasesWarning::FilamentSensorStuckHelpMMU;
#endif

#if HAS_LOADCELL() && ENABLED(PROBE_CLEANUP_SUPPORT)
    case WarningType::NozzleCleaningFailed:
        return PhasesWarning::NozzleCleaningFailed;
#endif

#if HAS_UNEVEN_BED_PROMPT()
    case WarningType::BedUnevenAlignmentPrompt:
        return PhasesWarning::BedUnevenAlignmentPrompt;
#endif

#if XL_ENCLOSURE_SUPPORT()
    case WarningType::EnclosureFilterExpiration:
        return PhasesWarning::EnclosureFilterExpiration;
#endif

#if PRINTER_IS_PRUSA_COREONE()
    case WarningType::OpenChamberVents:
        return PhasesWarning::ChamberVents;
    case WarningType::CloseChamberVents:
        return PhasesWarning::ChamberVents;
#endif

#if HAS_EMERGENCY_STOP()
    case WarningType::DoorOpen:
        return PhasesWarning::DoorOpen;
#endif

#if HAS_CHAMBER_API()
    case WarningType::FailedToReachChamberTemperature:
        return PhasesWarning::FailedToReachChamberTemperature;
#endif

#if HAS_ILI9488_DISPLAY()
    case WarningType::DisplayProblemDetected:
        return PhasesWarning::DisplayProblemDetected;
#endif

        //
    }
}

constexpr uint32_t warning_lifespan_sec_constexpr(WarningType type) {
    switch (type) {
#if PRINTER_IS_PRUSA_COREONE()
    case WarningType::OpenChamberVents:
    case WarningType::CloseChamberVents:
        return 60;
#endif
    default:
        return uint32_t(-1); // Unlimited
    }
}

uint32_t warning_lifespan_sec(WarningType type) {
    return warning_lifespan_sec_constexpr(type);
}
