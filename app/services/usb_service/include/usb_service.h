#ifndef _USB_SERVICE_H
#define _USB_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef void(*usb_callback_t)(void);

typedef struct{
    usb_callback_t mounted;
    usb_callback_t unmounted;
}usb_device_config_t;

void usb_device_init(usb_device_config_t*);
void usb_cdc_init(void);

#ifdef __cplusplus
}
#endif

#endif
