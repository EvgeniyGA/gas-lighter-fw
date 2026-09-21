#include "device.h"
#include "config_service.h"
#include <cstdio>
#include "config_wrapper.hpp"
#include <algorithm>
#include "gpio_driver.h"
#include "dma_driver.h"
#include "version.h"
#include "version_check.h"
#include "timers.h"

namespace device{

DeviceConfig config;

uint8_t config_step = 0;
inline constexpr uint8_t config_steps = LED_MODE_COUNT - 1;//???
TimerHandle_t fire_away_timer;

void heartbit_callback(void){
}

void start_generation(void){
  auto period = config.getConfig(config_step).value[0];
  if(period != 0){
    xTimerChangePeriod(fire_away_timer, pdMS_TO_TICKS(period), 0);
  }
  else{
    //todo
  }
  xTimerStart(fire_away_timer, 0);
}

void vRunTimerCallback( TimerHandle_t xTimer ) {
  gpio_led_go_change_state(GPIO_LED_ON);
  if(config.getConfig(config_step).value[0] == 0){
    xTimerStop(fire_away_timer, 0);
  }
  dma_send_data_to_fsmc(
    reinterpret_cast<int*>(config.getConfig(config_step).value.data() + 1), 
    config.getConfig(config_step).value.size() - 1
  );
  gpio_led_go_change_state(GPIO_LED_OFF);
}

void refresh_task(void* param){
  if(is_button_mode_pressed()){
    gpio_led_status_change_state(GPIO_LED_ON);
    config_step = (config_step < config_steps) ? (config_step + 1) : 0;
    xTimerStop(fire_away_timer, 0);
  }

  do{
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  while(is_button_mode_pressed());
  gpio_led_status_change_state(GPIO_LED_OFF);

  if(is_button_go_pressed() ){
    start_generation(); 
    do{
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }while(is_button_go_pressed());
  }

  vTaskDelay(10 / portTICK_PERIOD_MS);
}

void init(void){
  fire_away_timer = xTimerCreate("go_timer", pdMS_TO_TICKS(100), pdTRUE, 0, vRunTimerCallback);
  xTimerStart(fire_away_timer, 0);
  xTaskCreate(refresh_task, "refresh_task", configMINIMAL_STACK_SIZE * 2, NULL, 0, NULL);
}

}
