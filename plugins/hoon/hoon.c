#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdbool.h>

#include "nvme.h"
#include "libnvme.h"
#include "plugin.h"
#include "linux/types.h"
#include "nvme-print.h"

#define CREATE_CMD
#include "hoon.h"


static const __u32 OP_SCT_STATUS = 0xE0;
static const __u32 OP_SCT_COMMAND_TRANSFER = 0xE0;
static const __u32 OP_SCT_DATA_TRANSFER = 0xE1;

static const __u32 DW10_SCT_STATUS_COMMAND = 0x0;
static const __u32 DW10_SCT_COMMAND_TRANSFER = 0x1;

static const __u32 DW11_SCT_STATUS_COMMAND = 0x0;
static const __u32 DW11_SCT_COMMAND_TRANSFER = 0x0;

static const __u16 INTERNAL_LOG_ACTION_CODE = 0xFFFB;
static const __u16 CURRENT_LOG_FUNCTION_CODE = 0x0001;
static const __u16 SAVED_LOG_FUNCTION_CODE = 0x0002;

/* A bitmask field for supported devices */
typedef enum {
	MASK_0    = 1 << 0,
	MASK_1    = 1 << 1,
	/*
	 * Future devices can use the remaining 31 bits from this field
	 * and should use 1 << 2, 1 << 3, etc.
	 */
	MASK_IGNORE = 0
} DeviceMask;


static int nvme_sct_op(int fd,  __u32 opcode, __u32 cdw10, __u32 cdw11, void* data, __u32 data_len )
{
	void  *metadata = NULL;
	const __u32 cdw2 = 0;
	const __u32 cdw3 = 0;
	const __u32 cdw12 = 0;
	const __u32 cdw13 = 0;
	const __u32 cdw14 = 0;
	const __u32 cdw15 = 0;
	const __u32 timeout = 0;
	const __u32 metadata_len = 0;
	const __u32 namespace_id = 0x0;
	const __u32 flags = 0;
	const __u32 rsvd = 0;
	int err = 0;

	__u32 result;
	err = nvme_admin_passthru(fd, opcode, flags, rsvd,
				namespace_id, cdw2, cdw3, cdw10,
				cdw11, cdw12, cdw13, cdw14, cdw15,
				data_len, data, metadata_len, metadata,
				timeout, &result);
	return err;
}

// func 2
static int nvme_send_admin_cmd(int fd, const uint64_t zone_nb, uint64_t select_zone)
{
	int err;
	void* data = NULL;
	size_t data_len = 512;
	unsigned char *status;

	if (posix_memalign(&data, getpagesize(), data_len)) return ENOMEM;

	memset(data, 0, data_len);
	//err = nvme_sct_op(fd, OP_SCT_STATUS, DW10_SCT_STATUS_COMMAND, DW11_SCT_STATUS_COMMAND, data, data_len);

	void  *metadata = NULL;
	const __u32 cdw2 = 0;
	const __u32 cdw3 = 0;
	const __u32 cdw12 = 0;
	const __u32 cdw13 = 0;
	const __u32 cdw14 = 0;
	const __u32 cdw15 = 0;
	const __u32 timeout = 0;
	const __u32 metadata_len = 0;
	const __u32 namespace_id = 0x0;
	const __u32 flags = 0;
	const __u32 rsvd = 0;

	__u32 result;
	err = nvme_admin_passthru(fd, OP_SCT_STATUS, flags, rsvd,
				namespace_id, cdw2, cdw3, DW10_SCT_STATUS_COMMAND,
				DW11_SCT_STATUS_COMMAND, cdw12, cdw13, cdw14, cdw15,
				data_len, data, metadata_len, metadata,
				timeout, &result);


	if (err) {
		fprintf(stderr, "%s: hoon SCT status failed :%d, opcode(0x%x)\n", __func__, err, OP_SCT_STATUS);
		goto end;
	}

end:
	if (data) free(data);
	return err;
}



// func 1
static int change_flash_type(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{
	/*
	int err, fd;
	char *desc = "SLC setting command";
	const char *zone_offset = "select zone";
	const char *set_slc_zone = "select slc";

	struct config {
		uint64_t zone_offset;
		uint64_t set_slc_zone;
	};

	struct config cfg = {
		.zone_offset = 0,
		.set_slc_zone = 0
	};

	OPT_ARGS(opts) = {
		OPT_UINT("zone_offset-file",	'z', &cfg.zone_offset, zone_offset),
		OPT_UINT("set_slc_zone",			'o', &cfg.set_slc_zone, set_slc_zone),
		OPT_END()
	};

	fd = parse_and_open(argc, argv, desc, opts);
	if (fd < 0) {
		fprintf(stderr,"%s: failed to parse arguments\n", __func__);
		return EINVAL;
	}

	err = nvme_send_admin_cmd(fd, cfg.zone_offset, cfg.set_slc_zone);
	if (err < 0)
		fprintf(stderr, "%s: setting SLC failed \n", __func__);
	if (err > 0)
		nvme_show_status(err);

	return err;
	*/
	const char *desc = "Hoon plugin command";

	const char *select_zone = "select zone";
	const char *select_type = "select flash type";
	
	int flags;
	int mode = S_IRUSR | S_IWUSR |S_IRGRP | S_IWGRP| S_IROTH;
	void *data = NULL, *mdata = NULL;
	int err = 0, dfd, mfd, fd;
	__u32 result;
	bool huge = false;
	const char *cmd_name = NULL;
	struct timeval start_time, end_time;

	struct config {
		__u8	opcode;
		__u8	flags;
		__u16	rsvd;
		__u32	namespace_id;
		__u32	data_len;
		__u32	metadata_len;
		__u32	timeout;
		__u32	cdw2;
		__u32	cdw3;
		__u32	cdw10;
		__u32	cdw11;
		__u32	cdw12;
		__u32	cdw13;
		__u32	cdw14;
		__u32	cdw15;
		char	*input_file;
		char	*metadata;
		int	raw_binary;
		int	show_command;
		int	dry_run;
		int	read;
		int	write;
		__u8	prefill;
		int	latency;
	};

	struct config cfg = {
		.opcode		= 0x89,
		.flags		= 0,
		.prefill	= 0,
		.rsvd		= 0,
		.namespace_id	= 0,
		.data_len	= 0,
		.metadata_len	= 0,
		.timeout	= 0,
		.cdw2		= 0,
		.cdw3		= 0,
		.cdw10		= 0,
		.cdw11		= 0,
		.cdw12		= 0,
		.cdw13		= 0,
		.cdw14		= 0,
		.cdw15		= 0,
		.input_file	= "",
		.metadata	= "",
		.raw_binary	= 0,
		.show_command	= 0,
		.dry_run	= 0,
		.read		= 0,
		.write		= 0,
		.latency	= 0,
	};

	OPT_ARGS(opts) = {
		OPT_UINT("zone",     'z', &cfg.cdw10,        select_zone),
		OPT_UINT("type",     't', &cfg.cdw11,        select_type),
		
		OPT_END()
	};

	err = fd = parse_and_open(argc, argv, desc, opts);
	if (fd < 0)
		goto ret;

	if (cfg.show_command || cfg.dry_run) {
		printf("opcode       : %02x\n", cfg.opcode);
		printf("flags        : %02x\n", cfg.flags);
		printf("rsvd1        : %04x\n", cfg.rsvd);
		printf("nsid         : %08x\n", cfg.namespace_id);
		printf("cdw2         : %08x\n", cfg.cdw2);
		printf("cdw3         : %08x\n", cfg.cdw3);
		printf("data_len     : %08x\n", cfg.data_len);
		printf("metadata_len : %08x\n", cfg.metadata_len);
		printf("addr         : %"PRIx64"\n", (uint64_t)(uintptr_t)data);
		printf("metadata     : %"PRIx64"\n", (uint64_t)(uintptr_t)mdata);
		printf("cdw10        : %08x\n", cfg.cdw10);
		printf("cdw11        : %08x\n", cfg.cdw11);
		printf("cdw12        : %08x\n", cfg.cdw12);
		printf("cdw13        : %08x\n", cfg.cdw13);
		printf("cdw14        : %08x\n", cfg.cdw14);
		printf("cdw15        : %08x\n", cfg.cdw15);
		printf("timeout_ms   : %08x\n", cfg.timeout);
	}
	if (cfg.dry_run)
		goto free_data;

	gettimeofday(&start_time, NULL);

	//if (admin)
		err = nvme_admin_passthru(fd, cfg.opcode, cfg.flags, cfg.rsvd,
				cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
				cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14,
				cfg.cdw15, cfg.data_len, data, cfg.metadata_len,
				mdata, cfg.timeout, &result);
	// else
	// 	err = nvme_io_passthru(fd, cfg.opcode, cfg.flags, cfg.rsvd,
	// 			cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
	// 			cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14,
	// 			cfg.cdw15, cfg.data_len, data, cfg.metadata_len,
	// 			mdata, cfg.timeout, &result);

	gettimeofday(&end_time, NULL);
	cmd_name = nvme_cmd_to_string(true, cfg.opcode);
	if (cfg.latency)
		printf("%s Command %s latency: %llu us\n",
			//admin ? "Admin": "IO",
			"Admin",
			strcmp(cmd_name, "Unknown") ? cmd_name: "Vendor Specific",
			elapsed_utime(start_time, end_time));

	if (err < 0)
		fprintf(stderr, "passthru: %s\n", nvme_strerror(errno));
	else if (err)
		nvme_show_status(err);
	else  {
		fprintf(stderr, "%s Command %s is Success and result: 0x%08x\n",
				"Admin",
				strcmp(cmd_name, "Unknown") ? cmd_name: "Vendor Specific",
				result);
		if (cfg.read && cfg.input_file) {
			if (write(dfd, (void *)data, cfg.data_len) < 0)
				perror("failed to write data buffer");
			if (cfg.metadata_len && cfg.metadata)
				if (write(mfd, (void *)mdata, cfg.metadata_len) < 0)
					perror("failed to write metadata buffer");
		} else if (!cfg.raw_binary) {
			if (data && cfg.read && !err)
				d((unsigned char *)data, cfg.data_len, 16, 1);
		} else if (data && cfg.read)
			d_raw((unsigned char *)data, cfg.data_len);
	}
free_metadata:
	free(mdata);
free_data:
	nvme_free(data, huge);
close_dfd:
	if (strlen(cfg.input_file))
		close(dfd);
close_mfd:
	if (strlen(cfg.metadata))
		close(mfd);
close_fd:
	close(fd);
ret:
	return err;
}


// func 1
static int print_nand(int argc, char **argv, struct command *cmd, struct plugin *plugin)
{
	const char *desc = "Hoon plugin command";
	
	int flags;
	int mode = S_IRUSR | S_IWUSR |S_IRGRP | S_IWGRP| S_IROTH;
	void *data = NULL, *mdata = NULL;
	int err = 0, dfd, mfd, fd;
	__u32 result;
	bool huge = false;
	const char *cmd_name = NULL;
	struct timeval start_time, end_time;

	struct config {
		__u8	opcode;
		__u8	flags;
		__u16	rsvd;
		__u32	namespace_id;
		__u32	data_len;
		__u32	metadata_len;
		__u32	timeout;
		__u32	cdw2;
		__u32	cdw3;
		__u32	cdw10;
		__u32	cdw11;
		__u32	cdw12;
		__u32	cdw13;
		__u32	cdw14;
		__u32	cdw15;
		char	*input_file;
		char	*metadata;
		int	raw_binary;
		int	show_command;
		int	dry_run;
		int	read;
		int	write;
		__u8	prefill;
		int	latency;
	};

	struct config cfg = {
		.opcode		= 0x90,
		.flags		= 0,
		.prefill	= 0,
		.rsvd		= 0,
		.namespace_id	= 0,
		.data_len	= 0,
		.metadata_len	= 0,
		.timeout	= 0,
		.cdw2		= 0,
		.cdw3		= 0,
		.cdw10		= 0,
		.cdw11		= 0,
		.cdw12		= 0,
		.cdw13		= 0,
		.cdw14		= 0,
		.cdw15		= 0,
		.input_file	= "",
		.metadata	= "",
		.raw_binary	= 0,
		.show_command	= 0,
		.dry_run	= 0,
		.read		= 0,
		.write		= 0,
		.latency	= 0,
	};

	OPT_ARGS(opts) = {
	
		OPT_END()
	};

	err = fd = parse_and_open(argc, argv, desc, opts);
	if (fd < 0)
		goto ret;

	if (cfg.show_command || cfg.dry_run) {
		printf("opcode       : %02x\n", cfg.opcode);
		printf("flags        : %02x\n", cfg.flags);
		printf("rsvd1        : %04x\n", cfg.rsvd);
		printf("nsid         : %08x\n", cfg.namespace_id);
		printf("cdw2         : %08x\n", cfg.cdw2);
		printf("cdw3         : %08x\n", cfg.cdw3);
		printf("data_len     : %08x\n", cfg.data_len);
		printf("metadata_len : %08x\n", cfg.metadata_len);
		printf("addr         : %"PRIx64"\n", (uint64_t)(uintptr_t)data);
		printf("metadata     : %"PRIx64"\n", (uint64_t)(uintptr_t)mdata);
		printf("cdw10        : %08x\n", cfg.cdw10);
		printf("cdw11        : %08x\n", cfg.cdw11);
		printf("cdw12        : %08x\n", cfg.cdw12);
		printf("cdw13        : %08x\n", cfg.cdw13);
		printf("cdw14        : %08x\n", cfg.cdw14);
		printf("cdw15        : %08x\n", cfg.cdw15);
		printf("timeout_ms   : %08x\n", cfg.timeout);
	}
	if (cfg.dry_run)
		goto free_data;

	gettimeofday(&start_time, NULL);

	//if (admin)
		err = nvme_admin_passthru(fd, cfg.opcode, cfg.flags, cfg.rsvd,
				cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
				cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14,
				cfg.cdw15, cfg.data_len, data, cfg.metadata_len,
				mdata, cfg.timeout, &result);
	// else
	// 	err = nvme_io_passthru(fd, cfg.opcode, cfg.flags, cfg.rsvd,
	// 			cfg.namespace_id, cfg.cdw2, cfg.cdw3, cfg.cdw10,
	// 			cfg.cdw11, cfg.cdw12, cfg.cdw13, cfg.cdw14,
	// 			cfg.cdw15, cfg.data_len, data, cfg.metadata_len,
	// 			mdata, cfg.timeout, &result);

	gettimeofday(&end_time, NULL);
	cmd_name = nvme_cmd_to_string(true, cfg.opcode);
	if (cfg.latency)
		printf("%s Command %s latency: %llu us\n",
			//admin ? "Admin": "IO",
			"Admin",
			strcmp(cmd_name, "Unknown") ? cmd_name: "Vendor Specific",
			elapsed_utime(start_time, end_time));

	if (err < 0)
		fprintf(stderr, "passthru: %s\n", nvme_strerror(errno));
	else if (err)
		nvme_show_status(err);
	else  {
		fprintf(stderr, "%s Command %s is Success and result: 0x%08x\n",
				"Admin",
				strcmp(cmd_name, "Unknown") ? cmd_name: "Vendor Specific",
				result);
		if (cfg.read && cfg.input_file) {
			if (write(dfd, (void *)data, cfg.data_len) < 0)
				perror("failed to write data buffer");
			if (cfg.metadata_len && cfg.metadata)
				if (write(mfd, (void *)mdata, cfg.metadata_len) < 0)
					perror("failed to write metadata buffer");
		} else if (!cfg.raw_binary) {
			if (data && cfg.read && !err)
				d((unsigned char *)data, cfg.data_len, 16, 1);
		} else if (data && cfg.read)
			d_raw((unsigned char *)data, cfg.data_len);
	}
free_metadata:
	free(mdata);
free_data:
	nvme_free(data, huge);
close_dfd:
	if (strlen(cfg.input_file))
		close(dfd);
close_mfd:
	if (strlen(cfg.metadata))
		close(mfd);
close_fd:
	close(fd);
ret:
	return err;
}