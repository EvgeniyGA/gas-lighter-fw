#include "cli_service.h"
#include "cli_commands.h"
#include "cli_commands_fs.h"

void cli_service_init(void){
    CLI_install_commands();
	CLI_install_commands_fs();
}