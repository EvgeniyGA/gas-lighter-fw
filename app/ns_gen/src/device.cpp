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
#include "FreeRTOS.h"
#include "task.h"

namespace device{

DeviceConfig config;

uint8_t config_step = 0;
inline constexpr uint8_t config_steps = LED_MODE_COUNT - 1;//???
TimerHandle_t fire_away_timer, xLedOffTimer;

void heartbit_callback(void){
}

uint8_t is_started = 0;

void update_period(){
  auto period = config.getConfig(config_step).value[0];
  if(period != 0){
    xTimerChangePeriod(fire_away_timer, pdMS_TO_TICKS(period), 0);
  }
  else{
    xTimerChangePeriod(fire_away_timer, 1, 0);
  }
}

void start_generation(void){
  auto period = config.getConfig(config_step).value[0];
  if((period != 0) && (is_started == 0)){
    is_started = 1;
  }
  else{
    is_started = 0;
  }
  if(is_started || (period == 0)){
    xTimerReset(fire_away_timer, 0);
  }
}

void vRunTimerCallback( TimerHandle_t xTimer ) {
  auto period = config.getConfig(config_step).value[0];
  if(is_started || (period == 0)){
    gpio_led_go_change_state(GPIO_LED_ON);
    if(period != 0){
      xTimerReset(fire_away_timer, 0);
    }
    dma_send_data_to_fsmc(
      reinterpret_cast<int*>(config.getConfig(config_step).value.data() + 1), 
      config.getConfig(config_step).value.size() - 1
    );
    xTimerReset(xLedOffTimer, portMAX_DELAY);
  }
}

void refresh_task(void* param){
  gpio_led_mode_change_state(config_step, GPIO_LED_ON);
  while(1){
    if(is_button_mode_pressed()){
      gpio_led_status_change_state(GPIO_LED_ON);
      gpio_led_mode_change_state(config_step, GPIO_LED_OFF);
      config_step = (config_step < config_steps) ? (config_step + 1) : 0;
      gpio_led_mode_change_state(config_step, GPIO_LED_ON);
      update_period();
    }

    while(is_button_mode_pressed()){
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    gpio_led_status_change_state(GPIO_LED_OFF);

    if(is_button_go_pressed() ){
      start_generation(); 
      while(is_button_go_pressed()){
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

#define   refresh_task_stack_size     configMINIMAL_STACK_SIZE*2

StaticTask_t refresh_task_taskdef;

void init(void){
  static StackType_t refresh_task_stack[refresh_task_stack_size];
  fire_away_timer = xTimerCreate("go_timer", pdMS_TO_TICKS(100), pdFALSE, (void*)0, vRunTimerCallback);
  xLedOffTimer = xTimerCreate("led_off_timer", pdMS_TO_TICKS(200), pdFALSE, (void*)0,
    [](TimerHandle_t xTimer){ gpio_led_go_change_state(GPIO_LED_OFF); }
  );
  update_period();
  xTaskCreateStatic(refresh_task, "refresh_task", refresh_task_stack_size, 
    NULL, configMAX_PRIORITIES - 3, refresh_task_stack, &refresh_task_taskdef);
}

}
