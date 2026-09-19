#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*status_heartbit_t)(void);

uint8_t status_service_init(status_heartbit_t);

#ifdef __cplusplus
}
#endif
