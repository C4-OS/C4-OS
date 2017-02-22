#ifndef _C4OS_ATA_H
#define _C4OS_ATA_H 1
#include <stdint.h>
#include <stdbool.h>

enum {
	ATA_MODE_NULL,
	ATA_MODE_PIO,
};

// status codes
enum {
	ATA_STATUS_ERROR     = 0x01,
	ATA_STATUS_IDX       = 0x02,
	ATA_STATUS_CORR      = 0x04,
	ATA_STATUS_DRQ       = 0x08,
	ATA_STATUS_DSC       = 0x10,
	ATA_STATUS_DF        = 0x20,
	ATA_STATUS_DRV_READY = 0x40,
	ATA_STATUS_BUSY      = 0x80
};

// Error codes
// TODO: check standard to make sure these are correct
enum {
	ATA_ERROR_NO_ADDR_MARK = 0x01,
	ATA_ERROR_NO_MEDIA     = 0x02,
	ATA_ERROR_CMD_ABORTED  = 0x04,
	ATA_ERROR_MEDIA_ERROR  = 0x08,
	ATA_ERROR_NO_ID_MARK   = 0x10,
	ATA_ERROR_ASDF         = 0x20,
	ATA_ERROR_FATAL_ERROR  = 0x40,
	ATA_ERROR_BAD_SECTORS  = 0x80
};

// Command codes
enum {
	ATA_CMD_READ_PIO        = 0x20,
	ATA_CMD_READ_PIO_EXT    = 0x24,
	ATA_CMD_READ_DMA        = 0xc8,
	ATA_CMD_READ_DMA_EXT    = 0x25,

	ATA_CMD_WRITE_PIO       = 0x30,
	ATA_CMD_WRITE_PIO_EXT   = 0x34,
	ATA_CMD_WRITE_DMA       = 0xca,
	ATA_CMD_WRITE_DMA_EXT   = 0x35,

	ATA_CMD_CACHE_FLUSH     = 0xe7,
	ATA_CMD_CACHE_FLUSH_EXT = 0xea,
	ATA_CMD_PACKET          = 0xa0,
	ATA_CMD_IDENT_PACKET    = 0xa1,
	ATA_CMD_IDENT           = 0xec
};

// ATAPI command codes
enum {
	ATAPI_CMD_READ  = 0xa8,
	ATAPI_CMD_EJECT = 0x1b
};

// Identification
enum {
	ATA_IDENT_DEVICE_TYPE  = 0x00,
	ATA_IDENT_CYLINDERS    = 0x02,
	ATA_IDENT_HEADS        = 0x06,
	ATA_IDENT_SECTORS      = 0x0c,
	ATA_IDENT_SERIAL       = 0x14,
	ATA_IDENT_MODEL        = 0x36,
	ATA_IDENT_CAPABILITIES = 0x62,
	ATA_IDENT_FIELD_VALID  = 0x6a,
	ATA_IDENT_MAX_LBA      = 0x80,
	ATA_IDENT_COMMAND_SETS = 0xa4,
	ATA_IDENT_MAX_LBA_EXT  = 0xc8
};

// default compatibility port locations
enum {
	ATA_PRIMARY_PORT   = 0x1f0,
	ATA_SECONDARY_PORT = 0x170,
	ATA_CONTROL        = 0x204,
};

enum {
	ATA_PRIMARY,
	ATA_SECONDARY,
};

// Master and slave selection
enum {
	ATA_MASTER,
	ATA_SLAVE
};

// ATA registers
enum {
	ATA_REG_DATA          = 0x00,
	ATA_REG_ERROR         = 0x01,
	ATA_REG_FEATURES      = 0x01,
	ATA_REG_SEC_COUNT0    = 0x02,
	ATA_REG_LBA0          = 0x03,
	ATA_REG_LBA1          = 0x04,
	ATA_REG_LBA2          = 0x05,
	ATA_REG_HD_DEV_SELECT = 0x06,
	ATA_REG_COMMAND       = 0x07,
	ATA_REG_STATUS        = 0x07,
	ATA_REG_SEC_COUNT1    = 0x08,
	ATA_REG_LBA3          = 0x09,
	ATA_REG_LBA4          = 0x0a,
	ATA_REG_LBA5          = 0x0b,
	ATA_REG_CONTROL       = 0x06,
	ATA_REG_ALT_STATUS    = 0x06,
	ATA_REG_DEV_ADDR      = 0x0d
};

// control register values
enum {
	ATA_CONTROL_DISABLE_INTR = 1 << 1,
	ATA_CONTROL_SOFT_RESET   = 1 << 2,
	ATA_CONTROL_HOB          = 1 << 7,
};

typedef struct ide_channel {
	uint16_t base;
	uint16_t control;
	uint16_t busmaster;
	uint8_t  no_int;
} ide_channel_t;

struct ide_control;
typedef struct ide_device {
	bool     exists;
	uint8_t  channel;
	uint8_t  drive;
	uint16_t type;
	uint16_t sign;
	uint16_t capabilities;
	uint32_t commands;
	uint32_t size;
	char     model[41];

	struct ide_control *ctrl;
} ide_device_t;

typedef struct ide_control {
	ide_channel_t channels[2];
	ide_device_t devices[4];
} ide_control_t;

#endif
