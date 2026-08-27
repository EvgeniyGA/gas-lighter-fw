/*
 * wave_runner.c
 *
 *  Created on: Mar 31, 2026
 *      Author: evgeny
 */

#ifndef INC_WAVE_STARTER_H_
#define INC_WAVE_STARTER_H_

#include <stdint.h>
#include "wave_gen.h"
#include "app.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WAVE_TIME_RESOLUTION		(MAIN_TIME_RESOLUTION)

int wave_starter_init(waveGenConfig_s* config);
int wave_starter_run(waveGenConfig_s* config);

#ifdef __cplusplus
}
#endif

#endif /* INC_WAVE_RUNNER_C_ */
