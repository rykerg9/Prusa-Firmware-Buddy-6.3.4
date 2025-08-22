/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#define REVERSE_TEMP_SENSOR_RANGE

// PT1000 with HX717 ADC convertor
const short temptable_21[][2] PROGMEM = {
  { OV(497),   0 },
  { OV(513),  10 },
  { OV(529),  20 },
  { OV(544),  30 },
  { OV(559),  40 },
  { OV(574),  50 },
  { OV(590),  60 },
  { OV(605),  70 },
  { OV(621),  80 },
  { OV(636),  90 },
  { OV(652), 100 },
  { OV(667), 110 },
  { OV(683), 120 },
  { OV(698), 130 },
  { OV(714), 140 },
  { OV(729), 150 },
  { OV(745), 160 },
  { OV(760), 170 },
  { OV(776), 180 },
  { OV(792), 190 },
  { OV(807), 200 },
  { OV(823), 210 },
  { OV(838), 220 },
  { OV(854), 230 },
  { OV(869), 240 },
  { OV(885), 250 },
  { OV(900), 260 },
  { OV(916), 270 },
  { OV(931), 280 },
  { OV(947), 290 },
  { OV(962), 300 },
  { OV(978), 310 },
  { OV(993), 320 },
  { OV(1009),330 },
  { OV(1024),340 },
  { OV(1040),350 }
};
