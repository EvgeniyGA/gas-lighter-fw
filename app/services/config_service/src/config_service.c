#include "config_service.h"
#include "task.h"
#include "queue.h"
#include <string.h> 
#include "fatfs.h"

#define STORAGE_TASK_STACK_SIZE         (configMINIMAL_STACK_SIZE * 8)
#define STORAGE_TASK_PRIORITY           (configMAX_PRIORITIES - 5)

#define STORAGE_QUEUE_LEN               (1)
#define STORAGE_MAX_SAVING_SIZE         (100)

static StaticTask_t config_tx_task_def, config_rx_task_def;
static TaskHandle_t config_tx_task_handle, config_rx_task_handle;
static QueueHandle_t storage_rx_queue_handle, storage_tx_queue_handle;
static StaticQueue_t storage_rx_queue_def, storage_tx_queue_def;
SemaphoreHandle_t fs_mutex = NULL;

static uint8_t massive_for_saving[STORAGE_MAX_SAVING_SIZE];

typedef enum{
    STORAGE_MSG_SAVE_INT = 0,
    STORAGE_MSG_LOAD_INT,
    STORAGE_MSG_SAVE_MAS,
    STORAGE_MSG_LOAD_MAS,
    STORAGE_MSG_MAX_OPT
}storage_service_msg_e;

typedef struct{
    char* name;
    uint8_t type;
    uint8_t* data;
    uint8_t elements_size;
    uint8_t element_count;
    uint8_t result;
}storage_service_msg_tx_t;

typedef struct{
    char* name;
    uint8_t type;
    uint8_t* data;
    uint8_t element_size;
    uint8_t element_count;
    SemaphoreHandle_t done;
    uint8_t result;
}storage_service_msg_rx_t;

uint8_t config_save_mas(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count){
    static storage_service_msg_tx_t msg;
    msg.name = name;
    msg.type = STORAGE_MSG_SAVE_MAS;
    msg.data = data;
    msg.elements_size = element_size;
    msg.element_count = element_count;
    msg.result = -1;
    if(xQueueSendToBack(storage_tx_queue_handle, &msg, portMAX_DELAY) != pdPASS){
        return -1;
    }
    return 0;
}

uint8_t config_save_int(char* name, uint32_t data){
    static storage_service_msg_tx_t msg;
    msg.name = name;
    msg.type = STORAGE_MSG_SAVE_INT;
    msg.data = massive_for_saving;
    msg.elements_size = sizeof(data);//todo
    msg.element_count = 1;
    msg.result = -1;
    memcpy(msg.data, &data, msg.elements_size);

    if(xQueueSendToBack(storage_tx_queue_handle, &msg, portMAX_DELAY) != pdPASS){
        return -1;
    }
    return 0;
}

uint8_t config_load_mas(char* name, uint8_t* data, uint8_t element_size, uint8_t element_count){
    static storage_service_msg_rx_t msg;
    msg.name = name;
    msg.type = STORAGE_MSG_LOAD_INT;
    msg.data = (uint8_t*)data;
    msg.element_size = element_size;
    msg.element_count = element_count;
    msg.done = xSemaphoreCreateBinary();
    msg.result = -1;
    if(xQueueSendToBack(storage_rx_queue_handle, &msg, portMAX_DELAY) != pdPASS){
        vSemaphoreDelete(msg.done);
        return -1;
    }
    if(xSemaphoreTake(msg.done, portMAX_DELAY)){
        vSemaphoreDelete(msg.done);
        return 0;
    }
    vSemaphoreDelete(msg.done);
    return -1;
}

uint8_t config_load_int(char* name, uint32_t* data){
    static storage_service_msg_rx_t msg;
    msg.name = name;
    msg.type = STORAGE_MSG_LOAD_INT;
    msg.data = (uint8_t*)data;
    msg.element_size = sizeof(*data);//todo
    msg.element_count = 1;
    msg.done = xSemaphoreCreateBinary();
    msg.result = -1;
    if(xQueueSendToBack(storage_rx_queue_handle, &msg, portMAX_DELAY) != pdPASS){
        vSemaphoreDelete(msg.done);
        return -1;
    }
    if(xSemaphoreTake(msg.done, portMAX_DELAY)){
        vSemaphoreDelete(msg.done);
        return 0;
    }
    vSemaphoreDelete(msg.done);
    return -1;
}

void config_set_readonly(uint8_t event){

}

SemaphoreHandle_t storage_get_fs_mutex(void){
    return fs_mutex;
}

static FIL fil;
static char buf[10];

void config_service_tx_task(void* param){
    static uint8_t storage_queue_buffer[STORAGE_QUEUE_LEN * sizeof(storage_service_msg_tx_t)];
    storage_service_msg_tx_t msg;
    static FRESULT res;
    static uint32_t byteswritten, len;
    storage_tx_queue_handle = xQueueCreateStatic(STORAGE_QUEUE_LEN, sizeof(storage_service_msg_tx_t), storage_queue_buffer, &storage_tx_queue_def);
    while(1){
        if(xQueueReceive(storage_tx_queue_handle, &msg, portMAX_DELAY)){
            xSemaphoreTake(fs_mutex, portMAX_DELAY);
            //if(msg.type == STORAGE_MSG_SAVE_INT)
            {
                int tmp = 0;
                uint16_t i = 0;
                if(f_open(&fil, msg.name, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){
                    while(msg.element_count--){
                        memcpy(&tmp, msg.data + (i * msg.elements_size), msg.elements_size);
                        i++;
                        portENTER_CRITICAL();
                        len = snprintf(buf, sizeof(buf), "%d", tmp);//todo
                        portEXIT_CRITICAL();
                        buf[len] = 0;
                        res = f_write(&fil, buf, len + 1, (void*)&byteswritten);
                        if((byteswritten != 0) && (res == FR_OK)){
                            msg.result = 0;
                        }
                    }
                    f_close(&fil);
                }
            }
            //else if(msg.type == STORAGE_MSG_SAVE_MAS){
//todo
            //}
            xSemaphoreGive(fs_mutex);
        }
    }
}

void config_service_rx_task(void* param){
    static uint8_t storage_queue_buffer[STORAGE_QUEUE_LEN * sizeof(storage_service_msg_rx_t)];
    storage_service_msg_rx_t msg;
    storage_rx_queue_handle = xQueueCreateStatic(STORAGE_QUEUE_LEN, sizeof(storage_service_msg_rx_t), storage_queue_buffer, &storage_rx_queue_def);
    static UINT br;
    
    while(1){
        if(xQueueReceive(storage_rx_queue_handle, &msg, portMAX_DELAY)){
            xSemaphoreTake(fs_mutex, portMAX_DELAY);

            //if(msg.type == STORAGE_MSG_LOAD_INT)
            {
                int loaded_val = 0;
                uint16_t i = 0;
                if(f_open(&file, msg.name, FA_READ) == FR_OK){
                    while(msg.element_count--){
                        f_read(&file, buf, 4/*msg.element_size*/, &br);
                        //buf[br] = '\0';
                        loaded_val = atoi(buf);
                        memcpy(msg.data + i * msg.element_size, &loaded_val, sizeof(loaded_val));
                        msg.result = 0;
                        i++;
                    }
                    f_close(&file);
                }
            }

            xSemaphoreGive(fs_mutex);
            xSemaphoreGive(msg.done);
        }
    }
}

uint8_t config_service_init(void){
    static StackType_t storage_stack_tx[STORAGE_TASK_STACK_SIZE];
    static StackType_t storage_stack_rx[STORAGE_TASK_STACK_SIZE];
    static StaticSemaphore_t mutex_def;
    fs_mutex = xSemaphoreCreateMutexStatic(&mutex_def);//xSemaphoreCreateMutex();
    config_tx_task_handle = xTaskCreateStatic(config_service_tx_task, "config_tx", STORAGE_TASK_STACK_SIZE,
                NULL, STORAGE_TASK_PRIORITY, storage_stack_tx, &config_tx_task_def);
    config_rx_task_handle = xTaskCreateStatic(config_service_rx_task, "config_rx", STORAGE_TASK_STACK_SIZE,
                NULL, STORAGE_TASK_PRIORITY, storage_stack_rx, &config_rx_task_def);
    return 0;
}

