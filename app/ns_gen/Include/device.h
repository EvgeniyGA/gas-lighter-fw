#pragma once

#include <stdint.h>
#include <array>
#include "config_wrapper.hpp"

namespace device{

inline constexpr size_t calibrations_size = 100;

struct DeviceConfig{
    config::Item<uint32_t> ampl{"ampl.txt", 1u};
    config::Item<float> dac_set{"dac.txt", 32.23f};

    config::Item<std::array<int, calibrations_size>> config1{
		"conf1.txt", config::make_sequence_array<int, calibrations_size>()
	};
	
	using CalibrationType = decltype(config1);
	CalibrationType config2 {"conf2.txt", config::make_sequence_array<int, calibrations_size>()};
	CalibrationType config3 {"conf3.txt", config::make_sequence_array<int, calibrations_size>()};
	CalibrationType config4 {"conf4.txt", config::make_sequence_array<int, calibrations_size>()};
	CalibrationType config5 {"conf5.txt", config::make_sequence_array<int, calibrations_size>()};
	CalibrationType config6 {"conf6.txt", config::make_sequence_array<int, calibrations_size>()};

	CalibrationType& getConfig(size_t conf){
		CalibrationType* cal_ptrs[6] = {
			&config1, &config2, &config3, &config4, &config5, &config6};
		if(conf > 6){
			conf = 6;
		}
		return *cal_ptrs[conf];
	}

	template<typename T>
	void apply(T&& visitor){
		visitor(ampl);
		visitor(dac_set);
		visitor(config1);
		visitor(config2);
		visitor(config3);
		visitor(config4);
		visitor(config5);
		visitor(config6);
	}
};

extern DeviceConfig config;

void heartbit_callback(void);
void init(void);
void start_generation(void);

}
