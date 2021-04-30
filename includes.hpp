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

#include "tusb.h"
#include "tusb_config.h"
#include "bsp/board.h"