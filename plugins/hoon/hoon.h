#undef CMD_INC_FILE
#define CMD_INC_FILE plugins/hoon/hoon

#if !defined(HOON_NVME) || defined(CMD_HEADER_MULTI_READ)
//#ifndef HOON_NVME
#define HOON_NVME

#include "cmd.h"
#include "plugin.h"


PLUGIN(NAME("hoon", "hoon NVME plugin", NVME_VERSION),
    COMMAND_LIST(
			ENTRY("flash-type",     "change flash nand type",   change_flash_type)
            ENTRY("print-zones",    "print zone information",   print_nand)
            ENTRY("config-control", "change config parameter",  h_config_control)
            ENTRY("set-debug",      "set debug mode",           set_debug_mode)
    )
);

// typedef struct admin_config {
//     __u8	opcode;
//     __u8	flags;
//     __u16	rsvd;
//     __u32	namespace_id;
//     __u32	data_len;
//     __u32	metadata_len;
//     __u32	timeout;
//     __u32	cdw2;
//     __u32	cdw3;
//     __u32	cdw10;
//     __u32	cdw11;
//     __u32	cdw12;
//     __u32	cdw13;
//     __u32	cdw14;
//     __u32	cdw15;
//     char	*input_file;
//     char	*metadata;
//     int	raw_binary;
//     int	show_command;
//     int	dry_run;
//     int	read;
//     int	write;
//     __u8	prefill;
//     int	latency;
// } admin_config;

#endif
#include "define_cmd.h"