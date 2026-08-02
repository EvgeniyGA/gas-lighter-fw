#ifndef _USB_SERVICE_H
#define _USB_SERVICE_H

#define USBD_STACK_SIZE    (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 4 : 2))
#define CDC_STACK_SIZE      (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 3 : 2))

void cdc_task(void *params);
void usb_device_task(void *param);
void msc_disk_init(void);

#endif
