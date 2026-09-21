#include "dma_driver.h"
#include "fmc.h"
#include "dma.h"

void dma_send_data_to_fsmc(int* buf, uint32_t size){
    HAL_DMA_Start(&hdma_memtomem_dma2_stream0, (uint32_t)buf, (uint32_t)(0x60000000), size);
    HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream0, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
}
