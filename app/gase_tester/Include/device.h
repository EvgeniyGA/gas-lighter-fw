#pragma once

#include <stdint.h>
#include <array>
#include "config_wrapper.hpp"

namespace device{

inline constexpr uint32_t MAIN_FREQENCY_HZ = 10000;

struct DeviceConfig{
    config::Item<uint32_t> main_freq{"freq.txt", 444u};
    config::Item<float> dac_set{"dac.txt", 32.23f};
    config::Item<std::array<int, 10>> calibration{"cal.txt", config::make_sequence_array<int, 10>()};

	template<typename T>
	void apply(T&& visitor){
		visitor(main_freq);
		visitor(dac_set);
		visitor(calibration);
	}
};

extern DeviceConfig config;

void heartbit_callback(void);
void init(void);

}

