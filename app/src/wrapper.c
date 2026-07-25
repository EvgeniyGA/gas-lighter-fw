/*
 * wrapper.c
 *
 *  Created on: Jul 25, 2026
 *      Author: evgeny
 */
#include <stdio.h>
#include <stdarg.h>
#ifdef STM32F746xx
#include "stm32f7xx.h"
#elif defined STM32F407xx
#include "stm32f4xx.h"
#endif

int __real_printf(const char *fmt, ...);

int __wrap_printf(const char *fmt, ...) {
  unsigned int t = HAL_GetTick();
  va_list args;
  int count0, count1;
  count0 = __real_printf(">>%d.%d: ", t/1000, t%1000);
  va_start(args, fmt);
  count1 = vprintf(fmt, args);
  va_end(args);
  return count0+count1;
}
