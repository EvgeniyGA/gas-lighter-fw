// device_api.h
#ifndef DEVICE_API_H
#define DEVICE_API_H

#ifdef __cplusplus
extern "C" {
#endif

// Объявляем функцию с префиксом, чтобы было понятно, откуда она, 
// и чтобы избежать коллизий имён в глобальном пространстве C.
void device_start_generation(void);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_API_H