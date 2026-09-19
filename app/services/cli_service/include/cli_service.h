#ifndef _CLI_COMMANDS
#define _CLI_COMMANDS

#ifdef __cplusplus
extern "C" {
#endif

int debug_console_getchar(void);
int print_raw(const char *str);
void cli_service_init(void);

#ifdef __cplusplus
}
#endif

#endif