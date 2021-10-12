/*
 * Hardware Abstraction Layer (HAL)
 * 
 * Copyright (C) 2018 Khalid Baheyeldin
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#if defined(__AVR_ATmega2560__)
  #define MCU_STR "Mega2560"
  #include "HAL_Mega2560.h"

#elif defined(__MK64FX512__)
  // Teensy 3.5
  #define MCU_STR "Teensy3.5"
  #include "HAL_Teensy_3.5.h"

#elif defined(__MK66FX1M0__)
  // Teensy 3.6
  #define MCU_STR "Teensy3.6"
  #include "HAL_Teensy_3.6.h"

#elif defined(ARDUINO_TEENSY41)
  // Teensy 4.1
  #define MCU_STR "Teensy4.1"
  #include "HAL_Teensy_4.1.h"

#else
  // Generic
  #warning "Unknown Platform! If this is a new platform, it would probably do best with a new HAL designed for it."
  #define MCU_STR "Generic (Unknown)"
  #include "HAL_MISC.h"  
#endif

// create null decoration for non-ESP processors
#ifndef IRAM_ATTR
  #define IRAM_ATTR
#endif

#ifndef ICACHE_RAM_ATTR
  #define ICACHE_RAM_ATTR
#endif

#ifndef FPSTR
  #define FPSTR
#endif

#ifdef HAL_FRACTIONAL_SEC
  #define FRACTIONAL_SEC  HAL_FRACTIONAL_SEC
#else
  #define FRACTIONAL_SEC  100
#endif
