#undef CMD_INC_FILE
#define CMD_INC_FILE plugins/hoon/hoon

#if !defined(HOON_NVME) || defined(CMD_HEADER_MULTI_READ)
#define HOON_NVME

#include "cmd.h"
#include "plugin.h"

PLUGIN(NAME("hoon", "hoon NVME plugin", NVME_VERSION),
    COMMAND_LIST(
			ENTRY("flash-type", "change flash nand type", change_flash_type)
            ENTRY("print-zone", "print zone information", print_nand)
    )
);

#endif

#include "define_cmd.h"
