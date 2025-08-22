#pragma once

#include <core/types.h>
#include <common/enum_array.hpp>

#include <option/has_wastebin.h>

namespace mapi {

enum class ZAction : uint16_t {
    move_to_at_least,
    absolute_move,
    relative_move,
    relative_move_skip_xy, // TODO not implemented
    no_move,
    _cnt
};

enum class ParkPosition : uint8_t {
    park,
    purge,
    load,
    _cnt,
};

static constexpr EnumArray<ParkPosition, xyz_pos_t, ParkPosition::_cnt> park_positions {
    { ParkPosition::park, xyz_pos_t({ { XYZ_NOZZLE_PARK_POINT } }) },
#if HAS_WASTEBIN()
        { ParkPosition::purge, xyz_pos_t({ X_WASTEBIN_POINT, Y_WASTEBIN_POINT, Z_AXIS_LOAD_POS }) },
#else
        { ParkPosition::purge, xyz_pos_t({ X_AXIS_LOAD_POS, Y_AXIS_LOAD_POS, Z_AXIS_LOAD_POS }) },
#endif
        { ParkPosition::load, xyz_pos_t({ X_AXIS_LOAD_POS, Y_AXIS_LOAD_POS, Z_AXIS_LOAD_POS }) },
};

void park_move_with_conditional_home(const xyz_pos_t &park_position, ZAction z_action);

} // namespace mapi
