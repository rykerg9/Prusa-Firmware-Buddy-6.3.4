/**
 * @file MItem_love_board.hpp
 */
#pragma once

#include "WindowMenuItems.hpp"
#include "i18n.h"
#include "otp_types.hpp"

class MI_INFO_SERIAL_NUM_LOVEBOARD : public WiInfo<28> {
    static constexpr const char *const label = N_("Love Board");

public:
    MI_INFO_SERIAL_NUM_LOVEBOARD();
};
