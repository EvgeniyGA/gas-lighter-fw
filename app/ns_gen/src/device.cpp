#include "device.h"
#include "config_service.h"
#include <cstdio>
#include "config_wrapper.hpp"
#include <algorithm>
#include "gpio_driver.h"
#include "version.h"
#include "version_check.h"
#include "timers.h"

namespace device{

DeviceConfig config;

static uint8_t step = 0;

void heartbit_callback(void){
  gpio_led_mode_change_state(step, GPIO_LED_ON);
  device::step = (device::step < (LED_MODE_COUNT - 1)) ? (device::step + 1) : 0;
//  vTaskDelay(blink_interval_ms / portTICK_PERIOD_MS);////  vTaskDelay(pdMS_TO_TICKS(10000));
  if(is_button_mode_pressed()){
    gpio_led_status_change_state(GPIO_LED_ON);
  }
  else{
    gpio_led_status_change_state(GPIO_LED_OFF);
  }

  if(is_button_go_pressed()){
    //gpio_led_go_change_state(GPIO_LED_ON);
  }
  else{
    //gpio_led_go_change_state(GPIO_LED_OFF);
  }
}

void fire_away(void* param){
    for(const auto & i: config.config1.value){
        std::printf("%d\n\r", i);
    }
}

TimerHandle_t fire_away_timer;
uint8_t led_state = 0;
void vTimerCallback( TimerHandle_t xTimer ) {
    if(led_state){
        led_state = 0;
        gpio_led_go_change_state(GPIO_LED_ON);
        //HAL_DMA_Start(&hdma_memtomem_dma2_stream0, (uint32_t)buff_conf, (uint32_t)(0x60000000), sizeof(buff_conf));
        //HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream0, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
    }
    else{
        led_state = 1;
        gpio_led_go_change_state(GPIO_LED_OFF);
    }
}

void init(void){
    fire_away_timer = xTimerCreate("go_timer", pdMS_TO_TICKS(100), pdTRUE, 0, vTimerCallback);
    xTimerStart(fire_away_timer, 0);
//    xTimerChangePeriod(myTimer, pdMS_TO_TICKS(new_period_ms), 0);
}

}
