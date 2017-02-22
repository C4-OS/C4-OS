#include <ata/ata.h>
#include <c4rt/c4rt.h>
#include <c4/arch/interrupts.h>
#include <stdint.h>
#include <stdbool.h>

static ide_control_t ide_drives;

static inline ide_channel_t *ide_channel( unsigned channel );
static inline ide_device_t *ide_device( unsigned device );
static inline unsigned device_number( unsigned channel, unsigned select );

void    ide_init( void );
uint8_t ide_reg_read( ide_channel_t *channel, uint8_t reg );
void    ide_reg_write( ide_channel_t *channel, uint8_t reg, uint8_t data );
void    ide_copy_model( ide_device_t *device, uint8_t *buffer );
void    ide_init_drive( unsigned chan_num, unsigned select );
void    ide_select_drive( ide_channel_t *channel, unsigned select );
void    ide_identify( ide_channel_t *channel );
bool    ide_init_poll( ide_channel_t *channel );
void    ide_channel_poll( ide_channel_t *channel );

void ide_read_buffer( ide_channel_t *channel,
                      unsigned port,
                      uint32_t *buffer,
                      unsigned size );

void ide_pio_read( ide_device_t *device,
                   uint16_t *buffer,
                   unsigned location,
                   unsigned sectors );

void ide_pio_write( ide_device_t *device,
                    uint16_t *buffer,
                    unsigned location,
                    unsigned sectors );

void _start( uintptr_t nameserver ){
	c4_debug_printf( "--- ata: hello, world! thread %u\n", c4_get_id());
	ide_init();

	while ( true ){
		message_t msg;

		c4_msg_recieve( &msg, 0 );
	}

	c4_exit();
}

static inline void interrupt_subscribe( unsigned intr ){
	message_t msg = {
		.type = MESSAGE_TYPE_INTERRUPT_SUBSCRIBE,
		.data = { intr },
	};

	c4_msg_send( &msg, 0 );
}

static inline unsigned interrupt_wait( void ){
	message_t msg;

	c4_msg_recieve_async( &msg, MESSAGE_ASYNC_BLOCK );

	if ( msg.type != MESSAGE_TYPE_INTERRUPT ){
		c4_debug_printf(
			"--- ata: got asyncronous message which "
			"is not an interrupt, ignoring...\n" );
	}

	return msg.data[0];
}

void ide_init( void ){
	interrupt_subscribe( 0x20 + 9 );
	interrupt_subscribe( 0x20 + 11 );
	interrupt_subscribe( 0x20 + 14 );
	interrupt_subscribe( 0x20 + 15 );

	ide_channel(ATA_PRIMARY)->base      = ATA_PRIMARY_PORT;
	ide_channel(ATA_PRIMARY)->control   = ATA_PRIMARY_PORT + ATA_CONTROL;
	ide_channel(ATA_SECONDARY)->base    = ATA_SECONDARY_PORT;
	ide_channel(ATA_SECONDARY)->control = ATA_SECONDARY_PORT + ATA_CONTROL;

	ide_reg_read( ide_channel(ATA_PRIMARY), ATA_REG_STATUS );
	ide_reg_read( ide_channel(ATA_SECONDARY), ATA_REG_STATUS );

	// temporary, interrupts will need to be re-enabled once DMA is implemented
	// TODO: implement DMA
	ide_reg_write( ide_channel(ATA_PRIMARY),
	               ATA_REG_CONTROL, ATA_CONTROL_DISABLE_INTR );
	ide_reg_write( ide_channel(ATA_SECONDARY),
	               ATA_REG_CONTROL, ATA_CONTROL_DISABLE_INTR );

	for ( unsigned channel = ATA_PRIMARY; channel <= ATA_SECONDARY; channel++ ){
		for ( unsigned select = ATA_MASTER; select <= ATA_SLAVE; select++ ){
			ide_init_drive( channel, select );
		}
	}

	if ( ide_device(0)->exists ){
		static char volatile buf[1024];
		unsigned tempsec = 1054;

		ide_pio_read( ide_device(0), (uint16_t *)buf, 0, sizeof(buf)/512 );

		c4_debug_printf( "--- ata: first sector: " );
		for ( unsigned i = 0; i < sizeof(buf); i++ ){
			c4_debug_printf( "%x ", buf[i] & 0xff );
		}
		c4_debug_printf( "\n" );

		c4_debug_printf( "--- ata: writing first sector to %u...\n", tempsec );
		ide_pio_write( ide_device(0), (uint16_t *)buf, tempsec, sizeof(buf)/512 );
	}
}

void ide_init_drive( unsigned chan_num, unsigned select ){
	ide_device_t  *device  = ide_device( device_number(chan_num, select));
	ide_channel_t *channel = ide_channel( chan_num );

	device->exists = false;

	ide_select_drive( channel, select );
	ide_channel_poll( channel );
	ide_identify( channel );
	ide_channel_poll( channel );

	if ( ide_reg_read( channel, ATA_REG_STATUS ) == 0 ){
		return;
	}

	// poll until the drive is ready, and return if there's an error
	if ( ide_init_poll( channel )){
		return;
	}

	// `static` to avoid placing this on the thread's stack
	// currently this would fit on the stack anyway, but it might be important
	// in the future if a config option for the default stack size is added
	static uint8_t buffer[512];
	uint8_t *temp = buffer;

	ide_read_buffer( channel, ATA_REG_DATA, (uint32_t *)buffer, 512 );
	ide_copy_model( device, temp );

	device->exists       = true;
	device->channel      = chan_num;
	device->drive        = select;
	device->sign         = *(uint16_t *)(temp + ATA_IDENT_DEVICE_TYPE);
	device->capabilities = *(uint16_t *)(temp + ATA_IDENT_CAPABILITIES);
	device->commands     = *(uint32_t *)(temp + ATA_IDENT_COMMAND_SETS);
	device->size         = *(uint32_t *)(temp + ATA_IDENT_MAX_LBA);

	c4_debug_printf( "--- ata: found drive %u:%u, %u sectors, \"%s\"\n",
		chan_num, select, device->size, device->model );
}

void ide_define_access( ide_channel_t *channel,
                        unsigned location,
                        unsigned count )
{
	ide_reg_write( channel, ATA_REG_SEC_COUNT0, count );
	ide_reg_write( channel, ATA_REG_LBA0, location & 0xff );
	ide_reg_write( channel, ATA_REG_LBA1, (location >> 8)  & 0xff );
	ide_reg_write( channel, ATA_REG_LBA2, (location >> 16) & 0xff );
}

bool ide_channel_ready( ide_channel_t *channel ){
	unsigned status = ide_reg_read(channel, ATA_REG_STATUS);
	return !(status & ATA_STATUS_BUSY) || (status & ATA_STATUS_DRQ);
}

bool ide_channel_has_error( ide_channel_t *channel ){
	unsigned status = ide_reg_read(channel, ATA_REG_STATUS);

	return (status & ATA_STATUS_ERROR) || (status & ATA_STATUS_DF);
}

void ide_channel_poll( ide_channel_t *channel ){
	for ( unsigned i = 0; i < 4; i++ ){
		ide_reg_read( channel, ATA_REG_ALT_STATUS );
	}

	while ( !ide_channel_ready( channel )){
		unsigned status = ide_reg_read(channel, ATA_REG_STATUS);

		if ( ide_channel_has_error( channel )){
			c4_debug_printf( "--- ata: an error! %b\n",
				ide_reg_read(channel, ATA_REG_ERROR));

			break;
		}
	}

	// XXX: assume there was an interrupt message, and just ignore it if so
	message_t msg;
	c4_msg_recieve_async( &msg, 0 );
}

static inline void ide_start_access( ide_device_t *device,
                                     unsigned location,
                                     unsigned count,
                                     unsigned command )
{
	ide_channel_t *channel = ide_channel( device->channel );

	while ( !ide_channel_ready( channel ));

	ide_reg_write( channel, ATA_REG_HD_DEV_SELECT,
	               0xe0 | (device->drive << 4)
	               | ((location >> 24) & 0xf) );
	ide_define_access( channel, location, count );
	ide_reg_write( channel, ATA_REG_COMMAND, command );

	ide_channel_poll( channel );
}

void ide_pio_read( ide_device_t *device,
                   uint16_t *buffer,
                   unsigned location,
                   unsigned count )
{
	ide_channel_t *channel = ide_channel( device->channel );

	ide_start_access( device, location, count, ATA_CMD_READ_PIO );

	for ( unsigned sector = 0; sector < count; sector++ ){
		for ( unsigned i = 0; i < 256; i++ ){
			unsigned index = sector * 256 + i;

			buffer[index] = c4_in_word( channel->base );
		}

		ide_channel_poll( channel );
	}
}

void ide_pio_write( ide_device_t *device,
                    uint16_t *buffer,
                    unsigned location,
                    unsigned count )
{
	ide_channel_t *channel = ide_channel( device->channel );

	ide_start_access( device, location, count, ATA_CMD_WRITE_PIO );

	for ( unsigned sector = 0; sector < count; sector++ ){
		for ( unsigned i = 0; i < 256; i++ ){
			unsigned index = sector * 256 + i;

			c4_out_word( channel->base, buffer[index] );
		}

		ide_channel_poll( channel );
	}

	ide_reg_write( channel, ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH );
}

void ide_select_drive( ide_channel_t *channel, unsigned select ){
	ide_reg_write( channel,
	               ATA_REG_HD_DEV_SELECT,
	               ATA_CMD_PACKET | (select << 4) );
}

// this assumes the proper drive is already selected, with ide_select_drive()
void ide_identify( ide_channel_t *channel ){
	ide_reg_write( channel, ATA_REG_COMMAND, ATA_CMD_IDENT );
}

uint8_t ide_reg_read( ide_channel_t *channel, uint8_t reg ){
	// TODO: things for lba48 and bus mastering
	return c4_in_byte( channel->base + reg );
}

void ide_reg_write( ide_channel_t *channel, uint8_t reg, uint8_t data ){
	// TODO: things for lba48 and bus mastering
	c4_out_byte( channel->base + reg, data );
}

void ide_read_buffer( ide_channel_t *channel,
                      unsigned port,
                      uint32_t *buffer,
                      unsigned size )
{
	// TODO: things for lba48 and bus mastering
	//       (although will it be needed for this function?)
	unsigned dwords = size / 4;

	for ( unsigned i = 0; i < dwords; i++ ){
		buffer[i] = c4_in_dword( channel->base + port );
	}
}

bool ide_init_poll( ide_channel_t *channel ){
	while ( true ){
		unsigned status = ide_reg_read( channel, ATA_REG_STATUS );

		if ( status & ATA_STATUS_ERROR ){
			// TODO: Handle error conditions, might be ATAPI
			return true;;
		}

		if ( !(status & ATA_STATUS_BUSY) && (status & ATA_STATUS_DRQ) ){
			break;
		}
	}

	return false;
}

void ide_copy_model( ide_device_t *device, uint8_t *buffer ){
	uint8_t *model = buffer + ATA_IDENT_MODEL;

	for ( unsigned i = 0; i < 40; i += 2 ){
		device->model[i]     = model[i + 1];
		device->model[i + 1] = model[i];
	}

	device->model[40] = '\0';
}

static inline ide_channel_t *ide_channel( unsigned channel ){
	return ide_drives.channels + channel;
}

static inline ide_device_t *ide_device( unsigned device ){
	return ide_drives.devices + device;
}

static inline unsigned device_number( unsigned channel, unsigned select ){
	return channel * 2 + select;
}
