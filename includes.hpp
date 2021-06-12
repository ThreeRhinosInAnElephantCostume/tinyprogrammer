    # This file is part of tinyavrprogrammer.

    # tinyavrprogrammer is free software: you can redistribute it and/or modify
    # it under the terms of the GNU General Public License as published by
    # the Free Software Foundation, version 3.

    # tinyavrprogrammer is distributed in the hope that it will be useful,
    # but WITHOUT ANY WARRANTY; without even the implied warranty of
    # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    # GNU General Public License for more details.

    # You should have received a copy of the GNU General Public License
    # along with tinyavrprogrammer.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <stdio.h>
#include <locale>
#include <math.h>
#include <string.h>
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <functional>
#include <cstring>
#include <unordered_map>
#include <memory>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "pico/float.h"
#include "pico/double.h"
#include "pico/mutex.h"
#include "pico/critical_section.h"

#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/structs/pll.h"
#include "hardware/structs/clocks.h"
#include "hardware/pwm.h"
#include "hardware/watchdog.h"
#include "hardware/pio.h"

#include "tusb.h"
#include "tusb_config.h"
#include "bsp/board.h"

//#include "hvp.pio.h"