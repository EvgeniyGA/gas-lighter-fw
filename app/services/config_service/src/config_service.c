#include "config_service.h"
#include "task.h"
#include "queue.h"
#include <string.h> 
#include "fatfs.h"
#include "stdlib.h"
#include "stdint.h"

#define STORAGE_TASK_STACK_SIZE         (configMINIMAL_STACK_SIZE * 2)
#define STORAGE_TASK_PRIORITY           (configMAX_PRIORITIES - 5)

#define STORAGE_QUEUE_LEN               (10)

typedef enum{
    CONFIG_DATA_TYPE_INT,
    CONFIG_DATA_TYPE_FLOAT
}config_data_type_e;

static StaticTask_t config_tx_task_def;
static TaskHandle_t config_tx_task_handle;
static QueueHandle_t storage_tx_queue_handle;
static StaticQueue_t storage_tx_queue_def;
static SemaphoreHandle_t fs_mutex = NULL;

typedef struct{
    char* name;
    uint8_t* data;
    uint8_t data_type;
    uint8_t elements_size;
    uint8_t element_count;
    uint8_t result;
}storage_service_msg_tx_t;

uint8_t config_save(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count){
    storage_service_msg_tx_t msg;
    msg.name = name;
    msg.data = data;
    msg.elements_size = element_size;
    msg.element_count = element_count;
    msg.data_type = CONFIG_DATA_TYPE_INT;
    msg.result = -1;
    if(xQueueSendToBack(storage_tx_queue_handle, &msg, portMAX_DELAY) != pdPASS){
        return -1;
    }
    return 0;
}

uint8_t config_save_float(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count){
    storage_service_msg_tx_t msg;
    msg.name = name;
    msg.data = data;
    msg.elements_size = element_size;
    msg.element_count = element_count;
    msg.data_type = CONFIG_DATA_TYPE_FLOAT;
    msg.result = -1;
    if(xQueueSendToBack(storage_tx_queue_handle, &msg, portMAX_DELAY) != pdPASS){
        return -1;
    }
    return 0;
}

uint8_t config_load(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count){
    uint8_t res = -1;
    int loaded_val = 0;
    uint16_t i = 0;
    char* endptr;
    char buf[20];
    xSemaphoreTake(fs_mutex, portMAX_DELAY);
    if(f_open(&file, name, FA_READ) == FR_OK){
        while(element_count--){
            f_gets(buf, sizeof(buf), &file);
            loaded_val = strtol((const char*)buf, &endptr, 10);
            memcpy((void*)data + (i * element_size), &loaded_val, element_size);
            i++;
        }
        if(endptr != buf){
            res = 0;
        }
        f_close(&file);
    }
    xSemaphoreGive(fs_mutex);
    return res;
}

uint8_t config_load_float(char* name, float* data, uint8_t element_size, uint8_t element_count){
    uint8_t res = -1;
    float loaded_val = 0;
    uint16_t i = 0;
    char* endptr;
    char buf[20];
    xSemaphoreTake(fs_mutex, portMAX_DELAY);
    if(f_open(&file, name, FA_READ) == FR_OK){
        while(element_count--){
            f_gets(buf, sizeof(buf), &file);
            loaded_val = strtof((const char*)buf, &endptr);
            memcpy((void*)data + (i * element_size), &loaded_val, element_size);
            res = 0;
            i++;
        }
        if(endptr != buf){
            res = 0;
        }
        f_close(&file);
    }
    xSemaphoreGive(fs_mutex);
    return res;
}

SemaphoreHandle_t storage_get_fs_mutex(void){
    return fs_mutex;
}

void config_service_tx_task(void* param){
    static uint8_t storage_queue_buffer[STORAGE_QUEUE_LEN * sizeof(storage_service_msg_tx_t)];
    static storage_service_msg_tx_t msg;
    static FRESULT res;
    static uint32_t byteswritten, len;
    char buf[20];
    storage_tx_queue_handle = xQueueCreateStatic(STORAGE_QUEUE_LEN, sizeof(storage_service_msg_tx_t), storage_queue_buffer, &storage_tx_queue_def);
    while(1){
        if(xQueueReceive(storage_tx_queue_handle, &msg, portMAX_DELAY)){
            xSemaphoreTake(fs_mutex, portMAX_DELAY);

            uint16_t i = 0;
            if(f_open(&file, msg.name, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){
                while(msg.element_count--){
                    
                    if(msg.data_type == CONFIG_DATA_TYPE_INT){
                        int tmp = 0;
                        memcpy((void*)&tmp, msg.data + ((i++) * msg.elements_size), msg.elements_size);
                        len = snprintf(buf, sizeof(buf), "%d\n", tmp);
                    }
                    else if(msg.data_type == CONFIG_DATA_TYPE_FLOAT){
                        float tmp = 0;
                        memcpy((void*)&tmp, msg.data + ((i++) * msg.elements_size), msg.elements_size);
                        len = snprintf(buf, sizeof(buf) - 1, "%.3f\n", tmp);    
                    }
                    
                    res = f_write(&file, buf, len, (void*)&byteswritten);
                    if((byteswritten != 0) && (res == FR_OK)){
                        msg.result = 0;
                    }
                }
                f_close(&file);
            }
            
            xSemaphoreGive(fs_mutex);
        }
    }
}

uint8_t config_service_init(void){
    static StackType_t storage_stack_tx[STORAGE_TASK_STACK_SIZE];
    static StaticSemaphore_t mutex_def;
    fs_mutex = xSemaphoreCreateMutexStatic(&mutex_def);
    config_tx_task_handle = xTaskCreateStatic(config_service_tx_task, "config_tx", STORAGE_TASK_STACK_SIZE,
                NULL, STORAGE_TASK_PRIORITY, storage_stack_tx, &config_tx_task_def);
    return 0;
}

void config_set_readonly(uint8_t event){
//todo
}